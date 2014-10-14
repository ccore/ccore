#pragma once

#include <stdlib.h>
#include <Windows.h>
#include <string.h>
#include <hidsdi.h>
#include <stdint.h>

#include <ccore/window.h>

#include <ccore/string.h>
#include <ccore/assert.h>
#include <ccore/gamepad.h>

#include "../utils/win_file.h"
#include "win_gamepad.h"

#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
#define _CC_NRAWINPUTDEVICES 3
#define _CC_RAWINPUT_GAMEPAD 2
#define _CC_RAWINPUT_GAMEPADCOUNT 1
#else
#define _CC_NRAWINPUTDEVICES 2
#define _CC_RAWINPUT_GAMEPADCOUNT 0
#endif

#define _CC_RAWINPUT_KEYBOARD 0
#define _CC_RAWINPUT_MOUSE 1

typedef struct {
	HDC hdc;
	MSG msg;
	HWND winHandle;
	HGLRC renderContext;
	LONG style;
	RAWINPUTDEVICE rid[_CC_NRAWINPUTDEVICES];
	LPBYTE lpb;
	UINT lpbSize;
	UINT dwSize;
	ATOM winClass;
#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	bool queryXinput;
#endif

	ccEvent *eventStack;
	int eventStackSize;
	int eventStackPos;
	int eventStackIndex;

	ccCursor cursor;

	int flags;
} ccWindow_win;

void _ccEventStackPush(ccEvent event);

#define _CC_WINDOW_DATA ((ccWindow_win*)_ccWindow->data)