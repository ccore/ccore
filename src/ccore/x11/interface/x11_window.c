#include "x11_window.h"

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
	Atom wmState = XInternAtom(XWINDATA->XDisplay, "_NET_WM_STATE", 1);
	Atom newWmState = XInternAtom(XWINDATA->XDisplay, type, 1);

	XEvent event = {0};
	event.type = ClientMessage;
	event.xclient.window = XWINDATA->XWindow;
	event.xclient.message_type = wmState;
	event.xclient.format = 32;
	event.xclient.data.l[0] = value;
	event.xclient.data.l[1] = newWmState;

	XSendEvent(XWINDATA->XDisplay, DefaultRootWindow(XWINDATA->XDisplay), false, SubstructureNotifyMask, &event);

	return CC_SUCCESS;
}

static ccReturn setResizable(bool resizable)
{
	ccAssert(_ccWindow != NULL);

	XSizeHints *sizeHints = XAllocSizeHints();
	long flags = 0;

	if(XWINDATA->resizable == resizable) {
		XFree(sizeHints);
		return CC_SUCCESS;
	}

	XWINDATA->resizable = resizable;

	XGetWMNormalHints(XWINDATA->XDisplay, XWINDATA->XWindow, sizeHints, &flags);

	if(resizable) {
		sizeHints->flags &= ~(PMinSize | PMaxSize);
	} else {
		sizeHints->flags |= PMinSize | PMaxSize;
		sizeHints->min_width = sizeHints->max_width = _ccWindow->rect.width;
		sizeHints->min_height = sizeHints->max_height = _ccWindow->rect.height;
	}

	XSetWMNormalHints(XWINDATA->XDisplay, XWINDATA->XWindow, sizeHints);

	XFree(sizeHints);

	return CC_SUCCESS;
}

static ccReturn checkRawSupport()
{
	int event, error;
	if(CC_UNLIKELY(!XQueryExtension(XWINDATA->XDisplay, "XInputExtension", &XWINDATA->XInputOpcode, &event, &error))) {
		return CC_FAIL;
	}

	int mayor = 2;
	int minor = 0;
	if(CC_UNLIKELY(XIQueryVersion(XWINDATA->XDisplay, &mayor, &minor) == BadRequest)) {
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

	XISelectEvents(XWINDATA->XDisplay, XWINDATA->XWindow, &mask, 1);

	mask.deviceid = XIAllDevices;
	memset(mask.mask, 0, mask.mask_len);
	XISetMask(mask.mask, XI_RawMotion);
	XISetMask(mask.mask, XI_RawButtonPress);
	XISetMask(mask.mask, XI_RawButtonRelease);
	XISetMask(mask.mask, XI_RawKeyPress);
	XISetMask(mask.mask, XI_RawKeyRelease);

	XISelectEvents( XWINDATA->XDisplay, DefaultRootWindow(XWINDATA->XDisplay), &mask, 1);

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
	return XGetKeyboardMapping( XWINDATA->XDisplay, event->detail, 1, (int[]){1})[0];
}

static int (*origXError)(Display*, XErrorEvent*);
static int handleXError(Display *display, XErrorEvent *event)
{
	ccErrorPush(CC_ERROR_WM);

	char error[256];
	XGetErrorText(XWINDATA->XDisplay, event->error_code, error, sizeof(error));
	ccPrintf("X message: %s\n", error);

	return 0;
}

static unsigned long getWindowProperty(Window window, Atom property, Atom type, unsigned char **value)
{
	Atom actualType;
	int actualFormat;
	unsigned long length, overflow;
	XGetWindowProperty(XWINDATA->XDisplay, window, property, 0, LONG_MAX, False, type, &actualType, &actualFormat, &length, &overflow, value);

	if(type != AnyPropertyType && type != actualType) {
		return 0;
	}

	return length;
}

static bool handleSelectionRequest(XSelectionRequestEvent *request)
{
	const Atom formats[] = {XWINDATA->UTF8_STRING, XWINDATA->COMPOUND_STRING, XA_STRING};
	const Atom targets[] = {XWINDATA->TARGETS, XWINDATA->MULTIPLE, XWINDATA->UTF8_STRING, XWINDATA->COMPOUND_STRING, XA_STRING};
	const int formatCount = sizeof(formats) / sizeof(formats[0]);

	if(request->property == None) {
		// The requestor is a legacy client, we don't handle it
		return false;
	}

	XSelectionEvent event = {.type = SelectionNotify, .selection = request->selection, .target = request->target, .display = request->display, .requestor = request->requestor, .time = request->time};
	if(request->target == XWINDATA->TARGETS) {
		XChangeProperty(XWINDATA->XDisplay, request->requestor, request->property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, sizeof(targets) / sizeof(targets[0]));

		event.property = request->property;
	} else if(request->target == XWINDATA->MULTIPLE) {
		// TODO implement this and save target
		ccPrintf( "X handleSelectionRequest: multiple targets not implemented yet!\n");

		event.property = None;
	} else {
		event.property = None;
		int i;
		for(i = 0; i < formatCount; i++) {
			if(request->target == formats[i]) {
				XChangeProperty(XWINDATA->XDisplay, request->requestor, request->property, request->target, 8, PropModeReplace, (unsigned char*)XWINDATA->XClipString, XWINDATA->XClipStringLength);

				event.property = request->property;
				break;
			}
		}
	}

	XSendEvent(XWINDATA->XDisplay, request->requestor, False, 0, (XEvent*)&event);

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
	ccMalloc(_ccWindow->data, sizeof(ccWindow_x11));
	XWINDATA->windowFlags = flags;

	origXError = XSetErrorHandler(handleXError);

	XWINDATA->XDisplay = XOpenDisplay(NULL);
	if(CC_UNLIKELY(XWINDATA->XDisplay == NULL)) {
		ccErrorPush(CC_ERROR_WM);
		return CC_FAIL;
	}

	XWINDATA->XScreen = DefaultScreen(XWINDATA->XDisplay);

	Atom WM_WINDOW_TYPE = XInternAtom(XWINDATA->XDisplay, "_NET_WM_WINDOW_TYPE", False);
	Atom WM_WINDOW_TYPE_DIALOG = XInternAtom(XWINDATA->XDisplay, "_NET_WM_WINDOW_TYPE_DOCK", False);
	XWINDATA->WM_ICON = XInternAtom(XWINDATA->XDisplay, "_NET_WM_ICON", False);
	XWINDATA->WM_NAME = XInternAtom(XWINDATA->XDisplay, "_NET_WM_NAME", False);
	XWINDATA->WM_ICON_NAME = XInternAtom(XWINDATA->XDisplay, "_NET_WM_ICON_NAME", False);
	XWINDATA->CLIPBOARD = XInternAtom(XWINDATA->XDisplay, "CLIPBOARD", False);
	XWINDATA->INCR = XInternAtom(XWINDATA->XDisplay, "INCR", False);
	XWINDATA->TARGETS = XInternAtom(XWINDATA->XDisplay, "TARGETS", False);
	XWINDATA->MULTIPLE = XInternAtom(XWINDATA->XDisplay, "MULTIPLE", False);
	XWINDATA->UTF8_STRING = XInternAtom(XWINDATA->XDisplay, "UTF8_STRING", False);
	XWINDATA->COMPOUND_STRING = XInternAtom(XWINDATA->XDisplay, "COMPOUND_STRING", False);
	XWINDATA->CCORE_SELECTION = XInternAtom(XWINDATA->XDisplay, "CCORE_SELECTION", False);
	Atom DELETE = XInternAtom(XWINDATA->XDisplay, "WM_DELETE_WINDOW", True);

	XWINDATA->XWindow = XCreateWindow(XWINDATA->XDisplay, RootWindow(XWINDATA->XDisplay, XWINDATA->XScreen), rect.x, rect.y, rect.width, rect.height, 0, CopyFromParent, InputOutput, CopyFromParent, 0, 0);

	// Choose types of events
	XSelectInput(XWINDATA->XDisplay, XWINDATA->XWindow, PropertyChangeMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | FocusChangeMask);

	// Disable resizing
	XWINDATA->resizable = true;
	if(flags & CC_WINDOW_FLAG_NORESIZE) {
		setResizable(false);
	}

	if(flags & CC_WINDOW_FLAG_NOBUTTONS) {
		XChangeProperty(XWINDATA->XDisplay, XWINDATA->XWindow, WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*)&WM_WINDOW_TYPE_DIALOG, 1);
	}

	XMapWindow(XWINDATA->XDisplay, XWINDATA->XWindow);
	XStoreName(XWINDATA->XDisplay, XWINDATA->XWindow, title);

	XSetWMProtocols(XWINDATA->XDisplay, XWINDATA->XWindow, &DELETE, 1);

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

	_ccWindow->mouse.x = _ccWindow->mouse.y = 0;
	XWINDATA->XCursor = 0;
	XWINDATA->XEmptyCursorImage = XCreateBitmapFromData(XWINDATA->XDisplay, XWINDATA->XWindow, emptyCursorData, 8, 8);
	XWINDATA->XClipString = NULL;
	XWINDATA->XContext = NULL;
	XWINDATA->XClipStringLength = 0;

	return CC_SUCCESS;
}

ccReturn ccWindowFree(void)
{
	ccAssert(_ccWindow);

	XSetErrorHandler(origXError);

	if(XWINDATA->XCursor != 0) {
		XFreeCursor(XWINDATA->XDisplay, XWINDATA->XCursor);
	}
	XFreePixmap(XWINDATA->XDisplay, XWINDATA->XEmptyCursorImage);
	XUnmapWindow(XWINDATA->XDisplay, XWINDATA->XWindow);
	XCloseDisplay(XWINDATA->XDisplay);

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

	if(XPending(XWINDATA->XDisplay) == 0) {
		return false;
	}

	XEvent event;
	XNextEvent(XWINDATA->XDisplay, &event);
#if defined CC_USE_ALL || defined CC_USE_TEXT
	ccTextEvent te = ccTextEventHandle(event);
	if(te.type != CC_TEXT_UNHANDLED){
		_ccWindow->event.type = CC_EVENT_TEXT;
		_ccWindow->event.textEvent = te;
		return true;
	}
#endif
	switch(event.type) {
		case GenericEvent:
			if(CC_UNLIKELY(!_ccWindow->supportsRawInput)) {
				return false;
			}

			XGenericEventCookie *cookie = &event.xcookie;
			if(!XGetEventData(XWINDATA->XDisplay, cookie) || cookie->type != GenericEvent || cookie->extension != XWINDATA->XInputOpcode) {
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
				XGetWindowAttributes(XWINDATA->XDisplay, XWINDATA->XWindow, &_ccWindowAttributes);

				_ccWindow->rect.x = _ccWindowAttributes.x;
				_ccWindow->rect.y = _ccWindowAttributes.y;

				ccWindowUpdateDisplay();

				if(XWINDATA->windowFlags & CC_WINDOW_FLAG_NORESIZE) {
					setResizable(false);
				}
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
			if(XWINDATA->XClipString) {
				free(XWINDATA->XClipString);
				XWINDATA->XClipString = NULL;
				XWINDATA->XClipStringLength = 0;
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

	Atom atom = XInternAtom(XWINDATA->XDisplay, "_NET_WM_FULLSCREEN_MONITORS", False);

	XEvent event = {0};
	event.type = ClientMessage;
	event.xclient.window = XWINDATA->XWindow;
	event.xclient.message_type = atom;
	event.xclient.format = 32;
	event.xclient.data.l[0] = DISPLAY_DATA(topDisplay)->XineramaScreen;
	event.xclient.data.l[1] = DISPLAY_DATA(bottomDisplay)->XineramaScreen;
	event.xclient.data.l[2] = DISPLAY_DATA(leftDisplay)->XineramaScreen;
	event.xclient.data.l[3] = DISPLAY_DATA(rightDisplay)->XineramaScreen;
	event.xclient.data.l[4] = 0;

	XSendEvent(XWINDATA->XDisplay, DefaultRootWindow(XWINDATA->XDisplay), false, SubstructureNotifyMask, &event);

	setResizable(true);
	setWindowState("_NET_WM_STATE_FULLSCREEN", true);

	return CC_SUCCESS;
}

ccReturn ccWindowResizeMove(ccRect rect)
{
	ccAssert(_ccWindow);
	ccAssert(rect.width > 0 && rect.height > 0);

	setResizable(true);
	XMoveResizeWindow(XWINDATA->XDisplay, XWINDATA->XWindow, rect.x, rect.y, rect.width, rect.height);

	_ccWindow->rect = rect;
	if(XWINDATA->windowFlags & CC_WINDOW_FLAG_NORESIZE) {
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

	XSetTextProperty( XWINDATA->XDisplay, XWINDATA->XWindow, &titleProperty, XWINDATA->WM_NAME);
	XSetTextProperty(XWINDATA->XDisplay, XWINDATA->XWindow, &titleProperty, XWINDATA->WM_ICON_NAME);
	XSetWMName(XWINDATA->XDisplay, XWINDATA->XWindow, &titleProperty);
	XSetWMIconName(XWINDATA->XDisplay, XWINDATA->XWindow, &titleProperty);
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

	XChangeProperty(XWINDATA->XDisplay, XWINDATA->XWindow, XWINDATA->WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)data, totalLen);

	free(data);

	return CC_SUCCESS;
}

ccReturn ccWindowMouseSetPosition(ccPoint target)
{
	ccAssert(_ccWindow);

	XWarpPointer(XWINDATA->XDisplay, None, XWINDATA->XWindow, 0, 0, 0, 0, target.x, target.y);

	return CC_SUCCESS;
}

ccReturn ccWindowMouseSetCursor(ccCursor cursor)
{
	ccAssert(_ccWindow);

	if(XWINDATA->XCursor != 0) {
		XFreeCursor(XWINDATA->XDisplay, XWINDATA->XCursor);
	}

	if(cursor != CC_CURSOR_NONE) {
		XWINDATA->XCursor = XCreateFontCursor(XWINDATA->XDisplay, cursorList[cursor]);
		if(!XWINDATA->XCursor) {
			ccErrorPush(CC_ERROR_WINDOW_CURSOR);
		}
	} else {
		XColor black = {0};
		XWINDATA->XCursor = XCreatePixmapCursor(XWINDATA->XDisplay, XWINDATA->XEmptyCursorImage, XWINDATA->XEmptyCursorImage, &black, &black, 0, 0);
	}

	XDefineCursor(XWINDATA->XDisplay, XWINDATA->XWindow, XWINDATA->XCursor);

	return CC_SUCCESS;
}

ccReturn ccWindowClipboardSet(const char *text)
{
	ccAssert(_ccWindow);

	if(text == NULL) {
		return CC_FAIL;
	}

	if(XWINDATA->CLIPBOARD != None && XGetSelectionOwner(XWINDATA->XDisplay, XWINDATA->CLIPBOARD) != XWINDATA->XWindow) {
		XSetSelectionOwner(XWINDATA->XDisplay, XWINDATA->CLIPBOARD, XWINDATA->XWindow, CurrentTime);
	}

	XWINDATA->XClipStringLength = strlen(text);
	if(!XWINDATA->XClipString) {
		ccMalloc(XWINDATA->XClipString, XWINDATA->XClipStringLength);
	} else {
		ccRealloc(XWINDATA->XClipString, XWINDATA->XClipStringLength);
	}
	strcpy(XWINDATA->XClipString, text);

	return CC_SUCCESS;
}

char *ccWindowClipboardGet()
{
	const Atom formats[] = { XA_STRING, XWINDATA->UTF8_STRING, XWINDATA->COMPOUND_STRING};
	const int formatCount = sizeof(formats) / sizeof(formats[0]);

	ccAssert(_ccWindow);

	Window owner = XGetSelectionOwner(XWINDATA->XDisplay, XWINDATA->CLIPBOARD);
	if(owner == XWINDATA->XWindow) {
		return XWINDATA->XClipString;
	} else if(owner == None) {
		return NULL;
	}

	int i;
	for(i = 0; i < formatCount; i++) {
		XConvertSelection(XWINDATA->XDisplay, XWINDATA->CLIPBOARD, formats[i], XWINDATA->CCORE_SELECTION, XWINDATA->XWindow, CurrentTime); 
		XEvent event;
		while(XCheckTypedEvent(XWINDATA->XDisplay, SelectionNotify, &event) || event.xselection.requestor != XWINDATA->XWindow);

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
