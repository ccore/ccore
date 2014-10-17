//__________________________________________________________________________________//
//                               ______                                             //
//                              /  ___ /__  _ __ ___                                //
//                             /  / __/ _ \| '__/ _ \                               //
//                            |  | (_| (_) | | |  __/                               //
//                             \  \___\___/|_|  \___| 1.1                           //
//                              \______\                                            //
//                                                                                  //
//             Copyright (C) 2014 \ Job Talle (job@ccore.org)                       //
//                                 \ Thomas Versteeg (thomas@ccore.org)             //
//__________________________________________________________________________________//
//                                                                                  //
//      This program is free software: you can redistribute it and/or modify        //
//      it under the terms of the 3-clause BSD license.                             //
//                                                                                  //
//      You should have received a copy of the 3-clause BSD License along with      //
//      this program. If not, see <http://opensource.org/licenses/>.                //
//__________________________________________________________________________________//

#pragma once

#include "core.h"

#include "types.h"
#include "gamepad.h"

#ifdef __cplusplus
extern "C"
{
#endif

// A type of event returned used in ccWindow after the function ccPollEvent is called
typedef enum {
	// Event not handled in ccore, return false on pollEvent
	CC_EVENT_SKIP,
	// A key is pressed, store keycode
	CC_EVENT_KEY_DOWN,
	// A key is released, store keycode
	CC_EVENT_KEY_UP,
	// Mouse has moved, store coordinates
	CC_EVENT_MOUSE_MOVE,
	// Mouse button down, store buttoncode
	CC_EVENT_MOUSE_DOWN,
	// Mouse button up, store buttoncode
	CC_EVENT_MOUSE_UP,
	// Mouse wheel moved up
	CC_EVENT_MOUSE_SCROLL,
	// The keyboard is focussed on the window
	CC_EVENT_FOCUS_GAINED,
	// The keyboard lost focus on the window
	CC_EVENT_FOCUS_LOST,
	// Window close button was pressed
	CC_EVENT_WINDOW_QUIT,
	// Window has resized, new dimensions are stored in the window struct before the event fires
	CC_EVENT_WINDOW_RESIZE,
#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	// Gamepad event, see gamepad.h for specifics
	CC_EVENT_GAMEPAD
#endif
} ccEventType;

// The type of mouse button pressed, used inside the ccEvent union
typedef enum {
	CC_MOUSE_BUTTON_NONE,
	CC_MOUSE_BUTTON_LEFT,
	CC_MOUSE_BUTTON_MIDDLE,
	CC_MOUSE_BUTTON_RIGHT,
	CC_MOUSE_BUTTON_PREVIOUS,
	CC_MOUSE_BUTTON_NEXT
} ccMouseButtonType;

// The event structure used by ccWindow, set by the call ccPollEvent
typedef struct {
	ccEventType type;

	// The union contains different data types for different events
	union {
		ccMouseButtonType mouseButton;
		ccPoint mouseDelta;
		unsigned int keyCode;
		double scrollDelta;
#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
		ccGamepadEvent gamepadEvent;
#endif
	};
} ccEvent;

#ifdef X11
#include <X11/keysym.h>
#define KEYDEF(x11,win) x11
#elif defined WINDOWS
#include <Windows.h>
#define KEYDEF(x11,win) win
#endif

#define CC_KEY_0 KEYDEF(XK_0, '0')
#define CC_KEY_1 KEYDEF(XK_1, '1')
#define CC_KEY_2 KEYDEF(XK_2, '2')
#define CC_KEY_3 KEYDEF(XK_3, '3')
#define CC_KEY_4 KEYDEF(XK_4, '4')
#define CC_KEY_5 KEYDEF(XK_5, '5')
#define CC_KEY_6 KEYDEF(XK_6, '6')
#define CC_KEY_7 KEYDEF(XK_7, '7')
#define CC_KEY_8 KEYDEF(XK_8, '8')
#define CC_KEY_9 KEYDEF(XK_9, '9')

#define CC_KEY_A KEYDEF(XK_a, 'A')
#define CC_KEY_B KEYDEF(XK_b, 'B')
#define CC_KEY_C KEYDEF(XK_c, 'C')
#define CC_KEY_D KEYDEF(XK_d, 'D')
#define CC_KEY_E KEYDEF(XK_e, 'E')
#define CC_KEY_F KEYDEF(XK_f, 'F')
#define CC_KEY_G KEYDEF(XK_g, 'G')
#define CC_KEY_H KEYDEF(XK_h, 'H')
#define CC_KEY_I KEYDEF(XK_i, 'I')
#define CC_KEY_J KEYDEF(XK_j, 'J')
#define CC_KEY_K KEYDEF(XK_k, 'K')
#define CC_KEY_L KEYDEF(XK_l, 'L')
#define CC_KEY_M KEYDEF(XK_m, 'M')
#define CC_KEY_N KEYDEF(XK_n, 'N')
#define CC_KEY_O KEYDEF(XK_o, 'O')
#define CC_KEY_P KEYDEF(XK_p, 'P')
#define CC_KEY_Q KEYDEF(XK_q, 'Q')
#define CC_KEY_R KEYDEF(XK_r, 'R')
#define CC_KEY_S KEYDEF(XK_s, 'S')
#define CC_KEY_T KEYDEF(XK_t, 'T')
#define CC_KEY_U KEYDEF(XK_u, 'U')
#define CC_KEY_V KEYDEF(XK_v, 'V')
#define CC_KEY_W KEYDEF(XK_w, 'W')
#define CC_KEY_X KEYDEF(XK_x, 'X')
#define CC_KEY_Y KEYDEF(XK_y, 'Y')
#define CC_KEY_Z KEYDEF(XK_z, 'Z')

#define CC_KEY_F1 KEYDEF(XK_F1, VK_F1)
#define CC_KEY_F2 KEYDEF(XK_F2, VK_F2)
#define CC_KEY_F3 KEYDEF(XK_F3, VK_F3)
#define CC_KEY_F4 KEYDEF(XK_F4, VK_F4)
#define CC_KEY_F5 KEYDEF(XK_F5, VK_F5)
#define CC_KEY_F6 KEYDEF(XK_F6, VK_F6)
#define CC_KEY_F7 KEYDEF(XK_F7, VK_F7)
#define CC_KEY_F8 KEYDEF(XK_F8, VK_F8)
#define CC_KEY_F9 KEYDEF(XK_F9, VK_F9)
#define CC_KEY_F10 KEYDEF(XK_F10, VK_F10)
#define CC_KEY_F11 KEYDEF(XK_F11, VK_F11)
#define CC_KEY_F12 KEYDEF(XK_F12, VK_F12)

#define CC_KEY_NUM0 KEYDEF(XK_KP_0, VK_NUMPAD0)
#define CC_KEY_NUM1 KEYDEF(XK_KP_1, VK_NUMPAD1)
#define CC_KEY_NUM2 KEYDEF(XK_KP_2, VK_NUMPAD2)
#define CC_KEY_NUM3 KEYDEF(XK_KP_3, VK_NUMPAD3)
#define CC_KEY_NUM4 KEYDEF(XK_KP_4, VK_NUMPAD4)
#define CC_KEY_NUM5 KEYDEF(XK_KP_5, VK_NUMPAD5)
#define CC_KEY_NUM6 KEYDEF(XK_KP_6, VK_NUMPAD6)
#define CC_KEY_NUM7 KEYDEF(XK_KP_7, VK_NUMPAD7)
#define CC_KEY_NUM8 KEYDEF(XK_KP_8, VK_NUMPAD8)
#define CC_KEY_NUM9 KEYDEF(XK_KP_9, VK_NUMPAD9)

#define CC_KEY_BACKSPACE KEYDEF(XK_BackSpace, VK_BACK)
#define CC_KEY_TAB KEYDEF(XK_Tab, VK_TAB)
#define CC_KEY_RETURN KEYDEF(XK_Return, VK_RETURN)
#define CC_KEY_ESCAPE KEYDEF(XK_Escape, VK_ESCAPE)
#define CC_KEY_SPACE KEYDEF(XK_space, VK_SPACE)
#define CC_KEY_CAPSLOCK KEYDEF(XK_Caps_Lock, VK_CAPITAL)
#define CC_KEY_INSERT KEYDEF(XK_Insert, VK_INSERT)
#define CC_KEY_DELETE KEYDEF(XK_Delete, VK_DELETE)
#define CC_KEY_HOME KEYDEF(XK_Home, VK_HOME)
#define CC_KEY_END KEYDEF(XK_End, VK_END)
#define CC_KEY_PAGEUP KEYDEF(XK_Page_Up, VK_PRIOR)
#define CC_KEY_PAGEDOWN KEYDEF(XK_Page_Down, VK_NEXT)
#define CC_KEY_PRINTSCREEN KEYDEF(XK_Print, VK_SNAPSHOT)
#define CC_KEY_SCROLLLOCK KEYDEF(XK_Scroll_Lock, VK_SCROLL)
#define CC_KEY_NUMLOCK KEYDEF(XK_Num_Lock, VK_NUMLOCK)
#define CC_KEY_PAUSEBREAK KEYDEF(XK_Pause, VK_PAUSE)

#define CC_KEY_LSHIFT KEYDEF(XK_Shift_L, VK_LSHIFT)
#define CC_KEY_RSHIFT KEYDEF(XK_Shift_R, VK_RSHIFT)
#define CC_KEY_LCONTROL KEYDEF(XK_Control_L, VK_LCONTROL)
#define CC_KEY_RCONTROL KEYDEF(XK_Control_R, VK_RCONTROL)

#define CC_KEY_LEFT KEYDEF(XK_Left, VK_LEFT)
#define CC_KEY_UP KEYDEF(XK_Up, VK_UP)
#define CC_KEY_RIGHT KEYDEF(XK_Right, VK_RIGHT)
#define CC_KEY_DOWN KEYDEF(XK_Down, VK_DOWN)

char ccEventKeyToChar(int key);
char *ccEventKeyToStr(int key);

#ifdef __cplusplus
}
#endif
