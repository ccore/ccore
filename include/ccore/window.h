//__________________________________________________________________________________//
//                               ______                                             //
//                              /  ___ /__  _ __ ___                                //
//                             /  / __/ _ \| '__/ _ \                               //
//                            |  | (_| (_) | | |  __/                               //
//                             \  \___\___/|_|  \___| 1.1                           //
//                              \______\                                            //
//                                                                                  //
//             Copyright (C) 2014 \ Job Talle (jobtalle@hotmail.com)                //
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

#include <stdarg.h>

#include "core.h"

#include "error.h"
#include "display.h"
#include "types.h"
#include "event.h"

#define CC_FULLSCREEN_CURRENT_DISPLAY 0

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
	CC_CURSOR_ARROW,
	CC_CURSOR_CROSS,
	CC_CURSOR_BEAM,
	CC_CURSOR_MOVE,
	CC_CURSOR_HAND,
	CC_CURSOR_SIZEH,
	CC_CURSOR_SIZEV,
	CC_CURSOR_NO,
	CC_CURSOR_QUESTION,
	CC_CURSOR_NONE
} ccCursor;

// A window can contain multiple flags to determine the layout and functionality
typedef enum {
	CC_WINDOW_FLAG_NORESIZE = 1,
	CC_WINDOW_FLAG_ALWAYSONTOP = 2,
	CC_WINDOW_FLAG_NOBUTTONS = 4,
	CC_WINDOW_FLAG_NORAWINPUT = 8
} ccWindowFlag;

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
typedef enum {
	CC_FB_CHAR_RGB,
	CC_FB_CHAR_RGBA,
	CC_FB_INT_RGB,
	CC_FB_INT_RGBA,
} ccFramebufferFormat;
#endif

// The window struct
typedef struct {
	ccRect rect;
	ccPoint mouse;
	ccEvent event;
	ccDisplay *display;
	bool supportsRawInput;

	void *data;
} ccWindow;

// Only access through getters
ccWindow *_ccWindow;

#define ccWindowSupportsRawInput() _ccWindow->supportsRawInput

// Window functions
ccReturn ccWindowCreate(ccRect rect, const char *title, int flags);
ccReturn ccWindowFree(void);
bool ccWindowEventPoll(void); // Poll an event from the events that currently need to be processed in the window
ccReturn ccWindowResizeMove(ccRect rect);
ccReturn ccWindowSetCentered(void);

ccReturn ccWindowSetWindowed(ccRect *rect);
ccReturn ccWindowSetMaximized(void);
ccReturn ccWindowSetFullscreen(int displayCount, ...);
ccReturn ccWindowSetTitle(const char *title);

ccReturn ccWindowSetBlink(void);
ccReturn ccWindowIconSet(ccPoint size, unsigned long *icon);
ccReturn ccWindowMouseSetPosition(ccPoint target);
ccReturn ccWindowMouseSetCursor(ccCursor cursor);

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
ccReturn ccWindowFramebufferCreate(void **pixels, ccFramebufferFormat format);
ccReturn ccWindowFramebufferUpdate();
ccReturn ccWindowFramebufferFree();
#endif

ccReturn ccWindowClipboardSet(const char *data);
char *ccWindowClipboardGet(void);

// Getters
ccEvent ccWindowEventGet(void);
ccRect ccWindowGetRect(void);
ccPoint ccWindowGetMouse(void);
ccDisplay *ccWindowGetDisplay(void);
bool ccWindowExists(void);

// Usually for internal use only, finds the display the window currently is in
void ccWindowUpdateDisplay(void);

#ifdef __cplusplus
}
#endif
