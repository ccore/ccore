#include "x11_window.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include <ccore/window.h>
#include <ccore/gamepad.h>
#include <ccore/opengl.h>
#include <ccore/types.h>
#include <ccore/event.h>
#include <ccore/error.h>
#include <ccore/assert.h>
#include <ccore/print.h>

#include "x11_display.h"

static int cursorList[] = {XC_arrow,
													 XC_crosshair,
													 XC_xterm,
													 XC_fleur,
													 XC_hand1,
													 XC_sb_h_double_arrow,
													 XC_sb_v_double_arrow,
													 XC_X_cursor,
													 XC_question_arrow};

static char emptyCursorData[] = {0, 0, 0, 0, 0, 0, 0, 0};

static ccReturn setWindowState(const char *type, bool value)
{
	Atom wmState = XInternAtom(XD->display, "_NET_WM_STATE", 1);
	Atom newWmState = XInternAtom(XD->display, type, 1);

	XEvent event = {0};
	event.type = ClientMessage;
	event.xclient.window = XD->win;
	event.xclient.message_type = wmState;
	event.xclient.format = 32;
	event.xclient.data.l[0] = value;
	event.xclient.data.l[1] = newWmState;

	XSendEvent(XD->display, DefaultRootWindow(XD->display), false, SubstructureNotifyMask, &event);

	return CC_SUCCESS;
}

static ccReturn setResizable(bool resizable)
{
	ccAssert(_ccWindow != NULL);

	XSizeHints *sizeHints = XAllocSizeHints();
	long flags = 0;

	if(XD->resizable == resizable) {
		XFree(sizeHints);
		return CC_SUCCESS;
	}

	XD->resizable = resizable;

	XGetWMNormalHints(XD->display, XD->win, sizeHints, &flags);

	if(resizable) {
		sizeHints->flags &= ~(PMinSize | PMaxSize);
	} else {
		sizeHints->flags |= PMinSize | PMaxSize;
		sizeHints->min_width = sizeHints->max_width = _ccWindow->rect.width;
		sizeHints->min_height = sizeHints->max_height = _ccWindow->rect.height;
	}

	XSetWMNormalHints(XD->display, XD->win, sizeHints);

	XFree(sizeHints);

	return CC_SUCCESS;
}

static ccReturn checkRawSupport()
{
	int event, error;
	if(CC_UNLIKELY(!XQueryExtension(XD->display, "XInputExtension", &XD->inputopcode, &event, &error))) {
		return CC_FAIL;
	}

	int mayor = 2;
	int minor = 0;
	if(CC_UNLIKELY(XIQueryVersion(XD->display, &mayor, &minor) == BadRequest)) {
		return CC_FAIL;
	}

	return CC_SUCCESS;
}

static ccReturn initRawSupport()
{
	XIEventMask mask = {.deviceid = XIAllMasterDevices, .mask_len = XIMaskLen(XI_RawMotion)};
	ccCalloc(mask.mask, mask.mask_len, sizeof(char));

	XISetMask(mask.mask, XI_Enter);
	XISetMask(mask.mask, XI_Leave);
	XISetMask(mask.mask, XI_ButtonPress);
	XISetMask(mask.mask, XI_ButtonRelease);
	XISetMask(mask.mask, XI_KeyPress);
	XISetMask(mask.mask, XI_KeyRelease);

	XISelectEvents(XD->display, XD->win, &mask, 1);

	mask.deviceid = XIAllDevices;
	memset(mask.mask, 0, mask.mask_len);
	XISetMask(mask.mask, XI_RawMotion);
	XISetMask(mask.mask, XI_RawButtonPress);
	XISetMask(mask.mask, XI_RawButtonRelease);
	XISetMask(mask.mask, XI_RawKeyPress);
	XISetMask(mask.mask, XI_RawKeyRelease);

	XISelectEvents( XD->display, DefaultRootWindow(XD->display), &mask, 1);

	free(mask.mask);

	return CC_SUCCESS;
}

static ccPoint getRawMouseMovement(XIRawEvent *event)
{
	ccPoint delta = {0};
	double *rawValuator = event->raw_values;
	double *valuator = event->valuators.values;

	int i;
	for(i = 0; i < event->valuators.mask_len * 8; i++) {
		if(XIMaskIsSet(event->valuators.mask, i)) {
			if(i == 0) {
				delta.x += *valuator - *rawValuator;
			} else if(i == 1) {
				delta.y += *valuator - *rawValuator;
			}
			valuator++;
			rawValuator++;
		}
	}

	return delta;
}

static inline unsigned int getRawKeyboardCode(XIRawEvent *event)
{
	return XGetKeyboardMapping( XD->display, event->detail, 1, (int[]){1})[0];
}

static int (*origXError)(Display*, XErrorEvent*);
static int handleXError(Display *display, XErrorEvent *event)
{
	ccErrorPush(CC_ERROR_WM);

	char error[256];
	XGetErrorText(XD->display, event->error_code, error, sizeof(error));
	ccPrintf("X message: %s\n", error);

	return 0;
}

static int _xerrflag = 0;
static int handleXErrorSetFlag(Display *display, XErrorEvent *event)
{
	_xerrflag = 1;
	return 0;
}

static unsigned long getWindowProperty(Window window, Atom property, Atom type, unsigned char **value)
{
	Atom actualType;
	int actualFormat;
	unsigned long length, overflow;
	XGetWindowProperty(XD->display, window, property, 0, LONG_MAX, False, type, &actualType, &actualFormat, &length, &overflow, value);

	if(type != AnyPropertyType && type != actualType) {
		return 0;
	}

	return length;
}

static bool handleSelectionRequest(XSelectionRequestEvent *request)
{
	const Atom formats[] = {XD->UTF8_STRING, XD->COMPOUND_STRING, XA_STRING};
	const Atom targets[] = {XD->TARGETS, XD->MULTIPLE, XD->UTF8_STRING, XD->COMPOUND_STRING, XA_STRING};
	const int formatCount = sizeof(formats) / sizeof(formats[0]);

	if(request->property == None) {
		// The requestor is a legacy client, we don't handle it
		return false;
	}

	XSelectionEvent event = {.type = SelectionNotify, .selection = request->selection, .target = request->target, .display = request->display, .requestor = request->requestor, .time = request->time};
	if(request->target == XD->TARGETS) {
		XChangeProperty(XD->display, request->requestor, request->property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, sizeof(targets) / sizeof(targets[0]));

		event.property = request->property;
	} else if(request->target == XD->MULTIPLE) {
		// TODO implement this and save target
		ccPrintf( "X handleSelectionRequest: multiple targets not implemented yet!\n");

		event.property = None;
	} else {
		event.property = None;
		int i;
		for(i = 0; i < formatCount; i++) {
			if(request->target == formats[i]) {
				XChangeProperty(XD->display, request->requestor, request->property, request->target, 8, PropModeReplace, (unsigned char*)XD->clipstr, XD->clipstrlen);

				event.property = request->property;
				break;
			}
		}
	}

	XSendEvent(XD->display, request->requestor, False, 0, (XEvent*)&event);

	return true;
}

ccReturn ccWindowCreate(ccRect rect, const char *title, int flags)
{
	ccAssert(rect.width > 0 && rect.height > 0);

	if(CC_UNLIKELY(_ccWindow != NULL)) {
		ccErrorPush(CC_ERROR_WINDOW_CREATE);
		return CC_FAIL;
	}

	ccMalloc(_ccWindow, sizeof(ccWindow));

	_ccWindow->rect = rect;
	ccCalloc(_ccWindow->data, 1, sizeof(ccWindow_x11));
	XD->winflags = flags;

	origXError = XSetErrorHandler(handleXError);

	XD->display = XOpenDisplay(NULL);
	if(CC_UNLIKELY(XD->display == NULL)) {
		ccErrorPush(CC_ERROR_WM);
		return CC_FAIL;
	}

	XD->screen = DefaultScreen(XD->display);

	Atom WM_WINDOW_TYPE = XInternAtom(XD->display, "_NET_WM_WINDOW_TYPE", False);
	Atom WM_WINDOW_TYPE_DIALOG = XInternAtom(XD->display, "_NET_WM_WINDOW_TYPE_DOCK", False);
	XD->WM_ICON = XInternAtom(XD->display, "_NET_WM_ICON", False);
	XD->WM_NAME = XInternAtom(XD->display, "_NET_WM_NAME", False);
	XD->WM_ICON_NAME = XInternAtom(XD->display, "_NET_WM_ICON_NAME", False);
	XD->CLIPBOARD = XInternAtom(XD->display, "CLIPBOARD", False);
	XD->INCR = XInternAtom(XD->display, "INCR", False);
	XD->TARGETS = XInternAtom(XD->display, "TARGETS", False);
	XD->MULTIPLE = XInternAtom(XD->display, "MULTIPLE", False);
	XD->UTF8_STRING = XInternAtom(XD->display, "UTF8_STRING", False);
	XD->COMPOUND_STRING = XInternAtom(XD->display, "COMPOUND_STRING", False);
	XD->CCORE_SELECTION = XInternAtom(XD->display, "CCORE_SELECTION", False);
	Atom DELETE = XInternAtom(XD->display, "WM_DELETE_WINDOW", True);

	XD->win = XCreateWindow(XD->display, RootWindow(XD->display, XD->screen), rect.x, rect.y, rect.width, rect.height, 0, CopyFromParent, InputOutput, CopyFromParent, 0, 0);

	// Choose types of events
	XSelectInput(XD->display, XD->win, PropertyChangeMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | FocusChangeMask);

	// Disable resizing
	XD->resizable = true;
	if(flags & CC_WINDOW_FLAG_NORESIZE) {
		setResizable(false);
	}

	if(flags & CC_WINDOW_FLAG_NOBUTTONS) {
		XChangeProperty(XD->display, XD->win, WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*)&WM_WINDOW_TYPE_DIALOG, 1);
	}

	XMapWindow(XD->display, XD->win);
	XStoreName(XD->display, XD->win, title);

	XSetWMProtocols(XD->display, XD->win, &DELETE, 1);

	ccWindowUpdateDisplay();

	// Activate always on top if applicable
	if(flags & CC_WINDOW_FLAG_ALWAYSONTOP) {
		setWindowState("_NET_WM_STATE_ABOVE", true);
	}

	if((flags & CC_WINDOW_FLAG_NORAWINPUT) != 0) {
		_ccWindow->supportsRawInput = checkRawSupport();
		if(_ccWindow->supportsRawInput) {
			initRawSupport();
		}
	} else {
		_ccWindow->supportsRawInput = false;
	}

	XD->cursorimg = XCreateBitmapFromData(XD->display, XD->win, emptyCursorData, 8, 8);

	_ccWindow->mouse.x = _ccWindow->mouse.y = 0;

	return CC_SUCCESS;
}

ccReturn ccWindowFree(void)
{
	ccAssert(_ccWindow);

	XSetErrorHandler(origXError);

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
	ccWindowFramebufferFree();
#endif

	if(XD->cursor != 0) {
		XFreeCursor(XD->display, XD->cursor);
	}
	XFreePixmap(XD->display, XD->cursorimg);
	XUnmapWindow(XD->display, XD->win);
	XCloseDisplay(XD->display);

	free(_ccWindow->data);
	free(_ccWindow);
	_ccWindow = NULL;

	return CC_SUCCESS;
}

bool ccWindowEventPoll(void)
{
	ccAssert(_ccWindow);

	_ccWindow->event.type = CC_EVENT_SKIP;

#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	if(_ccGamepads != NULL) {
		ccGamepadEvent gamepadEvent = ccGamepadEventPoll();
		if(gamepadEvent.type != CC_GAMEPAD_UNHANDLED) {
			_ccWindow->event.type = CC_EVENT_GAMEPAD;
			_ccWindow->event.gamepadEvent = gamepadEvent;
			return true;
		}
	}
#endif

	if(XPending(XD->display) == 0) {
		return false;
	}

	XEvent event;
	XNextEvent(XD->display, &event);
	switch(event.type) {
		case GenericEvent:
			if(CC_UNLIKELY(!_ccWindow->supportsRawInput)) {
				return false;
			}

			XGenericEventCookie *cookie = &event.xcookie;
			if(!XGetEventData(XD->display, cookie) || cookie->type != GenericEvent || cookie->extension != XD->inputopcode) {
				return false;
			}

			switch(cookie->evtype) {
				case XI_RawMotion:
					_ccWindow->event.type = CC_EVENT_MOUSE_MOVE;
					_ccWindow->event.mouseDelta = getRawMouseMovement(cookie->data);
					break;
				case XI_RawButtonPress:
					_ccWindow->event.type = CC_EVENT_MOUSE_DOWN;
					_ccWindow->event.mouseButton = ((XIRawEvent*)cookie->data)->detail;
					break;
				case XI_RawButtonRelease:
					_ccWindow->event.type = CC_EVENT_MOUSE_UP;
					_ccWindow->event.mouseButton = ((XIRawEvent*)cookie->data)->detail;
					break;
				case XI_RawKeyPress:
					_ccWindow->event.type = CC_EVENT_KEY_DOWN;
					_ccWindow->event.keyCode = getRawKeyboardCode(cookie->data);
					break;
				case XI_RawKeyRelease:
					_ccWindow->event.type = CC_EVENT_KEY_UP;
					_ccWindow->event.keyCode = getRawKeyboardCode(cookie->data);
					break;
				case XI_Enter:
					_ccWindow->event.type = CC_EVENT_FOCUS_GAINED;
					break;
				case XI_Leave:
					_ccWindow->event.type = CC_EVENT_FOCUS_LOST;
					break;
			}
			break;
		case ButtonPress:
			if(event.xbutton.button <= 3) {
				_ccWindow->event.type = CC_EVENT_MOUSE_DOWN;
				_ccWindow->event.mouseButton = event.xbutton.button;
			} else if(event.xbutton.button == 4) {
				_ccWindow->event.type = CC_EVENT_MOUSE_SCROLL;
				_ccWindow->event.scrollDelta = 1;
			} else if(event.xbutton.button == 5) {
				_ccWindow->event.type = CC_EVENT_MOUSE_SCROLL;
				_ccWindow->event.scrollDelta = -1;
			}
			break;
		case ButtonRelease:
			_ccWindow->event.type = CC_EVENT_MOUSE_UP;
			_ccWindow->event.mouseButton = event.xbutton.button;
			break;
		case MotionNotify:
			if(CC_UNLIKELY(!_ccWindow->supportsRawInput)) {
				_ccWindow->event.type = CC_EVENT_MOUSE_MOVE;
				_ccWindow->event.mouseDelta.x = _ccWindow->mouse.x - event.xmotion.x;
				_ccWindow->event.mouseDelta.y = _ccWindow->mouse.y - event.xmotion.y;
			}
			if(CC_LIKELY(_ccWindow->mouse.x != event.xmotion.x || _ccWindow->mouse.y != event.xmotion.y)) {
				_ccWindow->event.type = CC_EVENT_MOUSE_MOVE;
				_ccWindow->mouse.x = event.xmotion.x;
				_ccWindow->mouse.y = event.xmotion.y;
			}
			break;
		case KeymapNotify:
			XRefreshKeyboardMapping(&event.xmapping);
			break;
		case KeyPress:
			_ccWindow->event.type = CC_EVENT_KEY_DOWN;
			_ccWindow->event.keyCode = XLookupKeysym(&event.xkey, 0);
			break;
		case KeyRelease:
			_ccWindow->event.type = CC_EVENT_KEY_UP;
			_ccWindow->event.keyCode = XLookupKeysym(&event.xkey, 0);
			break;
		case ConfigureNotify:
			if(_ccWindow->rect.width != event.xconfigure.width || _ccWindow->rect.height != event.xconfigure.height) {
				_ccWindow->event.type = CC_EVENT_WINDOW_RESIZE;
				_ccWindow->rect.width = event.xconfigure.width;
				_ccWindow->rect.height = event.xconfigure.height;

				XWindowAttributes _ccWindowAttributes;
				XGetWindowAttributes(XD->display, XD->win, &_ccWindowAttributes);

				_ccWindow->rect.x = _ccWindowAttributes.x;
				_ccWindow->rect.y = _ccWindowAttributes.y;

				ccWindowUpdateDisplay();

				if(XD->winflags & CC_WINDOW_FLAG_NORESIZE) {
					setResizable(false);
				}

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
				if(XD->fb != NULL){
					ccWindowFramebufferFree();

					ccFramebufferFormat format;
					ccWindowFramebufferCreate(&format);
				}
#endif
			}

			if(_ccWindow->rect.x != event.xconfigure.x || _ccWindow->rect.y != event.xconfigure.y) {
				_ccWindow->rect.x = event.xconfigure.x;
				_ccWindow->rect.y = event.xconfigure.y;

				ccWindowUpdateDisplay();
			}
			break;
		case ClientMessage:
			_ccWindow->event.type = CC_EVENT_WINDOW_QUIT;
			break;
		case FocusIn:
			_ccWindow->event.type = CC_EVENT_FOCUS_GAINED;
			break;
		case FocusOut:
			_ccWindow->event.type = CC_EVENT_FOCUS_LOST;
			break;
		case SelectionClear:
			if(XD->clipstr) {
				free(XD->clipstr);
				XD->clipstr = NULL;
				XD->clipstrlen = 0;
			}
			return false;
		case SelectionRequest:
			handleSelectionRequest(&event.xselectionrequest);
			return false;
		default:
			return false;
	}

	return true;
}

ccReturn ccWindowSetWindowed(ccRect *rect)
{
	ccAssert(_ccWindow);

	setResizable(true);
	setWindowState("_NET_WM_STATE_FULLSCREEN", false);
	setWindowState("_NET_WM_STATE_MAXIMIZED_VERT", false);
	setWindowState("_NET_WM_STATE_MAXIMIZED_HORZ", false);

	if(rect == NULL) {
		return CC_SUCCESS;
	} else {
		return ccWindowResizeMove(*rect);
	}
}

ccReturn ccWindowSetMaximized(void)
{
	ccAssert(_ccWindow);

	ccWindowSetWindowed(NULL);

	setWindowState("_NET_WM_STATE_MAXIMIZED_VERT", true);
	setWindowState("_NET_WM_STATE_MAXIMIZED_HORZ", true);

	return CC_SUCCESS;
}

ccReturn ccWindowSetFullscreen(int displayCount, ...)
{
	ccAssert(_ccWindow);

	ccDisplay *current, *topDisplay, *bottomDisplay, *leftDisplay, *rightDisplay;
	if(CC_LIKELY(displayCount == CC_FULLSCREEN_CURRENT_DISPLAY)) {
		ccDisplay *topDisplay = bottomDisplay = leftDisplay = rightDisplay = _ccWindow->display;
	} else {
		va_list displays;
		va_start(displays, displayCount);

		topDisplay = bottomDisplay = leftDisplay = rightDisplay = va_arg(displays, ccDisplay*);

		int i;
		for(i = 1; i < displayCount; i++) {
			current = va_arg(displays, ccDisplay*);

			if(current->x < leftDisplay->x) {
				leftDisplay = current;
			}
			if(current->y < topDisplay->y) {
				topDisplay = current;
			}
			if(current->x + ccDisplayResolutionGetCurrent(current)->width > rightDisplay->x + ccDisplayResolutionGetCurrent(rightDisplay)->width) {
				rightDisplay = current;
			}
			if(current->y + ccDisplayResolutionGetCurrent(current)->height > bottomDisplay->y + ccDisplayResolutionGetCurrent(bottomDisplay)->width) {
				bottomDisplay = current;
			}
		}

		va_end(displays);
	}

	Atom atom = XInternAtom(XD->display, "_NET_WM_FULLSCREEN_MONITORS", False);

	XEvent event = {0};
	event.type = ClientMessage;
	event.xclient.window = XD->win;
	event.xclient.message_type = atom;
	event.xclient.format = 32;
	event.xclient.data.l[0] = DISPLAY_DATA(topDisplay)->XineramaScreen;
	event.xclient.data.l[1] = DISPLAY_DATA(bottomDisplay)->XineramaScreen;
	event.xclient.data.l[2] = DISPLAY_DATA(leftDisplay)->XineramaScreen;
	event.xclient.data.l[3] = DISPLAY_DATA(rightDisplay)->XineramaScreen;
	event.xclient.data.l[4] = 0;

	XSendEvent(XD->display, DefaultRootWindow(XD->display), false, SubstructureNotifyMask, &event);

	setResizable(true);
	setWindowState("_NET_WM_STATE_FULLSCREEN", true);

	return CC_SUCCESS;
}

ccReturn ccWindowResizeMove(ccRect rect)
{
	ccAssert(_ccWindow);
	ccAssert(rect.width > 0 && rect.height > 0);

	setResizable(true);
	XMoveResizeWindow(XD->display, XD->win, rect.x, rect.y, rect.width, rect.height);

	if(XD->winflags & CC_WINDOW_FLAG_NORESIZE) {
		setResizable(false);
	}

	ccWindowUpdateDisplay();

	return CC_SUCCESS;
}

ccReturn ccWindowSetCentered(void)
{
	ccAssert(_ccWindow);
	if(CC_UNLIKELY(_ccWindow->display == NULL)) {
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	ccDisplayData *currentResolution = ccDisplayResolutionGetCurrent(_ccWindow->display);

	ccRect newRect = { .x = (currentResolution->width - _ccWindow->rect.width) >> 1, .y = (currentResolution->height - _ccWindow->rect.height) >> 1, .width = _ccWindow->rect.width, .height = _ccWindow->rect.height};

	ccWindowResizeMove(newRect);

	return CC_SUCCESS;
}

ccReturn ccWindowSetBlink(void)
{
	ccAssert(_ccWindow);

	return setWindowState("_NET_WM_STATE_DEMANDS_ATTENTION", true);
}

ccReturn ccWindowSetTitle(const char *title)
{
	char *titleCopy = strdup(title);

	XTextProperty titleProperty;
	if(!XStringListToTextProperty(&titleCopy, 1, &titleProperty)) {
		ccErrorPush(CC_ERROR_WINDOW_MODE);
		return CC_FAIL;
	}
	free(titleCopy);

	XSetTextProperty( XD->display, XD->win, &titleProperty, XD->WM_NAME);
	XSetTextProperty(XD->display, XD->win, &titleProperty, XD->WM_ICON_NAME);
	XSetWMName(XD->display, XD->win, &titleProperty);
	XSetWMIconName(XD->display, XD->win, &titleProperty);
	XFree(titleProperty.value);

	return CC_SUCCESS;
}

ccReturn ccWindowIconSet(ccPoint size, unsigned long *icon)
{
	ccAssert(_ccWindow);

	if(size.x <= 0 || size.y <= 0) {
		ccErrorPush(CC_ERROR_WINDOW_MODE);
		return CC_FAIL;
	}

	size_t dataLen = size.x * size.y;
	size_t totalLen = dataLen + 2;
	unsigned long *data;
	ccMalloc(data, totalLen * sizeof(unsigned long));

	data[0] = (unsigned long)size.x;
	data[1] = (unsigned long)size.y;
	memcpy(data + 2, icon, dataLen * sizeof(unsigned long));

	XChangeProperty(XD->display, XD->win, XD->WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)data, totalLen);

	free(data);

	return CC_SUCCESS;
}

ccReturn ccWindowMouseSetPosition(ccPoint target)
{
	ccAssert(_ccWindow);

	XWarpPointer(XD->display, None, XD->win, 0, 0, 0, 0, target.x, target.y);

	return CC_SUCCESS;
}

ccReturn ccWindowMouseSetCursor(ccCursor cursor)
{
	ccAssert(_ccWindow);

	if(XD->cursor != 0) {
		XFreeCursor(XD->display, XD->cursor);
	}

	if(cursor != CC_CURSOR_NONE) {
		XD->cursor = XCreateFontCursor(XD->display, cursorList[cursor]);
		if(!XD->cursor) {
			ccErrorPush(CC_ERROR_WINDOW_CURSOR);
		}
	} else {
		XColor black = {0};
		XD->cursor = XCreatePixmapCursor(XD->display, XD->cursorimg, XD->cursorimg, &black, &black, 0, 0);
	}

	XDefineCursor(XD->display, XD->win, XD->cursor);

	return CC_SUCCESS;
}

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
ccReturn ccWindowFramebufferCreate(ccFramebufferFormat *format)
{
	ccAssert(_ccWindow);

	// There already is a OpenGL context, you can't create both
	if(XD->context != NULL){
		ccErrorPush(CC_ERROR_FRAMEBUFFER_CREATE);
		return CC_FAIL;
	}

	int ignore;
	if(!XQueryExtension(XD->display, "MIT-SHM", &ignore, &ignore, &ignore)){
		ccErrorPush(CC_ERROR_FRAMEBUFFER_SHAREDMEM);
		return CC_FAIL;
	}

	XD->gc = XCreateGC(XD->display, XD->win, 0, 0);
	if(!XD->gc){
		ccErrorPush(CC_ERROR_FRAMEBUFFER_CREATE);
		return CC_FAIL;
	}

	XD->shminfo = (XShmSegmentInfo){0};
	XD->fb = XShmCreateImage(XD->display, DefaultVisual(XD->display, XD->screen), DefaultDepth(XD->display, XD->screen), ZPixmap, NULL, &XD->shminfo, _ccWindow->rect.width, _ccWindow->rect.height);
	if(XD->fb == NULL){
		XFreeGC(XD->display, XD->gc);
		XD->gc = NULL;
		ccErrorPush(CC_ERROR_FRAMEBUFFER_CREATE);
		return CC_FAIL;
	}

	switch(XD->fb->bits_per_pixel){
		case 24:
			*format = CC_FRAMEBUFFER_PIXEL_BGR24;
			break;
		case 32:
			*format = CC_FRAMEBUFFER_PIXEL_BGR32;
			break;
		default:
			XFreeGC(XD->display, XD->gc);
			XD->gc = NULL;
			XDestroyImage(XD->fb);
			XD->fb = NULL;
			ccErrorPush(CC_ERROR_FRAMEBUFFER_PIXELFORMAT);
			return CC_FAIL;
	}
	
	XD->shminfo.shmid = shmget(IPC_PRIVATE, XD->fb->bytes_per_line * XD->fb->height, IPC_CREAT | 0777);
	if(XD->shminfo.shmid == -1){
		XFreeGC(XD->display, XD->gc);
		XD->gc = NULL;
		XDestroyImage(XD->fb);
		XD->fb = NULL;
		ccErrorPush(CC_ERROR_FRAMEBUFFER_SHAREDMEM);
		return CC_FAIL;
	}

	XD->shminfo.shmaddr = (char*)shmat(XD->shminfo.shmid, 0, 0);
	if(XD->shminfo.shmaddr == (char*)-1){
		XFreeGC(XD->display, XD->gc);
		XD->gc = NULL;
		XDestroyImage(XD->fb);
		XD->fb = NULL;
		ccErrorPush(CC_ERROR_FRAMEBUFFER_SHAREDMEM);
		return CC_FAIL;
	}
	XD->fb->data = XD->shminfo.shmaddr;
	_ccWindow->pixels = XD->shminfo.shmaddr;

	XD->shminfo.readOnly = False;

	// Check if we can trigger an X error event to know if we are on a remote display
	_xerrflag = 0;
	XSetErrorHandler(handleXErrorSetFlag);
	XShmAttach(XD->display, &XD->shminfo);
	XSync(XD->display, False);

	shmctl(XD->shminfo.shmid, IPC_RMID, 0);
	if(_xerrflag){
		_xerrflag = 0;
		XFlush(XD->display);

		XFreeGC(XD->display, XD->gc);
		XD->gc = NULL;
		XDestroyImage(XD->fb);
		XD->fb = NULL;
		shmdt(XD->shminfo.shmaddr);

		ccErrorPush(CC_ERROR_FRAMEBUFFER_SHAREDMEM);
		return CC_FAIL;
	}

	XD->w = _ccWindow->rect.width;
	XD->h = _ccWindow->rect.height;

	return CC_SUCCESS;
}

ccReturn ccWindowFramebufferUpdate()
{
	if(CC_UNLIKELY(XD->fb == NULL)){
		return CC_FAIL;
	}

	XShmPutImage(XD->display, XD->win, XD->gc, XD->fb, 0, 0, 0, 0, XD->w, XD->h, False);
	XSync(XD->display, False);

	return CC_SUCCESS;
}

ccReturn ccWindowFramebufferFree()
{
	ccAssert(_ccWindow);

	if(XD->fb){
		XShmDetach(XD->display, &XD->shminfo);
		XDestroyImage(XD->fb);		
		XD->fb = NULL;
		shmdt(XD->shminfo.shmaddr);
		shmctl(XD->shminfo.shmid, IPC_RMID, 0);
	}

	if(XD->gc){
		XFreeGC(XD->display, XD->gc);
		XD->gc = NULL;
	}

	_ccWindow->pixels = NULL;

	return CC_SUCCESS;
}
#endif

ccReturn ccWindowClipboardSet(const char *text)
{
	ccAssert(_ccWindow);

	if(text == NULL) {
		return CC_FAIL;
	}

	if(XD->CLIPBOARD != None && XGetSelectionOwner(XD->display, XD->CLIPBOARD) != XD->win) {
		XSetSelectionOwner(XD->display, XD->CLIPBOARD, XD->win, CurrentTime);
	}

	XD->clipstrlen = strlen(text);
	if(!XD->clipstr) {
		ccMalloc(XD->clipstr, XD->clipstrlen);
	} else {
		ccRealloc(XD->clipstr, XD->clipstrlen);
	}
	strcpy(XD->clipstr, text);

	return CC_SUCCESS;
}

char *ccWindowClipboardGet()
{
	const Atom formats[] = { XA_STRING, XD->UTF8_STRING, XD->COMPOUND_STRING};
	const int formatCount = sizeof(formats) / sizeof(formats[0]);

	ccAssert(_ccWindow);

	Window owner = XGetSelectionOwner(XD->display, XD->CLIPBOARD);
	if(owner == XD->win) {
		return XD->clipstr;
	} else if(owner == None) {
		return NULL;
	}

	int i;
	for(i = 0; i < formatCount; i++) {
		XConvertSelection(XD->display, XD->CLIPBOARD, formats[i], XD->CCORE_SELECTION, XD->win, CurrentTime); 
		XEvent event;
		while(XCheckTypedEvent(XD->display, SelectionNotify, &event) || event.xselection.requestor != XD->win);

		if(event.xselection.property == None) {
			continue;
		}

		unsigned char *data;
		unsigned long length = getWindowProperty(event.xselection.requestor, event.xselection.property, event.xselection.target, &data);

		if(length == 0) {
			XFree(data);
			continue;
		}

		char *output = malloc(length + 1);
		if(!output) {
			XFree(data);
			ccErrorPush(CC_ERROR_MEMORY_OVERFLOW);
			return NULL;
		}
		memcpy(output, data, length);
		output[length] = '\0';

		XFree(data);
		return output;
	}

	return NULL;
}
