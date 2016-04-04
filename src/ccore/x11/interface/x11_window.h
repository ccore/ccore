#pragma once

#include <stdbool.h>

#include <ccore/core.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XInput2.h>

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
#ifndef LINUX
#error "Shared memory libraries are needed to create a framebuffer object"
#endif
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

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
#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
	XImage *XFramebuffer;
	XShmSegmentInfo XShminfo;
	GC XGc;
#endif
	char *XClipString;
	size_t XClipStringLength;
	Atom CCORE_SELECTION, WM_ICON, WM_ICON_NAME, WM_NAME, CLIPBOARD, INCR,
			TARGETS, MULTIPLE, UTF8_STRING, COMPOUND_STRING;
	int XScreen, windowFlags, XInputOpcode;
	bool resizable;
} ccWindow_x11;

#define XWINDATA ((ccWindow_x11 *)_ccWindow->data)
