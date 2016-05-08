#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XInput2.h>
#include <GL/glx.h>

#include <ccore/core.h>
#include <ccore/window.h>
#include <ccore/gamepad.h>
#include <ccore/opengl.h>
#include <ccore/types.h>
#include <ccore/event.h>

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
#ifndef LINUX
#error "Shared memory libraries are needed to create a framebuffer object"
#endif
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#include "x11_display.h"

static ccRect _rect;
static ccPoint _mouse;
static ccEvent _event;
static ccDisplay *_display;
static bool _supportsRawInput;
static bool _hasWindow = false;

static Window _xWin;
static Display *_xDisplay;
static GLXContext _xContext;
static XID _xCursor;
static Pixmap _xCursorimg;
static char *_xClipstr;
static size_t _xClipstrlen;
static Atom _CCORE_SELECTION, _WM_ICON, _WM_ICON_NAME, _WM_NAME, _CLIPBOARD, _INCR, _TARGETS, _MULTIPLE, _UTF8_STRING, _COMPOUND_STRING;
static int _xScreen, _xWinFlags, _xInpOpCode;
static bool _xResizable;
#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
static void *_xPixels;
static XImage *_xFramebuffer;
static XShmSegmentInfo _xShminfo;
static GC _xGc;
static int _xFramebufferWidth, _xFramebufferHeight;
#endif

/* Attribute list for a double buffered OpenGL context, with at least 4 bits per
 * color and a 16 bit depth buffer */
static int _glAttrList[] = {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16, None};

static int _cursorList[] = {XC_arrow, XC_crosshair, XC_xterm, XC_fleur, XC_hand1, XC_sb_h_double_arrow, XC_sb_v_double_arrow, XC_X_cursor, XC_question_arrow};
static char _emptyCursorData[] = {0, 0, 0, 0, 0, 0, 0, 0};

static ccError setWindowState(const char *type, bool value)
{
	Atom wmState = XInternAtom(_xDisplay, "_NET_WM_STATE", 1);
	Atom newWmState = XInternAtom(_xDisplay, type, 1);

	XEvent event = {0};
	event.type = ClientMessage;
	event.xclient.window = _xWin;
	event.xclient.message_type = wmState;
	event.xclient.format = 32;
	event.xclient.data.l[0] = value;
	event.xclient.data.l[1] = newWmState;

	XSendEvent(_xDisplay, DefaultRootWindow(_xDisplay), false, SubstructureNotifyMask, &event);

	return CC_E_NONE;
}

static ccError setResizable(bool resizable)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	XSizeHints *sizeHints = XAllocSizeHints();
	long flags = 0;

	if(_xResizable == resizable) {
		XFree(sizeHints);
		return CC_E_NONE;
	}

	_xResizable = resizable;

	XGetWMNormalHints(_xDisplay, _xWin, sizeHints, &flags);

	if(resizable) {
		sizeHints->flags &= ~(PMinSize | PMaxSize);
	} else {
		sizeHints->flags |= PMinSize | PMaxSize;
		sizeHints->min_width = sizeHints->max_width = _rect.width;
		sizeHints->min_height = sizeHints->max_height = _rect.height;
	}

	XSetWMNormalHints(_xDisplay, _xWin, sizeHints);

	XFree(sizeHints);

	return CC_E_NONE;
}

static ccError checkRawSupport()
{
	int event, error;
	if(CC_UNLIKELY(!XQueryExtension(_xDisplay, "XInputExtension", &_xInpOpCode, &event, &error))) {
		return CC_E_WM;
	}

	int mayor = 2;
	int minor = 0;
	if(CC_UNLIKELY(XIQueryVersion(_xDisplay, &mayor, &minor) == BadRequest)) {
		return CC_E_WM;
	}

	return CC_E_NONE;
}

static ccError initRawSupport()
{
	XIEventMask mask = {.deviceid = XIAllMasterDevices, .mask_len = XIMaskLen(XI_RawMotion)};
	mask.mask = calloc(mask.mask_len, sizeof(char));
	if(mask.mask == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}

	XISetMask(mask.mask, XI_Enter);
	XISetMask(mask.mask, XI_Leave);
	XISetMask(mask.mask, XI_ButtonPress);
	XISetMask(mask.mask, XI_ButtonRelease);
	XISetMask(mask.mask, XI_KeyPress);
	XISetMask(mask.mask, XI_KeyRelease);

	XISelectEvents(_xDisplay, _xWin, &mask, 1);

	mask.deviceid = XIAllDevices;
	memset(mask.mask, 0, mask.mask_len);
	XISetMask(mask.mask, XI_RawMotion);
	XISetMask(mask.mask, XI_RawButtonPress);
	XISetMask(mask.mask, XI_RawButtonRelease);
	XISetMask(mask.mask, XI_RawKeyPress);
	XISetMask(mask.mask, XI_RawKeyRelease);

	XISelectEvents( _xDisplay, DefaultRootWindow(_xDisplay), &mask, 1);

	free(mask.mask);

	return CC_E_NONE;
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
	return XGetKeyboardMapping( _xDisplay, event->detail, 1, (int[]){1})[0];
}

static int (*origXError)(Display*, XErrorEvent*);
static int handleXError(Display *display, XErrorEvent *event)
{
	char error[256];
	XGetErrorText(_xDisplay, event->error_code, error, sizeof(error));
	fprintf(stderr, "X message: %s\n", error);

	return 0;
}

static int _xErrflag = 0;
static int handleXErrorSetFlag(Display *display, XErrorEvent *event)
{
	_xErrflag = 1;
	return 0;
}

static unsigned long getWindowProperty(Window window, Atom property, Atom type, unsigned char **value)
{
	Atom actualType;
	int actualFormat;
	unsigned long length, overflow;
	XGetWindowProperty(_xDisplay, window, property, 0, LONG_MAX, False, type, &actualType, &actualFormat, &length, &overflow, value);

	if(type != AnyPropertyType && type != actualType) {
		return 0;
	}

	return length;
}

static bool handleSelectionRequest(XSelectionRequestEvent *request)
{
	const Atom formats[] = {_UTF8_STRING, _COMPOUND_STRING, XA_STRING};
	const Atom targets[] = {_TARGETS, _MULTIPLE, _UTF8_STRING, _COMPOUND_STRING, XA_STRING};
	const int formatCount = sizeof(formats) / sizeof(formats[0]);

	if(request->property == None) {
		// The requestor is a legacy client, we don't handle it
		return false;
	}

	XSelectionEvent event = {.type = SelectionNotify, .selection = request->selection, .target = request->target, .display = request->display, .requestor = request->requestor, .time = request->time};
	if(request->target == _TARGETS) {
		XChangeProperty(_xDisplay, request->requestor, request->property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, sizeof(targets) / sizeof(targets[0]));

		event.property = request->property;
	} else if(request->target == _MULTIPLE) {
		// TODO implement this and save target
		fprintf(stderr, "X handleSelectionRequest: multiple targets not implemented yet!\n");

		event.property = None;
	} else {
		event.property = None;
		int i;
		for(i = 0; i < formatCount; i++) {
			if(request->target == formats[i]) {
				XChangeProperty(_xDisplay, request->requestor, request->property, request->target, 8, PropModeReplace, (unsigned char*)_xClipstr, _xClipstrlen);

				event.property = request->property;
				break;
			}
		}
	}

	XSendEvent(_xDisplay, request->requestor, False, 0, (XEvent*)&event);

	return true;
}

ccError ccWindowCreate(ccRect rect, const char *title, int flags)
{
#ifdef _DEBUG
	assert(rect.width > 0 && rect.height > 0);
#endif

	if(CC_UNLIKELY(_hasWindow)) {
		return CC_E_WINDOW_CREATE;
	}

	_rect = rect;
	_xWinFlags = flags;

	origXError = XSetErrorHandler(handleXError);

	_xDisplay = XOpenDisplay(NULL);
	if(CC_UNLIKELY(_xDisplay == NULL)) {
		return CC_E_WM;
	}

	_xScreen = DefaultScreen(_xDisplay);

	Atom WM_WINDOW_TYPE = XInternAtom(_xDisplay, "_NET_WM_WINDOW_TYPE", False);
	Atom WM_WINDOW_TYPE_DIALOG = XInternAtom(_xDisplay, "_NET_WM_WINDOW_TYPE_DOCK", False);
	_WM_ICON = XInternAtom(_xDisplay, "_NET_WM_ICON", False);
	_WM_NAME = XInternAtom(_xDisplay, "_NET_WM_NAME", False);
	_WM_ICON_NAME = XInternAtom(_xDisplay, "_NET_WM_ICON_NAME", False);
	_CLIPBOARD = XInternAtom(_xDisplay, "CLIPBOARD", False);
	_INCR = XInternAtom(_xDisplay, "INCR", False);
	_TARGETS = XInternAtom(_xDisplay, "TARGETS", False);
	_MULTIPLE = XInternAtom(_xDisplay, "MULTIPLE", False);
	_UTF8_STRING = XInternAtom(_xDisplay, "UTF8_STRING", False);
	_COMPOUND_STRING = XInternAtom(_xDisplay, "COMPOUND_STRING", False);
	_CCORE_SELECTION = XInternAtom(_xDisplay, "CCORE_SELECTION", False);
	Atom DELETE = XInternAtom(_xDisplay, "WM_DELETE_WINDOW", True);

	_xWin = XCreateWindow(_xDisplay, RootWindow(_xDisplay, _xScreen), rect.x, rect.y, rect.width, rect.height, 0, CopyFromParent, InputOutput, CopyFromParent, 0, 0);

	// Choose types of events
	XSelectInput(_xDisplay, _xWin, PropertyChangeMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | FocusChangeMask);

	// Disable resizing
	_xResizable = true;
	if(flags & CC_WINDOW_FLAG_NORESIZE) {
		setResizable(false);
	}

	if(flags & CC_WINDOW_FLAG_NOBUTTONS) {
		XChangeProperty(_xDisplay, _xWin, WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*)&WM_WINDOW_TYPE_DIALOG, 1);
	}

	XMapWindow(_xDisplay, _xWin);
	XStoreName(_xDisplay, _xWin, "");
	ccWindowSetTitle(title);

	XSetWMProtocols(_xDisplay, _xWin, &DELETE, 1);

	ccWindowUpdateDisplay();

	// Activate always on top if applicable
	if(flags & CC_WINDOW_FLAG_ALWAYSONTOP) {
		setWindowState("_NET_WM_STATE_ABOVE", true);
	}

	if((flags & CC_WINDOW_FLAG_NORAWINPUT) != 0) {
		_supportsRawInput = checkRawSupport();
		if(_supportsRawInput) {
			initRawSupport();
		}
	} else {
		_supportsRawInput = false;
	}

	_xCursorimg = XCreateBitmapFromData(_xDisplay, _xWin, _emptyCursorData, 8, 8);
	_mouse.x = _mouse.y = 0;

	_hasWindow = true;

	return CC_E_NONE;
}

ccError ccWindowFree(void)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	XSetErrorHandler(origXError);

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
	ccWindowFramebufferFree();
#endif

	if(_xCursor != 0) {
		XFreeCursor(_xDisplay, _xCursor);
	}
	XFreePixmap(_xDisplay, _xCursorimg);
	XUnmapWindow(_xDisplay, _xWin);
	XCloseDisplay(_xDisplay);

	return CC_E_NONE;
}

bool ccWindowEventPoll(void)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	_event.type = CC_EVENT_SKIP;

#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	ccGamepadEvent gamepadEvent = ccGamepadEventPoll();
	if(gamepadEvent.type != CC_GAMEPAD_UNHANDLED) {
		_event.type = CC_EVENT_GAMEPAD;
		_event.gamepadEvent = gamepadEvent;
		return true;
	}
#endif

	if(XPending(_xDisplay) == 0) {
		return false;
	}

	XEvent event;
	XNextEvent(_xDisplay, &event);
	switch(event.type) {
		case GenericEvent:
			if(CC_UNLIKELY(!_supportsRawInput)) {
				return false;
			}

			XGenericEventCookie *cookie = &event.xcookie;
			if(!XGetEventData(_xDisplay, cookie) || cookie->type != GenericEvent || cookie->extension != _xInpOpCode) {
				return false;
			}

			switch(cookie->evtype) {
				case XI_RawMotion:
					_event.type = CC_EVENT_MOUSE_MOVE;
					_event.mouseDelta = getRawMouseMovement(cookie->data);
					break;
				case XI_RawButtonPress:
					_event.type = CC_EVENT_MOUSE_DOWN;
					_event.mouseButton = ((XIRawEvent*)cookie->data)->detail;
					break;
				case XI_RawButtonRelease:
					_event.type = CC_EVENT_MOUSE_UP;
					_event.mouseButton = ((XIRawEvent*)cookie->data)->detail;
					break;
				case XI_RawKeyPress:
					_event.type = CC_EVENT_KEY_DOWN;
					_event.keyCode = getRawKeyboardCode(cookie->data);
					break;
				case XI_RawKeyRelease:
					_event.type = CC_EVENT_KEY_UP;
					_event.keyCode = getRawKeyboardCode(cookie->data);
					break;
				case XI_Enter:
					_event.type = CC_EVENT_FOCUS_GAINED;
					break;
				case XI_Leave:
					_event.type = CC_EVENT_FOCUS_LOST;
					break;
			}
			break;
		case ButtonPress:
			if(event.xbutton.button <= 3) {
				_event.type = CC_EVENT_MOUSE_DOWN;
				_event.mouseButton = event.xbutton.button;
			} else if(event.xbutton.button == 4) {
				_event.type = CC_EVENT_MOUSE_SCROLL;
				_event.scrollDelta = 1;
			} else if(event.xbutton.button == 5) {
				_event.type = CC_EVENT_MOUSE_SCROLL;
				_event.scrollDelta = -1;
			}
			break;
		case ButtonRelease:
			_event.type = CC_EVENT_MOUSE_UP;
			_event.mouseButton = event.xbutton.button;
			break;
		case MotionNotify:
			if(CC_UNLIKELY(!_supportsRawInput)) {
				_event.type = CC_EVENT_MOUSE_MOVE;
				_event.mouseDelta.x = _mouse.x - event.xmotion.x;
				_event.mouseDelta.y = _mouse.y - event.xmotion.y;
			}
			if(CC_LIKELY(_mouse.x != event.xmotion.x || _mouse.y != event.xmotion.y)) {
				_event.type = CC_EVENT_MOUSE_MOVE;
				_mouse.x = event.xmotion.x;
				_mouse.y = event.xmotion.y;
			}
			break;
		case KeymapNotify:
			XRefreshKeyboardMapping(&event.xmapping);
			break;
		case KeyPress:
			_event.type = CC_EVENT_KEY_DOWN;
			_event.keyCode = XLookupKeysym(&event.xkey, 0);
			break;
		case KeyRelease:
			_event.type = CC_EVENT_KEY_UP;
			_event.keyCode = XLookupKeysym(&event.xkey, 0);
			break;
		case ConfigureNotify:
			if(_rect.width != event.xconfigure.width || _rect.height != event.xconfigure.height) {
				_event.type = CC_EVENT_WINDOW_RESIZE;
				_rect.width = event.xconfigure.width;
				_rect.height = event.xconfigure.height;

				XWindowAttributes _ccWindowAttributes;
				XGetWindowAttributes(_xDisplay, _xWin, &_ccWindowAttributes);

				_rect.x = _ccWindowAttributes.x;
				_rect.y = _ccWindowAttributes.y;

				ccWindowUpdateDisplay();

				if(_xWinFlags & CC_WINDOW_FLAG_NORESIZE) {
					setResizable(false);
				}

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
				if(_xFramebuffer != NULL){
					ccWindowFramebufferFree();

					ccFramebufferFormat format;
					ccWindowFramebufferCreate(&format);
				}
#endif
			}

			if(_rect.x != event.xconfigure.x || _rect.y != event.xconfigure.y) {
				_rect.x = event.xconfigure.x;
				_rect.y = event.xconfigure.y;

				ccWindowUpdateDisplay();
			}
			break;
		case ClientMessage:
			_event.type = CC_EVENT_WINDOW_QUIT;
			break;
		case FocusIn:
			_event.type = CC_EVENT_FOCUS_GAINED;
			break;
		case FocusOut:
			_event.type = CC_EVENT_FOCUS_LOST;
			break;
		case SelectionClear:
			if(_xClipstr) {
				free(_xClipstr);
				_xClipstr = NULL;
				_xClipstrlen = 0;
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

ccError ccWindowSetWindowed(ccRect *rect)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	setResizable(true);
	setWindowState("_NET_WM_STATE_FULLSCREEN", false);
	setWindowState("_NET_WM_STATE_MAXIMIZED_VERT", false);
	setWindowState("_NET_WM_STATE_MAXIMIZED_HORZ", false);

	if(rect == NULL) {
		return CC_E_NONE;
	} else {
		return ccWindowResizeMove(*rect);
	}
}

ccError ccWindowSetMaximized(void)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	ccWindowSetWindowed(NULL);

	setWindowState("_NET_WM_STATE_MAXIMIZED_VERT", true);
	setWindowState("_NET_WM_STATE_MAXIMIZED_HORZ", true);

	return CC_E_NONE;
}

ccError ccWindowSetFullscreen(int displayCount, ...)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	ccDisplay *current, *topDisplay, *bottomDisplay, *leftDisplay, *rightDisplay;
	if(CC_LIKELY(displayCount == CC_FULLSCREEN_CURRENT_DISPLAY)) {
		ccDisplay *topDisplay = bottomDisplay = leftDisplay = rightDisplay = _display;
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

	Atom atom = XInternAtom(_xDisplay, "_NET_WM_FULLSCREEN_MONITORS", False);

	XEvent event = {0};
	event.type = ClientMessage;
	event.xclient.window = _xWin;
	event.xclient.message_type = atom;
	event.xclient.format = 32;
	event.xclient.data.l[0] = DISPLAY_DATA(topDisplay)->XineramaScreen;
	event.xclient.data.l[1] = DISPLAY_DATA(bottomDisplay)->XineramaScreen;
	event.xclient.data.l[2] = DISPLAY_DATA(leftDisplay)->XineramaScreen;
	event.xclient.data.l[3] = DISPLAY_DATA(rightDisplay)->XineramaScreen;
	event.xclient.data.l[4] = 0;

	XSendEvent(_xDisplay, DefaultRootWindow(_xDisplay), false, SubstructureNotifyMask, &event);

	setResizable(true);
	setWindowState("_NET_WM_STATE_FULLSCREEN", true);

	return CC_E_NONE;
}

ccError ccWindowResizeMove(ccRect rect)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif
#ifdef _DEBUG
	assert(rect.width > 0 && rect.height > 0);
#endif

	setResizable(true);
	XMoveResizeWindow(_xDisplay, _xWin, rect.x, rect.y, rect.width, rect.height);

	if(_xWinFlags & CC_WINDOW_FLAG_NORESIZE) {
		setResizable(false);
	}

	ccWindowUpdateDisplay();

	return CC_E_NONE;
}

ccError ccWindowSetCentered(void)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif
	if(CC_UNLIKELY(_display == NULL)) {
		return CC_E_DISPLAY_NONE;
	}

	ccDisplayData *currentResolution = ccDisplayResolutionGetCurrent(_display);

	ccRect newRect = { .x = (currentResolution->width - _rect.width) >> 1, .y = (currentResolution->height - _rect.height) >> 1, .width = _rect.width, .height = _rect.height};

	ccWindowResizeMove(newRect);

	return CC_E_NONE;
}

ccError ccWindowSetBlink(void)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	return setWindowState("_NET_WM_STATE_DEMANDS_ATTENTION", true);
}

ccError ccWindowSetTitle(const char *title)
{
#ifdef X_HAVE_UTF8_STRING
	size_t len = strlen(title);
	XChangeProperty(_xDisplay, _xWin, _WM_NAME, _UTF8_STRING, 8, PropModeReplace, (unsigned char*)title, len);
	XChangeProperty(_xDisplay, _xWin, _WM_ICON_NAME, _UTF8_STRING, 8, PropModeReplace, (unsigned char*)title, len);
#else
	char *titleCopy = strdup(title);

	XTextProperty titleProperty;
	if(!XStringListToTextProperty(&titleCopy, 1, &titleProperty)) {
		return CC_E_WINDOW_MODE;
		return CC_FAIL;
	}
	free(titleCopy);

	XSetTextProperty(_xDisplay, _xWin, &titleProperty, _WM_NAME);
	XSetTextProperty(_xDisplay, _xWin, &titleProperty, _WM_ICON_NAME);
	XSetWMName(_xDisplay, _xWin, &titleProperty);
	XSetWMIconName(_xDisplay, _xWin, &titleProperty);
	XFree(titleProperty.value);
#endif

	return CC_E_NONE;
}

ccError ccWindowIconSet(ccPoint size, const uint32_t *icon)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif
#ifdef _DEBUG
	assert(size.x > 0 && size.y > 0);
#endif

	size_t datalen = size.x * size.y;
	size_t totallen = datalen + 2;
	uint32_t *data = malloc(totallen * sizeof(uint32_t));
	if(data == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}

	data[0] = (uint32_t)size.x;
	data[1] = (uint32_t)size.y;
	memcpy(data + 2, icon, datalen * sizeof(uint32_t));

	XChangeProperty(_xDisplay, _xWin, _WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)data, totallen);

	free(data);

	return CC_E_NONE;
}

ccError ccWindowMouseSetPosition(ccPoint target)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	XWarpPointer(_xDisplay, None, _xWin, 0, 0, 0, 0, target.x, target.y);

	return CC_E_NONE;
}

ccError ccWindowMouseSetCursor(ccCursor cursor)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	if(_xCursor != 0) {
		XFreeCursor(_xDisplay, _xCursor);
	}

	if(cursor != CC_CURSOR_NONE) {
		_xCursor = XCreateFontCursor(_xDisplay, _cursorList[cursor]);
		if(!_xCursor) {
			return CC_E_WINDOW_CURSOR;
		}
	} else {
		XColor black = {0};
		_xCursor = XCreatePixmapCursor(_xDisplay, _xCursorimg, _xCursorimg, &black, &black, 0, 0);
	}

	XDefineCursor(_xDisplay, _xWin, _xCursor);

	return CC_E_NONE;
}

ccError ccWindowClipboardSet(const char *text)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	if(text == NULL) {
		return CC_E_INVALID_ARGUMENT;
	}

	if(_CLIPBOARD != None && XGetSelectionOwner(_xDisplay, _CLIPBOARD) != _xWin) {
		XSetSelectionOwner(_xDisplay, _CLIPBOARD, _xWin, CurrentTime);
	}

	_xClipstrlen = strlen(text);
	if(!_xClipstr) {
		_xClipstr = malloc(_xClipstrlen);
		if(_xClipstr == NULL){
			return CC_E_MEMORY_OVERFLOW;
		}
	} else {
		_xClipstr = realloc(_xClipstr, _xClipstrlen);
		if(_xClipstr == NULL){
			return CC_E_MEMORY_OVERFLOW;
		}
	}
	strcpy(_xClipstr, text);

	return CC_E_NONE;
}

char *ccWindowClipboardGet()
{
	const Atom formats[] = { XA_STRING, _UTF8_STRING, _COMPOUND_STRING};
	const int formatCount = sizeof(formats) / sizeof(formats[0]);

#ifdef _DEBUG
	assert(_hasWindow);
#endif

	Window owner = XGetSelectionOwner(_xDisplay, _CLIPBOARD);
	if(owner == _xWin) {
		return _xClipstr;
	} else if(owner == None) {
		return NULL;
	}

	int i;
	for(i = 0; i < formatCount; i++) {
		XConvertSelection(_xDisplay, _CLIPBOARD, formats[i], _CCORE_SELECTION, _xWin, CurrentTime); 
		XEvent event;
		while(XCheckTypedEvent(_xDisplay, SelectionNotify, &event) || event.xselection.requestor != _xWin);

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
			return NULL;
		}
		memcpy(output, data, length);
		output[length] = '\0';

		XFree(data);
		return output;
	}

	return NULL;
}

ccEvent ccWindowEventGet(void)
{
	return _event;
}

ccRect ccWindowGetRect(void)
{	
	return _rect;
}

ccPoint ccWindowGetMouse(void)
{
	return _mouse;
}

ccDisplay *ccWindowGetDisplay(void)
{
#ifdef _DEBUG
	assert(_display != NULL);
#endif

	return _display;
}

bool ccWindowExists(void)
{
	return _hasWindow;
}

void ccWindowUpdateDisplay(void)
{
	int i;
	int area, largestArea;
	ccRect displayRect;

	largestArea = 0;
	for(i = 0; i < ccDisplayGetAmount(); i++) {
		displayRect = ccDisplayGetRect(ccDisplayGet(i));
		area = ccRectIntersectionArea(&displayRect, &_rect);
		if(area > largestArea) {
			largestArea = area;
			_display = ccDisplayGet(i);
		}
	}
}

bool ccWindowSupportsRawInput(void)
{
	return _supportsRawInput;
}

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
ccError ccWindowFramebufferCreate(ccFramebufferFormat *format)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	// There already is a OpenGL context, you can't create both
	if(_xContext != NULL){
		return CC_E_FRAMEBUFFER_CREATE;
	}

	int ignore;
	if(!XQueryExtension(_xDisplay, "MIT-SHM", &ignore, &ignore, &ignore)){
		return CC_E_FRAMEBUFFER_SHAREDMEM;
	}

	_xGc = XCreateGC(_xDisplay, _xWin, 0, 0);
	if(!_xGc){
		return CC_E_FRAMEBUFFER_CREATE;
	}

	_xShminfo = (XShmSegmentInfo){0};
	_xFramebuffer = XShmCreateImage(_xDisplay, DefaultVisual(_xDisplay, _xScreen), DefaultDepth(_xDisplay, _xScreen), ZPixmap, NULL, &_xShminfo, _rect.width, _rect.height);
	if(_xFramebuffer == NULL){
		XFreeGC(_xDisplay, _xGc);
		_xGc = NULL;
		return CC_E_FRAMEBUFFER_CREATE;
	}

	switch(_xFramebuffer->bits_per_pixel){
		case 24:
			*format = CC_FRAMEBUFFER_PIXEL_BGR24;
			break;
		case 32:
			*format = CC_FRAMEBUFFER_PIXEL_BGR32;
			break;
		default:
			XFreeGC(_xDisplay, _xGc);
			_xGc = NULL;
			XDestroyImage(_xFramebuffer);
			_xFramebuffer = NULL;
			return CC_E_FRAMEBUFFER_PIXELFORMAT;
	}

	_xShminfo.shmid = shmget(IPC_PRIVATE, _xFramebuffer->bytes_per_line * _xFramebuffer->height, IPC_CREAT | 0777);
	if(_xShminfo.shmid == -1){
		XFreeGC(_xDisplay, _xGc);
		_xGc = NULL;
		XDestroyImage(_xFramebuffer);
		_xFramebuffer = NULL;
		return CC_E_FRAMEBUFFER_SHAREDMEM;
	}

	_xShminfo.shmaddr = (char*)shmat(_xShminfo.shmid, 0, 0);
	if(_xShminfo.shmaddr == (char*)-1){
		XFreeGC(_xDisplay, _xGc);
		_xGc = NULL;
		XDestroyImage(_xFramebuffer);
		_xFramebuffer = NULL;
		return CC_E_FRAMEBUFFER_SHAREDMEM;
	}
	_xFramebuffer->data = _xShminfo.shmaddr;
	_xPixels = _xShminfo.shmaddr;

	_xShminfo.readOnly = False;

	// Check if we can trigger an X error event to know if we are on a remote display
	_xErrflag = 0;
	XSetErrorHandler(handleXErrorSetFlag);
	XShmAttach(_xDisplay, &_xShminfo);
	XSync(_xDisplay, False);

	shmctl(_xShminfo.shmid, IPC_RMID, 0);
	if(_xErrflag){
		_xErrflag = 0;
		XFlush(_xDisplay);

		XFreeGC(_xDisplay, _xGc);
		_xGc = NULL;
		XDestroyImage(_xFramebuffer);
		_xFramebuffer = NULL;
		shmdt(_xShminfo.shmaddr);

		return CC_E_FRAMEBUFFER_SHAREDMEM;
	}

	_xFramebufferWidth = _rect.width;
	_xFramebufferHeight = _rect.height;

	return CC_E_NONE;
}

ccError ccWindowFramebufferUpdate()
{
	if(CC_UNLIKELY(_xFramebuffer == NULL)){
		return CC_E_FRAMEBUFFER_CREATE;
	}

	XShmPutImage(_xDisplay, _xWin, _xGc, _xFramebuffer, 0, 0, 0, 0, _xFramebufferWidth, _xFramebufferHeight, False);
	XSync(_xDisplay, False);

	return CC_E_NONE;
}

ccError ccWindowFramebufferFree()
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif

	if(_xFramebuffer){
		XShmDetach(_xDisplay, &_xShminfo);
		XDestroyImage(_xFramebuffer);		
		_xFramebuffer = NULL;
		shmdt(_xShminfo.shmaddr);
		shmctl(_xShminfo.shmid, IPC_RMID, 0);
	}

	if(_xGc){
		XFreeGC(_xDisplay, _xGc);
		_xGc = NULL;
	}

	_xPixels = NULL;

	return CC_E_NONE;
}

void *ccWindowFramebufferGetPixels()
{	
	return _xPixels;
}
#endif

#if defined CC_USE_ALL || defined CC_USE_OPENGL
ccError ccGLContextBind(void)
{
	if(CC_UNLIKELY(!_hasWindow)) {
		return CC_E_WINDOW_NONE;
	}

	XVisualInfo *visual = glXChooseVisual(_xDisplay, _xScreen, _glAttrList);
	if(CC_UNLIKELY(!visual)) {
		return CC_E_GL_CONTEXT;
	}

	_xContext = glXCreateContext(_xDisplay, visual, NULL, GL_TRUE);
	glXMakeCurrent(_xDisplay, _xWin, _xContext);

	return CC_E_NONE;
}

ccError ccGLContextFree(void)
{
	if(CC_UNLIKELY(_xContext == NULL)) {
		return CC_E_GL_CONTEXT;
	}

	glXDestroyContext(_xDisplay, _xContext);

	return CC_E_NONE;
}

ccError ccGLBuffersSwap(void)
{
	if(CC_UNLIKELY(_xContext == NULL)) {
		return CC_E_GL_CONTEXT;
	}

	glXSwapBuffers(_xDisplay, _xWin);

	return CC_E_NONE;
}

bool ccGLContextIsActive(void)
{
	return _xContext != NULL;
}
#endif
