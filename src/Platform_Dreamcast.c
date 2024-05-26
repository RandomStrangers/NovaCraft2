#include "Core.h"
#if defined CC_BUILD_DREAMCAST
#include "_PlatformBase.h"
#include "Stream.h"
#include "ExtMath.h"
#include "Funcs.h"
#include "Window.h"
#include "Utils.h"
#include "Errors.h"

#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <time.h>
#include <ppp/ppp.h>
#include <kos.h>
#include <dc/sd.h>
#include <fat/fs_fat.h>
#include <kos/dbgio.h>
#include "_PlatformConsole.h"
KOS_INIT_FLAGS(INIT_DEFAULT | INIT_NET);

const cc_result ReturnCode_FileShareViolation = 1000000000; // not used
const cc_result ReturnCode_FileNotFound     = ENOENT;
const cc_result ReturnCode_SocketInProgess  = EINPROGRESS;
const cc_result ReturnCode_SocketWouldBlock = EWOULDBLOCK;
const cc_result ReturnCode_DirectoryExists  = EEXIST;
const char* Platform_AppNameSuffix = " Dreamcast";


/*########################################################################################################################*
*------------------------------------------------------Logging/Time-------------------------------------------------------*
*#########################################################################################################################*/
cc_uint64 Stopwatch_ElapsedMicroseconds(cc_uint64 beg, cc_uint64 end) {
	if (end < beg) return 0;
	return end - beg;
}

cc_uint64 Stopwatch_Measure(void) {
	return timer_us_gettime64();
}

static uint32 str_offset;
extern cc_bool window_inited;
#define MAX_ONSCREEN_LINES 20

static void LogOnscreen(const char* msg, int len) {
	char buffer[50];
	cc_string str;
	uint32 secs, ms;
	timer_ms_gettime(&secs, &ms);
	
	String_InitArray_NT(str, buffer);
	String_Format2(&str, "[%p2.%p3] ", &secs, &ms);
	String_AppendAll(&str, msg, len);
	buffer[str.length] = '\0';
	
	uint32 line_offset = (10 + (str_offset * BFONT_HEIGHT)) * vid_mode->width;
	bfont_draw_str(vram_s + line_offset, vid_mode->width, 1, buffer);
	str_offset = (str_offset + 1) % MAX_ONSCREEN_LINES;
}

void Platform_Log(const char* msg, int len) {
	dbgio_write_buffer_xlat(msg,  len);
	dbgio_write_buffer_xlat("\n",   1);
	
	if (window_inited) return;
	// Log details on-screen for initial model initing etc
	//  (this can take around 40 seconds on average)
	LogOnscreen(msg, len);
}

TimeMS DateTime_CurrentUTC(void) {
	uint32 secs, ms;
	timer_ms_gettime(&secs, &ms);
	
	time_t boot_time = rtc_boot_time();
	// workaround when RTC clock hasn't been setup
	int boot_time_2000 =  946684800;
	int boot_time_2024 = 1704067200;
	if (boot_time < boot_time_2000) boot_time = boot_time_2024;
	
	cc_uint64 curSecs = boot_time + secs;
	return curSecs + UNIX_EPOCH_SECONDS;
}

void DateTime_CurrentLocal(struct DateTime* t) {
	uint32 secs, ms;
	time_t total_secs;
	struct tm loc_time;
	
	timer_ms_gettime(&secs, &ms);
	total_secs = rtc_boot_time() + secs;
	localtime_r(&total_secs, &loc_time);

	t->year   = loc_time.tm_year + 1900;
	t->month  = loc_time.tm_mon  + 1;
	t->day    = loc_time.tm_mday;
	t->hour   = loc_time.tm_hour;
	t->minute = loc_time.tm_min;
	t->second = loc_time.tm_sec;
}


/*########################################################################################################################*
*-----------------------------------------------------Directory/File------------------------------------------------------*
*#########################################################################################################################*/
static cc_string root_path = String_FromConst("/cd/");

static void GetNativePath(char* str, const cc_string* path) {
	Mem_Copy(str, root_path.buffer, root_path.length);
	str += root_path.length;
	String_EncodeUtf8(str, path);
}

cc_result Directory_Create(const cc_string* path) {
	char str[NATIVE_STR_LEN];
	GetNativePath(str, path);
	
	int res = fs_mkdir(str);
	int err = res == -1 ? errno : 0;
	
	// Filesystem returns EINVAL when operation unsupported (e.g. CD system)
	//  so rather than logging an error, just pretend it already exists
	if (err == EINVAL) err = EEXIST;
	return err;
}

int File_Exists(const cc_string* path) {
	char str[NATIVE_STR_LEN];
	struct stat sb;
	GetNativePath(str, path);
	return fs_stat(str, &sb, 0) == 0 && S_ISREG(sb.st_mode);
}

cc_result Directory_Enum(const cc_string* dirPath, void* obj, Directory_EnumCallback callback) {
	cc_string path; char pathBuffer[FILENAME_SIZE];
	char str[NATIVE_STR_LEN];
	int res;
	// CD filesystem loader doesn't usually set errno
	//  when it can't find the requested file
	errno = 0;

	GetNativePath(str, dirPath);
	int fd = fs_open(str, O_DIR | O_RDONLY);
	if (fd < 0) return errno;

	String_InitArray(path, pathBuffer);
	dirent_t* entry;
	errno = 0;
	
	while ((entry = fs_readdir(fd))) {
		path.length = 0;
		String_Format1(&path, "%s/", dirPath);

		// ignore . and .. entry (PSP does return them)
		// TODO: Does Dreamcast?
		char* src = entry->name;
		if (src[0] == '.' && src[1] == '\0')                  continue;
		if (src[0] == '.' && src[1] == '.' && src[2] == '\0') continue;
		
		int len = String_Length(src);
		String_AppendUtf8(&path, src, len);

		// negative size indicates a directory entry
		if (entry->size < 0) {
			res = Directory_Enum(&path, obj, callback);
			if (res) break;
		} else {
			callback(&path, obj);
		}
	}

	int err = errno; // save error from fs_readdir
	fs_close(fd);
	return err;
}

static cc_result File_Do(cc_file* file, const cc_string* path, int mode) {
	char str[NATIVE_STR_LEN];
	GetNativePath(str, path);
	// CD filesystem loader doesn't usually set errno
	//  when it can't find the requested file
	errno = 0;

	int res = fs_open(str, mode);
	*file   = res;
	
	int err = res == -1 ? errno : 0;
	if (res == -1 && err == 0) err = ENOENT;
	return err;
}

cc_result File_Open(cc_file* file, const cc_string* path) {
	return File_Do(file, path, O_RDONLY);
}
cc_result File_Create(cc_file* file, const cc_string* path) {
	return File_Do(file, path, O_RDWR | O_CREAT | O_TRUNC);
}
cc_result File_OpenOrCreate(cc_file* file, const cc_string* path) {
	return File_Do(file, path, O_RDWR | O_CREAT);
}

cc_result File_Read(cc_file file, void* data, cc_uint32 count, cc_uint32* bytesRead) {
	int res    = fs_read(file, data, count);
	*bytesRead = res;
	return res == -1 ? errno : 0;
}

cc_result File_Write(cc_file file, const void* data, cc_uint32 count, cc_uint32* bytesWrote) {
	int res     = fs_write(file, data, count);
	*bytesWrote = res;
	return res == -1 ? errno : 0;
}

cc_result File_Close(cc_file file) {
	int res = fs_close(file);
	return res == -1 ? errno : 0;
}

cc_result File_Seek(cc_file file, int offset, int seekType) {
	static cc_uint8 modes[3] = { SEEK_SET, SEEK_CUR, SEEK_END };
	
	int res = fs_seek(file, offset, modes[seekType]);
	return res == -1 ? errno : 0;
}

cc_result File_Position(cc_file file, cc_uint32* pos) {
	int res = fs_seek(file, 0, SEEK_CUR);
	*pos    = res;
	return res == -1 ? errno : 0;
}

cc_result File_Length(cc_file file, cc_uint32* len) {
	int res = fs_total(file);
	*len    = res;
	return res == -1 ? errno : 0;
}


/*########################################################################################################################*
*--------------------------------------------------------Threading--------------------------------------------------------*
*#########################################################################################################################*/
// !!! NOTE: Dreamcast is configured to use preemptive multithreading !!!
void Thread_Sleep(cc_uint32 milliseconds) { 
	thd_sleep(milliseconds); 
}

static void* ExecThread(void* param) {
	Thread_StartFunc func = (Thread_StartFunc)param;
	(func)();
	return NULL;
}

void Thread_Run(void** handle, Thread_StartFunc func, int stackSize, const char* name) {
	kthread_attr_t attrs = { 0 };
	attrs.stack_size     = stackSize;
	attrs.label          = name;
	*handle = thd_create_ex(&attrs, ExecThread, func);
}

void Thread_Detach(void* handle) {
	thd_detach((kthread_t*)handle);
}

void Thread_Join(void* handle) {
	thd_join((kthread_t*)handle, NULL);
}

void* Mutex_Create(void) {
	mutex_t* ptr = (mutex_t*)Mem_Alloc(1, sizeof(mutex_t), "mutex");
	int res = mutex_init(ptr, MUTEX_TYPE_NORMAL);
	if (res) Logger_Abort2(errno, "Creating mutex");
	return ptr;
}

void Mutex_Free(void* handle) {
	int res = mutex_destroy((mutex_t*)handle);
	if (res) Logger_Abort2(errno, "Destroying mutex");
	Mem_Free(handle);
}

void Mutex_Lock(void* handle) {
	int res = mutex_lock((mutex_t*)handle);
	if (res) Logger_Abort2(errno, "Locking mutex");
}

void Mutex_Unlock(void* handle) {
	int res = mutex_unlock((mutex_t*)handle);
	if (res) Logger_Abort2(errno, "Unlocking mutex");
}

void* Waitable_Create(void) {
	semaphore_t* ptr = (semaphore_t*)Mem_Alloc(1, sizeof(semaphore_t), "waitable");
	int res = sem_init(ptr, 0);
	if (res) Logger_Abort2(errno, "Creating waitable");
	return ptr;
}

void Waitable_Free(void* handle) {
	int res = sem_destroy((semaphore_t*)handle);
	if (res) Logger_Abort2(errno, "Destroying waitable");
	Mem_Free(handle);
}

void Waitable_Signal(void* handle) {
	int res = sem_signal((semaphore_t*)handle);
	if (res < 0) Logger_Abort2(errno, "Signalling event");
}

void Waitable_Wait(void* handle) {
	int res = sem_wait((semaphore_t*)handle);
	if (res < 0) Logger_Abort2(errno, "Event wait");
}

void Waitable_WaitFor(void* handle, cc_uint32 milliseconds) {
	int res = sem_wait_timed((semaphore_t*)handle, milliseconds);
	if (res >= 0) return;
	
	int err = errno;
	if (err != ETIMEDOUT) Logger_Abort2(err, "Event timed wait");
}


/*########################################################################################################################*
*---------------------------------------------------------Socket----------------------------------------------------------*
*#########################################################################################################################*/
cc_result Socket_ParseAddress(const cc_string* address, int port, cc_sockaddr* addrs, int* numValidAddrs) {
	char str[NATIVE_STR_LEN];

	char portRaw[32]; cc_string portStr;
	struct addrinfo hints = { 0 };
	struct addrinfo* result;
	struct addrinfo* cur;
	int res, i = 0;
	
	String_EncodeUtf8(str, address);
	*numValidAddrs = 0;

	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	String_InitArray(portStr,  portRaw);
	String_AppendInt(&portStr, port);
	portRaw[portStr.length] = '\0';
	
	// getaddrinfo IP address resolution was only added in Nov 2023
	//   https://github.com/KallistiOS/KallistiOS/pull/358
	// So include this special case for backwards compatibility
	struct sockaddr_in* addr4 = (struct sockaddr_in*)addrs[0].data;
	if (inet_pton(AF_INET, str, &addr4->sin_addr) > 0) {
		addr4->sin_family = AF_INET;
		addr4->sin_port   = htons(port);
		
		addrs[0].size  = sizeof(struct sockaddr_in);
		*numValidAddrs = 1;
		return 0;
	}

	res = getaddrinfo(str, portRaw, &hints, &result);
	if (res == EAI_NONAME) return SOCK_ERR_UNKNOWN_HOST;
	if (res) return res;

	for (cur = result; cur && i < SOCKET_MAX_ADDRS; cur = cur->ai_next, i++) 
	{
		Mem_Copy(addrs[i].data, cur->ai_addr, cur->ai_addrlen);
		addrs[i].size = cur->ai_addrlen;
	}

	freeaddrinfo(result);
	*numValidAddrs = i;
	return i == 0 ? ERR_INVALID_ARGUMENT : 0;
}

cc_result Socket_Connect(cc_socket* s, cc_sockaddr* addr, cc_bool nonblocking) {
	struct sockaddr* raw = (struct sockaddr*)addr->data;
	cc_result res;

	*s = socket(raw->sa_family, SOCK_STREAM, IPPROTO_TCP);
	if (*s == -1) return errno;

	if (nonblocking) {
		fcntl(*s, F_SETFL, O_NONBLOCK);
	}

	res = connect(*s, raw, addr->size);
	return res == -1 ? errno : 0;
}

cc_result Socket_Read(cc_socket s, cc_uint8* data, cc_uint32 count, cc_uint32* modified) {
	int recvCount = recv(s, data, count, 0);
	if (recvCount != -1) { *modified = recvCount; return 0; }
	*modified = 0; return errno;
}

cc_result Socket_Write(cc_socket s, const cc_uint8* data, cc_uint32 count, cc_uint32* modified) {
	int sentCount = send(s, data, count, 0);
	if (sentCount != -1) { *modified = sentCount; return 0; }
	*modified = 0; return errno;
}

void Socket_Close(cc_socket s) {
	shutdown(s, SHUT_RDWR);
	close(s);
}

static cc_result Socket_Poll(cc_socket s, int mode, cc_bool* success) {
	struct pollfd pfd;
	int flags;

	pfd.fd     = s;
	pfd.events = mode == SOCKET_POLL_READ ? POLLIN : POLLOUT;
	if (poll(&pfd, 1, 0) == -1) { *success = false; return errno; }
	
	/* to match select, closed socket still counts as readable */
	flags    = mode == SOCKET_POLL_READ ? (POLLIN | POLLHUP) : POLLOUT;
	*success = (pfd.revents & flags) != 0;
	return 0;
}

cc_result Socket_CheckReadable(cc_socket s, cc_bool* readable) {
	return Socket_Poll(s, SOCKET_POLL_READ, readable);
}

cc_result Socket_CheckWritable(cc_socket s, cc_bool* writable) {
	socklen_t resultSize = sizeof(socklen_t);
	cc_result res = Socket_Poll(s, SOCKET_POLL_WRITE, writable);
	if (res || *writable) return res;

	/* https://stackoverflow.com/questions/29479953/so-error-value-after-successful-socket-operation */
	getsockopt(s, SOL_SOCKET, SO_ERROR, &res, &resultSize);
	return res;
}


/*########################################################################################################################*
*--------------------------------------------------------Platform---------------------------------------------------------*
*#########################################################################################################################*/
static kos_blockdev_t sd_dev;
static uint8 partition_type;

static void InitSDCard(void) {
	if (sd_init()) {
		// Both SD card and debug interface use the serial port
		// So if initing SD card fails, need to restore serial port state for debug logging
		scif_init();
		Platform_LogConst("Failed to init SD card"); return;
	}
	
	if (sd_blockdev_for_partition(0, &sd_dev, &partition_type)) {
		Platform_LogConst("Unable to find first partition on SD card"); return;
  	}
  	
  	if (fs_fat_init()) {
		Platform_LogConst("Failed to init FAT filesystem"); return;
	}
	
  	if (fs_fat_mount("/sd", &sd_dev, FS_FAT_MOUNT_READWRITE)) {
		Platform_LogConst("Failed to mount SD card"); return;
  	}
  	
  	root_path = String_FromReadonly("/sd/ClassiCube");
	fs_mkdir("/sd/ClassiCube");
	Platform_ReadonlyFilesystem = false;
}

static void InitModem(void) {
	int err;
	Platform_LogConst("Initialising modem..");
	
	if (!modem_init()) {
		Platform_LogConst("Modem initing failed"); return;
	}
	ppp_init();
	
	Platform_LogConst("Dialling modem.. (can take ~20 seconds)");
	err = ppp_modem_init("111111111111", 0, NULL);
	if (err) {
		Platform_Log1("Establishing link failed (%i)", &err); return;
	}

	ppp_set_login("dream", "dreamcast");

	Platform_LogConst("Connecting link.. (can take ~20 seconds)");
	err = ppp_connect();
	if (err) {
		Platform_Log1("Connecting link failed (%i)", &err); return;
 	}
}

void Platform_Init(void) {
	Platform_ReadonlyFilesystem = true;
	InitSDCard();
	
	if (net_default_dev) return;
	// in case Broadband Adapter isn't active
	InitModem();
	// give some time for messages to stay on-screen
	Platform_LogConst("Starting in 5 seconds..");
	Thread_Sleep(5000);
}
void Platform_Free(void) { }

cc_bool Platform_DescribeError(cc_result res, cc_string* dst) {
	char chars[NATIVE_STR_LEN];
	int len;

	/* For unrecognised error codes, strerror_r might return messages */
	/*  such as 'No error information', which is not very useful */
	/* (could check errno here but quicker just to skip entirely) */
	if (res >= 1000) return false;

	len = strerror_r(res, chars, NATIVE_STR_LEN);
	if (len == -1) return false;

	len = String_CalcLen(chars, NATIVE_STR_LEN);
	String_AppendUtf8(dst, chars, len);
	return true;
}

cc_bool Process_OpenSupported = false;
cc_result Process_StartOpen(const cc_string* args) {
	return ERR_NOT_SUPPORTED;
}


/*########################################################################################################################*
*-------------------------------------------------------Encryption--------------------------------------------------------*
*#########################################################################################################################*/
static cc_result GetMachineID(cc_uint32* key) {
	return ERR_NOT_SUPPORTED;
}
#endif
