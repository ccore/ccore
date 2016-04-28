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
#include <stdint.h>

#include "core.h"

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
	CC_FRAMEBUFFER_PIXEL_NONE = 0,
	CC_FRAMEBUFFER_PIXEL_BGR24 = 24,
	CC_FRAMEBUFFER_PIXEL_BGR32 = 32
} ccFramebufferFormat;
#endif

// The window struct
typedef struct {
	ccRect rect;
	ccPoint mouse;
	ccEvent event;
	ccDisplay *display;
	bool supportsRawInput;
#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
	void *pixels;
#endif

	void *data;
} ccWindow;

// Only access through getters
ccWindow *_ccWindow;

#define ccWindowSupportsRawInput() _ccWindow->supportsRawInput

// Window functions
ccError ccWindowCreate(ccRect rect, const char *title, int flags);
ccError ccWindowFree(void);
bool ccWindowEventPoll(void); // Poll an event from the events that currently need to be processed in the window
ccError ccWindowResizeMove(ccRect rect);
ccError ccWindowSetCentered(void);

ccError ccWindowSetWindowed(ccRect *rect);
ccError ccWindowSetMaximized(void);
ccError ccWindowSetFullscreen(int displayCount, ...);
ccError ccWindowSetTitle(const char *title);

ccError ccWindowSetBlink(void);
ccError ccWindowIconSet(ccPoint size, const uint32_t *icon);
ccError ccWindowMouseSetPosition(ccPoint target);
ccError ccWindowMouseSetCursor(ccCursor cursor);

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
// Create a framebuffer object, sets the format
ccError ccWindowFramebufferCreate(ccFramebufferFormat *format);
ccError ccWindowFramebufferUpdate();
ccError ccWindowFramebufferFree();
void *ccWindowFramebufferGetPixels();
#endif

ccError ccWindowClipboardSet(const char *data);
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
