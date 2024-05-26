#ifndef CC_INPUT_H
#define CC_INPUT_H
#include "Core.h"
/* 
Manages input state, raising input related events, and base input handling
Copyright 2014-2023 ClassiCube | Licensed under BSD-3
*/
struct IGameComponent;
struct StringsBuffer;
extern struct IGameComponent Input_Component;

enum InputButtons {
	INPUT_NONE, /* Unrecognised key */

	CCKEY_F1, CCKEY_F2, CCKEY_F3, CCKEY_F4, CCKEY_F5, CCKEY_F6, CCKEY_F7, CCKEY_F8, CCKEY_F9, CCKEY_F10,
	CCKEY_F11, CCKEY_F12, CCKEY_F13, CCKEY_F14, CCKEY_F15, CCKEY_F16, CCKEY_F17, CCKEY_F18, CCKEY_F19, CCKEY_F20,
	CCKEY_F21, CCKEY_F22, CCKEY_F23, CCKEY_F24,

	CCKEY_TILDE, CCKEY_MINUS, CCKEY_EQUALS, CCKEY_LBRACKET, CCKEY_RBRACKET, CCKEY_SLASH,
	CCKEY_SEMICOLON, CCKEY_QUOTE, CCKEY_COMMA, CCKEY_PERIOD, CCKEY_BACKSLASH,

	CCKEY_LSHIFT, CCKEY_RSHIFT, CCKEY_LCTRL, CCKEY_RCTRL,
	CCKEY_LALT, CCKEY_RALT, CCKEY_LWIN, CCKEY_RWIN,

	CCKEY_UP, CCKEY_DOWN, CCKEY_LEFT, CCKEY_RIGHT,

	CCKEY_0, CCKEY_1, CCKEY_2, CCKEY_3, CCKEY_4,
	CCKEY_5, CCKEY_6, CCKEY_7, CCKEY_8, CCKEY_9, /* same as '0'-'9' */

	CCKEY_INSERT, CCKEY_DELETE, CCKEY_HOME, CCKEY_END, CCKEY_PAGEUP, CCKEY_PAGEDOWN,
	CCKEY_MENU,

	CCKEY_A, CCKEY_B, CCKEY_C, CCKEY_D, CCKEY_E, CCKEY_F, CCKEY_G, CCKEY_H, CCKEY_I, CCKEY_J,
	CCKEY_K, CCKEY_L, CCKEY_M, CCKEY_N, CCKEY_O, CCKEY_P, CCKEY_Q, CCKEY_R, CCKEY_S, CCKEY_T,
	CCKEY_U, CCKEY_V, CCKEY_W, CCKEY_X, CCKEY_Y, CCKEY_Z, /* same as 'A'-'Z' */

	CCKEY_ENTER, CCKEY_ESCAPE, CCKEY_SPACE, CCKEY_BACKSPACE, CCKEY_TAB, CCKEY_CAPSLOCK,
	CCKEY_SCROLLLOCK, CCKEY_PRINTSCREEN, CCKEY_PAUSE, CCKEY_NUMLOCK,

	CCKEY_KP0, CCKEY_KP1, CCKEY_KP2, CCKEY_KP3, CCKEY_KP4,
	CCKEY_KP5, CCKEY_KP6, CCKEY_KP7, CCKEY_KP8, CCKEY_KP9,
	CCKEY_KP_DIVIDE, CCKEY_KP_MULTIPLY, CCKEY_KP_MINUS,
	CCKEY_KP_PLUS, CCKEY_KP_DECIMAL, CCKEY_KP_ENTER,
	
	CCKEY_VOLUME_MUTE, CCKEY_VOLUME_UP, CCKEY_VOLUME_DOWN, CCKEY_SLEEP,
	CCKEY_MEDIA_NEXT, CCKEY_MEDIA_PREV, CCKEY_MEDIA_PLAY, CCKEY_MEDIA_STOP,
	CCKEY_BROWSER_PREV, CCKEY_BROWSER_NEXT, CCKEY_BROWSER_REFRESH, CCKEY_BROWSER_STOP, CCKEY_BROWSER_SEARCH, CCKEY_BROWSER_FAVORITES, CCKEY_BROWSER_HOME,
	CCKEY_LAUNCH_MAIL, CCKEY_LAUNCH_MEDIA, CCKEY_LAUNCH_APP1, CCKEY_LAUNCH_CALC, 

	/* NOTE: RMOUSE must be before MMOUSE for PlayerClick compatibility */
	CCMOUSE_X1, CCMOUSE_X2, CCMOUSE_X3, CCMOUSE_X4, CCMOUSE_X5, CCMOUSE_X6,
	CCMOUSE_L, CCMOUSE_R, CCMOUSE_M,
	CCWHEEL_UP, CCWHEEL_DOWN, CCWHEEL_LEFT, CCWHEEL_RIGHT,

	CCPAD_A, CCPAD_B, CCPAD_X, CCPAD_Y, CCPAD_L, CCPAD_R, 
	CCPAD_Z, CCPAD_C, CCPAD_D,
	CCPAD_LEFT, CCPAD_RIGHT, CCPAD_UP, CCPAD_DOWN,
	CCPAD_START, CCPAD_SELECT, CCPAD_ZL, CCPAD_ZR,
	CCPAD_LSTICK, CCPAD_RSTICK,
	CCPAD_CLEFT, CCPAD_CRIGHT, CCPAD_CUP, CCPAD_CDOWN,

	INPUT_COUNT,

	INPUT_CLIPBOARD_COPY  = 1001,
	INPUT_CLIPBOARD_PASTE = 1002
};
#define Input_IsPadButton(btn) ((btn) >= CCPAD_A && (btn) < INPUT_COUNT)

extern const char* const Input_DisplayNames[INPUT_COUNT];

extern struct _InputState {
	/* Pressed state of each input button. Use Input_Set to change */
	cc_bool Pressed[INPUT_COUNT];
	/* Whether raw mouse/touch input is currently being listened for */
	cc_bool RawMode;
	/* Sources available for input (Mouse/Keyboard, Gamepad) */
	cc_uint8 Sources;
} Input;

#define INPUT_SOURCE_NORMAL  (1 << 0)
#define INPUT_SOURCE_GAMEPAD (1 << 1)

/* Sets Input_Pressed[key] to true and raises InputEvents.Down */
void Input_SetPressed(int key);
/* Sets Input_Pressed[key] to false and raises InputEvents.Up */
void Input_SetReleased(int key);
/* Calls either Input_SetPressed or Input_SetReleased */
void Input_Set(int key, int pressed);
void Input_SetNonRepeatable(int key, int pressed);
/* Resets all input buttons to released state. (Input_SetReleased) */
void Input_Clear(void);


#define Input_IsWinPressed()   (Input.Pressed[CCKEY_LWIN]   || Input.Pressed[CCKEY_RWIN])
#define Input_IsAltPressed()   (Input.Pressed[CCKEY_LALT]   || Input.Pressed[CCKEY_RALT])
#define Input_IsCtrlPressed()  (Input.Pressed[CCKEY_LCTRL]  || Input.Pressed[CCKEY_RCTRL])
#define Input_IsShiftPressed() (Input.Pressed[CCKEY_LSHIFT] || Input.Pressed[CCKEY_RSHIFT])

#define Input_IsUpButton(btn)     ((btn) == CCKEY_UP     || (btn) == CCPAD_UP)
#define Input_IsDownButton(btn)   ((btn) == CCKEY_DOWN   || (btn) == CCPAD_DOWN)
#define Input_IsLeftButton(btn)   ((btn) == CCKEY_LEFT   || (btn) == CCPAD_LEFT)
#define Input_IsRightButton(btn)  ((btn) == CCKEY_RIGHT  || (btn) == CCPAD_RIGHT)

#define Input_IsEnterButton(btn)  ((btn) == CCKEY_ENTER  || (btn) == CCPAD_START || (btn) == CCKEY_KP_ENTER)
#define Input_IsPauseButton(btn)  ((btn) == CCKEY_ESCAPE || (btn) == CCPAD_START || (btn) == CCKEY_PAUSE)
#define Input_IsEscapeButton(btn) ((btn) == CCKEY_ESCAPE || (btn) == CCPAD_SELECT)

#if defined CC_BUILD_HAIKU
	/* Haiku uses ALT instead of CTRL for clipboard and stuff */
	#define Input_IsActionPressed() Input_IsAltPressed()
#elif defined CC_BUILD_DARWIN
	/* macOS uses CMD instead of CTRL for clipboard and stuff */
	#define Input_IsActionPressed() Input_IsWinPressed()
#else
	#define Input_IsActionPressed() Input_IsCtrlPressed()
#endif
int Input_CalcDelta(int btn, int horDelta, int verDelta);


#ifdef CC_BUILD_TOUCH
#define INPUT_MAX_POINTERS 32
enum INPUT_MODE { INPUT_MODE_PLACE, INPUT_MODE_DELETE, INPUT_MODE_NONE, INPUT_MODE_COUNT };

extern int Pointers_Count;
extern int Input_TapMode, Input_HoldMode;
/* Whether touch input is being used. */
extern cc_bool Input_TouchMode;
void Input_SetTouchMode(cc_bool enabled);

void Input_AddTouch(long id,    int x, int y);
void Input_UpdateTouch(long id, int x, int y);
void Input_RemoveTouch(long id, int x, int y);
#else
#define INPUT_MAX_POINTERS 1
#define Pointers_Count 1
#define Input_TouchMode false
#endif

/* Touch fingers are initially are 'all' type, meaning they could */
/*  trigger menu clicks, camera movement, or place/delete blocks  */
/* But for example, after clicking on a menu button, you wouldn't */
/*  want moving that finger anymore to move the camera */
#define TOUCH_TYPE_GUI    1
#define TOUCH_TYPE_CAMERA 2
#define TOUCH_TYPE_BLOCKS 4
#define TOUCH_TYPE_ALL (TOUCH_TYPE_GUI | TOUCH_TYPE_CAMERA | TOUCH_TYPE_BLOCKS)

/* Data for mouse and touch */
extern struct Pointer { int x, y; } Pointers[INPUT_MAX_POINTERS];
/* Raises appropriate events for a mouse vertical scroll */
void Mouse_ScrollVWheel(float delta);
/* Raises appropriate events for a mouse horizontal scroll */
void Mouse_ScrollHWheel(float delta);
/* Sets X and Y position of the given pointer, then raises appropriate events */
void Pointer_SetPosition(int idx, int x, int y);


/* Enumeration of all input bindings. */
enum InputBind_ {
	BIND_FORWARD, BIND_BACK, BIND_LEFT, BIND_RIGHT,
	BIND_JUMP, BIND_RESPAWN, BIND_SET_SPAWN, BIND_CHAT,
	BIND_INVENTORY, BIND_FOG, BIND_SEND_CHAT, BIND_TABLIST,
	BIND_SPEED, BIND_NOCLIP, BIND_FLY, BIND_FLY_UP, BIND_FLY_DOWN,
	BIND_EXT_INPUT, BIND_HIDE_FPS, BIND_SCREENSHOT, BIND_FULLSCREEN,
	BIND_THIRD_PERSON, BIND_HIDE_GUI, BIND_AXIS_LINES, BIND_ZOOM_SCROLL,
	BIND_HALF_SPEED, BIND_DELETE_BLOCK, BIND_PICK_BLOCK, BIND_PLACE_BLOCK,
	BIND_AUTOROTATE, BIND_HOTBAR_SWITCH, BIND_SMOOTH_CAMERA,
	BIND_DROP_BLOCK, BIND_IDOVERLAY, BIND_BREAK_LIQUIDS,
	BIND_LOOK_UP, BIND_LOOK_DOWN, BIND_LOOK_RIGHT, BIND_LOOK_LEFT,
	BIND_HOTBAR_1, BIND_HOTBAR_2, BIND_HOTBAR_3,
	BIND_HOTBAR_4, BIND_HOTBAR_5, BIND_HOTBAR_6,
	BIND_HOTBAR_7, BIND_HOTBAR_8, BIND_HOTBAR_9,
	BIND_HOTBAR_LEFT, BIND_HOTBAR_RIGHT,
	BIND_COUNT
};
typedef int InputBind;
typedef struct BindMapping_ { cc_uint8 button1, button2; } BindMapping;
#define BindMapping_Set(mapping, btn1, btn2) (mapping)->button1 = btn1; (mapping)->button2 = btn2;

/* The keyboard/mouse buttons that are bound to each input binding */
extern BindMapping KeyBind_Mappings[BIND_COUNT];
/* The gamepad buttons that are bound to each input binding */
extern BindMapping PadBind_Mappings[BIND_COUNT];
/* Default keyboard/mouse button that each input binding is bound to */
extern const BindMapping KeyBind_Defaults[BIND_COUNT];
/* Default gamepad button that each input binding is bound to */
extern const BindMapping PadBind_Defaults[BIND_COUNT];

/* Whether the given binding should be triggered in response to given input button being pressed */
CC_API cc_bool InputBind_Claims(InputBind binding, int btn);
/* Gets whether the given input binding is currently being triggered */
CC_API cc_bool InputBind_IsPressed(InputBind binding);

/* Sets the key/mouse button that the given input binding is bound to */
void KeyBind_Set(InputBind binding, int btn);
/* Sets the gamepad button that the given input binding is bound to */
void PadBind_Set(InputBind binding, int btn);
/* Resets the key/mouse button that the given input binding is bound to */
void KeyBind_Reset(InputBind binding);
/* Resets the gamepad button that the given input binding is bound to*/
void PadBind_Reset(InputBind binding);


/* Gamepad axes. Default behaviour is: */
/*  - left axis:  player movement  */
/*  - right axis: camera movement */
enum PAD_AXIS         { PAD_AXIS_LEFT, PAD_AXIS_RIGHT };
enum AXIS_SENSITIVITY { AXIS_SENSI_LOWER, AXIS_SENSI_LOW, AXIS_SENSI_NORMAL, AXIS_SENSI_HIGH, AXIS_SENSI_HIGHER };
enum AXIS_BEHAVIOUR   { AXIS_BEHAVIOUR_MOVEMENT, AXIS_BEHAVIOUR_CAMERA };
extern int Gamepad_AxisBehaviour[2];
extern int Gamepad_AxisSensitivity[2];

/* Sets value of the given gamepad button */
void Gamepad_SetButton(int port, int btn, int pressed);
/* Sets value of the given axis */
void Gamepad_SetAxis(int port, int axis, float x, float y, float delta);
void Gamepad_Tick(float delta);
#define INPUT_MAX_GAMEPADS 4


/* whether to leave text input open for user to enter further input */
#define HOTKEY_FLAG_STAYS_OPEN   0x01
/* Whether the hotkey was auto defined (e.g. by server) */
#define HOTKEY_FLAG_AUTO_DEFINED 0x02

extern const cc_uint8 Hotkeys_LWJGL[256];
struct HotkeyData {
	int textIndex;     /* contents to copy directly into the input bar */
	cc_uint8 trigger;  /* Member of Key enumeration */
	cc_uint8 mods;     /* HotkeyModifiers bitflags */
	cc_uint8 flags;    /* HOTKEY_FLAG flags */
};

#define HOTKEYS_MAX_COUNT 256
extern struct HotkeyData HotkeysList[HOTKEYS_MAX_COUNT];
extern struct StringsBuffer HotkeysText;
enum HotkeyModifiers {
	HOTKEY_MOD_CTRL = 1, HOTKEY_MOD_SHIFT = 2, HOTKEY_MOD_ALT = 4
};

/* Adds or updates a new hotkey. */
void Hotkeys_Add(int trigger, cc_uint8 modifiers, const cc_string* text, cc_uint8 flags);
/* Removes the given hotkey. */
cc_bool Hotkeys_Remove(int trigger, cc_uint8 modifiers);
/* Returns the first hotkey which is bound to the given key and has its modifiers pressed. */
/* NOTE: The hotkeys list is sorted, so hotkeys with most modifiers are checked first. */
int Hotkeys_FindPartial(int key);

/* Loads the given hotkey from options. (if it exists) */
void StoredHotkeys_Load(int trigger, cc_uint8 modifiers);
/* Removes the given hotkey from options. */
void StoredHotkeys_Remove(int trigger, cc_uint8 modifiers);
/* Adds the given hotkey from options. */
void StoredHotkeys_Add(int trigger, cc_uint8 modifiers, cc_bool moreInput, const cc_string* text);


cc_bool InputHandler_SetFOV(int fov);
cc_bool Input_HandleMouseWheel(float delta);
void InputHandler_Tick(void);
void InputHandler_OnScreensChanged(void);
void InputHandler_DeleteBlock(void);
void InputHandler_PlaceBlock(void);
void InputHandler_PickBlock(void);
#endif
