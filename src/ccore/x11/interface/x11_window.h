#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XInput2.h>

#include <ccore/opengl.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <ccore/window.h>
#include <ccore/gamepad.h>

#include <ccore/types.h>
#include <ccore/event.h>
#include <ccore/error.h>
#include <ccore/assert.h>
#include <ccore/print.h>

#ifdef LINUX
#include "../../linux/interface/lin_gamepad.h"
#endif

#include "x11_display.h"
#include "x11_text.h"

typedef struct {
	Display *XDisplay;
	Window XWindow;
	GLXContext XContext;
	XID XCursor;
	Pixmap XEmptyCursorImage;
	char *XClipString;
	size_t XClipStringLength;
	Atom CCORE_SELECTION, WM_ICON, WM_ICON_NAME, WM_NAME, CLIPBOARD, INCR,
			TARGETS, MULTIPLE, UTF8_STRING, COMPOUND_STRING;
	int XScreen, windowFlags, XInputOpcode;
	bool resizable;
} ccWindow_x11;

#define XWINDATA ((ccWindow_x11 *)_ccWindow->data)
