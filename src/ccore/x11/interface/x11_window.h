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
	Display *display;
	Window win;
	GLXContext context;
	XID cursor;
	Pixmap cursorimg;
	char *clipstr;
	size_t clipstrlen;
	Atom CCORE_SELECTION, WM_ICON, WM_ICON_NAME, WM_NAME, CLIPBOARD, INCR, TARGETS, MULTIPLE, UTF8_STRING, COMPOUND_STRING;
	int screen, winflags, inputopcode;
	bool resizable;
#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
	XImage *fb;
	XShmSegmentInfo shminfo;
	GC gc;
	int w, h;
#endif
} ccWindow_x11;

#define XD ((ccWindow_x11*)_ccWindow->data)
