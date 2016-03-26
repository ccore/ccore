#pragma once

#include <stdbool.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XInput2.h>

#include <GL/glx.h>

#ifdef LINUX
#include "../../linux/interface/lin_gamepad.h"
#endif

typedef struct {
	Display *XDisplay;
	Window XWindow;
	GLXContext XContext;
	XID XCursor;
	Pixmap XEmptyCursorImage;
	XImage *XFramebuffer;
	char *XClipString;
	size_t XClipStringLength;
	Atom CCORE_SELECTION, WM_ICON, WM_ICON_NAME, WM_NAME, CLIPBOARD, INCR,
			TARGETS, MULTIPLE, UTF8_STRING, COMPOUND_STRING;
	int XScreen, windowFlags, XInputOpcode;
	bool resizable;
} ccWindow_x11;

#define XWINDATA ((ccWindow_x11 *)_ccWindow->data)
