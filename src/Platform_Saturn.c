#include "Core.h"
#if defined PLAT_SATURN

#include "_PlatformBase.h"
#include "Stream.h"
#include "ExtMath.h"
#include "Funcs.h"
#include "Window.h"
#include "Utils.h"
#include "Errors.h"
#include "Options.h"
#include "PackedCol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <yaul.h>

void* calloc(size_t num, size_t size) {
	void* ptr = malloc(num * size);
	if (ptr) memset(ptr, 0, num * size);
	return ptr;
}
#include "_PlatformConsole.h"

const cc_result ReturnCode_FileShareViolation = 1000000000; // not used
const cc_result ReturnCode_FileNotFound       = 99999;
const cc_result ReturnCode_DirectoryExists    = 99999;

const cc_result ReturnCode_SocketInProgess  = -1;
const cc_result ReturnCode_SocketWouldBlock = -1;
const char* Platform_AppNameSuffix = " Saturn";


/*########################################################################################################################*
*------------------------------------------------------Logging/Time-------------------------------------------------------*
*#########################################################################################################################*/
#define WRITE_ADDRESS CS0(0x00100001)
// in Medafen, patch DummyWrite in src/ss/Cart.cpp to instead log the value

void Platform_Log(const char* msg, int len) {
	for (int i = 0; i < len; i++) {
		MEMORY_WRITE(8, WRITE_ADDRESS, msg[i]);
	}
	MEMORY_WRITE(8, WRITE_ADDRESS, '\n');
}

TimeMS DateTime_CurrentUTC(void) {
	return 0;
}

void DateTime_CurrentLocal(struct DateTime* t) {
	Mem_Set(t, 0, sizeof(struct DateTime));
}


/*########################################################################################################################*
*--------------------------------------------------------Stopwatch--------------------------------------------------------*
*#########################################################################################################################*/
static volatile cc_uint32 overflow_count;

cc_uint64 Stopwatch_Measure(void) {
	return cpu_frt_count_get() + (overflow_count * 65536);
}

cc_uint64 Stopwatch_ElapsedMicroseconds(cc_uint64 beg, cc_uint64 end) {
	if (end < beg) return 0;
	cc_uint32 delta = end - beg;

	// TODO still wrong?? and overflows?? and PAL detection ???
	return (delta * 1000) / CPU_FRT_NTSC_320_128_COUNT_1MS;
}

static void ovf_handler(void) { overflow_count++; }

static void Stopwatch_Init(void) {
	//cpu_frt_init(CPU_FRT_CLOCK_DIV_8);
	cpu_frt_init(CPU_FRT_CLOCK_DIV_128);
	cpu_frt_ovi_set(ovf_handler);

	cpu_frt_interrupt_priority_set(15);
	cpu_frt_count_set(0);
}


/*########################################################################################################################*
*-----------------------------------------------------Directory/File------------------------------------------------------*
*#########################################################################################################################*/
cc_result Directory_Create(const cc_string* path) {
	return ERR_NOT_SUPPORTED;
}

int File_Exists(const cc_string* path) {
	return ERR_NOT_SUPPORTED;
}

cc_result Directory_Enum(const cc_string* dirPath, void* obj, Directory_EnumCallback callback) {
	return ERR_NOT_SUPPORTED;
}

cc_result File_Open(cc_file* file, const cc_string* path) {
	return ERR_NOT_SUPPORTED;
}

cc_result File_Create(cc_file* file, const cc_string* path) {
	return ERR_NOT_SUPPORTED;
}

cc_result File_OpenOrCreate(cc_file* file, const cc_string* path) {
	return ERR_NOT_SUPPORTED;
}

cc_result File_Read(cc_file file, void* data, cc_uint32 count, cc_uint32* bytesRead) {
	return ERR_NOT_SUPPORTED;
}

cc_result File_Write(cc_file file, const void* data, cc_uint32 count, cc_uint32* bytesWrote) {
	return ERR_NOT_SUPPORTED;
}

cc_result File_Close(cc_file file) {
	return ERR_NOT_SUPPORTED;
}

cc_result File_Seek(cc_file file, int offset, int seekType) {	
	return ERR_NOT_SUPPORTED;
}

cc_result File_Position(cc_file file, cc_uint32* pos) {
	return ERR_NOT_SUPPORTED;
}

cc_result File_Length(cc_file file, cc_uint32* len) {
	return ERR_NOT_SUPPORTED;
}


/*########################################################################################################################*
*--------------------------------------------------------Threading--------------------------------------------------------*
*#########################################################################################################################*/
void Thread_Sleep(cc_uint32 milliseconds) {
	// TODO sleep a bit
}

void Thread_Run(void** handle, Thread_StartFunc func, int stackSize, const char* name) {
	*handle = NULL;
}

void Thread_Detach(void* handle) {
}

void Thread_Join(void* handle) {
}

void* Mutex_Create(void) {
	return NULL;
}

void Mutex_Free(void* handle) {
}

void Mutex_Lock(void* handle) {
}

void Mutex_Unlock(void* handle) {
}

void* Waitable_Create(void) {
	return NULL;
}

void Waitable_Free(void* handle) {
}

void Waitable_Signal(void* handle) {
}

void Waitable_Wait(void* handle) {
}

void Waitable_WaitFor(void* handle, cc_uint32 milliseconds) {
}


/*########################################################################################################################*
*---------------------------------------------------------Socket----------------------------------------------------------*
*#########################################################################################################################*/
cc_result Socket_ParseAddress(const cc_string* address, int port, cc_sockaddr* addrs, int* numValidAddrs) {
	return ERR_NOT_SUPPORTED;
}

cc_result Socket_Connect(cc_socket* s, cc_sockaddr* addr, cc_bool nonblocking) {
	return ERR_NOT_SUPPORTED;
}

cc_result Socket_Read(cc_socket s, cc_uint8* data, cc_uint32 count, cc_uint32* modified) {
	return ERR_NOT_SUPPORTED;
}

cc_result Socket_Write(cc_socket s, const cc_uint8* data, cc_uint32 count, cc_uint32* modified) {
	return ERR_NOT_SUPPORTED;
}

void Socket_Close(cc_socket s) {
}

cc_result Socket_CheckReadable(cc_socket s, cc_bool* readable) {
	return ERR_NOT_SUPPORTED;
}

cc_result Socket_CheckWritable(cc_socket s, cc_bool* writable) {
	return ERR_NOT_SUPPORTED;
}


/*########################################################################################################################*
*--------------------------------------------------------Platform---------------------------------------------------------*
*#########################################################################################################################*/
void Platform_Init(void) {
	Stopwatch_Init();
}

void Platform_Free(void) { }

cc_bool Platform_DescribeError(cc_result res, cc_string* dst) {
	return false;
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
