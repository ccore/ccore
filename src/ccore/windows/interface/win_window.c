#include "win_window.h"

static LPSTR _cc_cursor[] =
{
	IDC_ARROW,
	IDC_CROSS,
	IDC_IBEAM,
	IDC_SIZEALL,
	IDC_HAND,
	IDC_SIZEWE,
	IDC_SIZENS,
	IDC_NO,
	IDC_HELP
};

void _ccEventStackPush(ccEvent event)
{
	_CC_WINDOW_DATA->eventStackPos++;

	if(_CC_WINDOW_DATA->eventStackPos >= _CC_WINDOW_DATA->eventStackSize) {
		_CC_WINDOW_DATA->eventStackSize++;
		_CC_WINDOW_DATA->eventStack = realloc(_CC_WINDOW_DATA->eventStack, sizeof(ccEvent)*_CC_WINDOW_DATA->eventStackSize);
	}

	_CC_WINDOW_DATA->eventStack[_CC_WINDOW_DATA->eventStackPos] = event;
}

static void updateWindowDisplay(void)
{
	RECT winRect;
	GetClientRect(_CC_WINDOW_DATA->winHandle, &winRect);
	ClientToScreen(_CC_WINDOW_DATA->winHandle, (LPPOINT)&winRect.left);
	ClientToScreen(_CC_WINDOW_DATA->winHandle, (LPPOINT)&winRect.right);

	_ccWindow->rect.x = winRect.left;
	_ccWindow->rect.y = winRect.top;

	ccWindowUpdateDisplay();
}

static void updateWindowResolution(void)
{
	RECT winRect;
	ccEvent resizeEvent;

	if(GetClientRect(_CC_WINDOW_DATA->winHandle, &winRect) == 0) {
		return CC_E_WINDOW_MODE;
		return;
	}

	if(winRect.right - winRect.left == _ccWindow->rect.width && winRect.bottom - winRect.top == _ccWindow->rect.height) {
		return;
	}

	_ccWindow->rect.width = winRect.right - winRect.left;
	_ccWindow->rect.height = winRect.bottom - winRect.top;

	resizeEvent.type = CC_EVENT_WINDOW_RESIZE;
	_ccEventStackPush(resizeEvent);
}

static bool initializeRawInput(void)
{
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_KEYBOARD].usUsagePage = 1;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_KEYBOARD].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_KEYBOARD].dwFlags = RIDEV_NOLEGACY;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_KEYBOARD].hwndTarget = _CC_WINDOW_DATA->winHandle;

	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_MOUSE].usUsagePage = 1;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_MOUSE].usUsage = HID_USAGE_GENERIC_MOUSE;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_MOUSE].dwFlags = 0;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_MOUSE].hwndTarget = _CC_WINDOW_DATA->winHandle;

	return (bool)RegisterRawInputDevices(_CC_WINDOW_DATA->rid, _CC_NRAWINPUTDEVICES - _CC_RAWINPUT_GAMEPADCOUNT, sizeof(RAWINPUTDEVICE));
}

static bool freeRawInput(void)
{
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_KEYBOARD].dwFlags = RIDEV_REMOVE;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_KEYBOARD].hwndTarget = NULL;

	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_MOUSE].dwFlags = RIDEV_REMOVE;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_MOUSE].hwndTarget = NULL;

	return (bool)RegisterRawInputDevices(_CC_WINDOW_DATA->rid, _CC_NRAWINPUTDEVICES - 1, sizeof(RAWINPUTDEVICE));
}

static void processRid(HRAWINPUT rawInput)
{
	RAWINPUT* raw;

	GetRawInputData(rawInput, RID_INPUT, NULL, &_CC_WINDOW_DATA->dwSize, sizeof(RAWINPUTHEADER));

	if(_CC_WINDOW_DATA->dwSize > _CC_WINDOW_DATA->lpbSize) {
		_CC_WINDOW_DATA->lpb = realloc(_CC_WINDOW_DATA->lpb, _CC_WINDOW_DATA->dwSize);
		_CC_WINDOW_DATA->lpbSize = _CC_WINDOW_DATA->dwSize;
	}

	GetRawInputData(rawInput, RID_INPUT, _CC_WINDOW_DATA->lpb, &_CC_WINDOW_DATA->dwSize, sizeof(RAWINPUTHEADER));

	raw = (RAWINPUT*)_CC_WINDOW_DATA->lpb;

	if(raw->header.dwType == RIM_TYPEMOUSE) {
		USHORT buttonFlags = raw->data.mouse.usButtonFlags;

		if(buttonFlags == 0) {
			_ccWindow->event.type = CC_EVENT_MOUSE_MOVE;
			_ccWindow->event.mouseDelta.x = raw->data.mouse.lLastX;
			_ccWindow->event.mouseDelta.y = raw->data.mouse.lLastY;
		}
		else if(buttonFlags & RI_MOUSE_WHEEL) {
			_ccWindow->event.type = CC_EVENT_MOUSE_SCROLL;
			_ccWindow->event.scrollDelta = (float)((short)raw->data.mouse.usButtonData) / WHEEL_DELTA;
		}
		else if(buttonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
			_ccWindow->event.type = CC_EVENT_MOUSE_DOWN;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_LEFT;
		}
		else if(buttonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
			_ccWindow->event.type = CC_EVENT_MOUSE_UP;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_LEFT;
		}
		else if(buttonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
			_ccWindow->event.type = CC_EVENT_MOUSE_DOWN;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_RIGHT;
		}
		else if(buttonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
			_ccWindow->event.type = CC_EVENT_MOUSE_UP;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_RIGHT;
		}
		else if(buttonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
			_ccWindow->event.type = CC_EVENT_MOUSE_DOWN;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_MIDDLE;
		}
		else if(buttonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
			_ccWindow->event.type = CC_EVENT_MOUSE_UP;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_MIDDLE;
		}
		else if(buttonFlags & RI_MOUSE_BUTTON_4_DOWN) {
			_ccWindow->event.type = CC_EVENT_MOUSE_DOWN;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_PREVIOUS;
		}
		else if(buttonFlags & RI_MOUSE_BUTTON_4_UP) {
			_ccWindow->event.type = CC_EVENT_MOUSE_UP;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_PREVIOUS;
		}
		else if(buttonFlags & RI_MOUSE_BUTTON_5_DOWN) {
			_ccWindow->event.type = CC_EVENT_MOUSE_DOWN;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_NEXT;
		}
		else if(buttonFlags & RI_MOUSE_BUTTON_5_UP) {
			_ccWindow->event.type = CC_EVENT_MOUSE_UP;
			_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_NEXT;
		}
	}
	else if(raw->header.dwType == RIM_TYPEKEYBOARD)
	{
		UINT vkCode = raw->data.keyboard.VKey;

		//Parse raw keycodes

		if(vkCode == 255) {
			return;
		}
		else if(vkCode == VK_CONTROL) {
			vkCode = raw->data.keyboard.Flags & RI_KEY_E0?VK_RCONTROL:VK_LCONTROL;
		}
		else if(vkCode == VK_SHIFT) {
			vkCode = MapVirtualKey(raw->data.keyboard.MakeCode, MAPVK_VSC_TO_VK_EX);
		}

		//fill event with data
		_ccWindow->event.type = raw->data.keyboard.Message == WM_KEYDOWN?CC_EVENT_KEY_DOWN:CC_EVENT_KEY_UP;
		_ccWindow->event.keyCode = vkCode;
	}
#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	else if(raw->header.dwType == RIM_TYPEHID)
	{
		_generateGamepadEvents(raw);
	}
#endif
}

static LRESULT CALLBACK wndProc(HWND winHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	_ccWindow->event.type = CC_EVENT_SKIP;
	switch(message) {
		case WM_INPUT:
			processRid((HRAWINPUT)lParam);
			break;
		case WM_CLOSE:
			_ccWindow->event.type = CC_EVENT_WINDOW_QUIT;
			break;
		case WM_SIZE:
			updateWindowResolution();
			break;
		case WM_MOVE:
			updateWindowDisplay();
			break;
		case WM_MOUSEMOVE:
			_ccWindow->mouse.x = (unsigned short)lParam & 0x0000FFFF;
			_ccWindow->mouse.y = (unsigned short)((lParam & 0xFFFF0000) >> 16);
			break;
		case WM_SETFOCUS:
			{
				ccEvent event;
				event.type = CC_EVENT_FOCUS_GAINED;
				_ccEventStackPush(event);
			}
			break;
		case WM_KILLFOCUS:
			{
				ccEvent event;
				event.type = CC_EVENT_FOCUS_LOST;
				_ccEventStackPush(event);
			}
			break;
		case WM_SYSCOMMAND:
			{
				LONG_PTR style = GetWindowLongPtr(_CC_WINDOW_DATA->winHandle, GWL_STYLE);
				if(((wParam & 0xFFF0) == SC_MOVE) && (style & WS_MAXIMIZE) && !(style & WS_MAXIMIZEBOX)) return 0;
				return DefWindowProc(winHandle, message, wParam, lParam);
			}
			break;
		case WM_SETCURSOR:
		default:
			return DefWindowProc(winHandle, message, wParam, lParam);
			break;
	}
	return FALSE;
}

static bool regHinstance(HINSTANCE instanceHandle)
{
	WNDCLASSEX winClass;
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = wndProc;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = instanceHandle;
	winClass.hIcon = NULL;
	winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = NULL;
	winClass.lpszMenuName = NULL;
	winClass.lpszClassName = _CC_WINDOW_CLASS_NAME;
	winClass.hIconSm = NULL;

	if((_CC_WINDOW_DATA->winClass = RegisterClassEx(&winClass)) == 0) {
		return CC_E_WINDOW_CREATE;
		return false;
	}
	return true;
}

bool ccWindowEventPoll(void)
{
	static bool canPollInput = true;

#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	if(canPollInput) {
		canPollInput = false;
#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
		if(_CC_WINDOW_DATA->queryXinput) _queryXinput();
#endif
	}

	if(_CC_WINDOW_DATA->eventStackPos != -1) {

		_ccWindow->event = _CC_WINDOW_DATA->eventStack[_CC_WINDOW_DATA->eventStackIndex];

		_CC_WINDOW_DATA->eventStackIndex++;
		_CC_WINDOW_DATA->eventStackPos--;

		if(_CC_WINDOW_DATA->eventStackPos == -1) _CC_WINDOW_DATA->eventStackIndex = 0;

		return true;
	}

	if(PeekMessage(&_CC_WINDOW_DATA->msg, NULL, 0, 0, PM_REMOVE)){
		TranslateMessage(&_CC_WINDOW_DATA->msg);
		DispatchMessage(&_CC_WINDOW_DATA->msg);
		return true;
	}

	canPollInput = true;
	return false;
}

ccError ccWindowCreate(ccRect rect, const char* title, int flags)
{
	HMODULE moduleHandle = GetModuleHandle(NULL);
	RECT windowRect;

#ifdef _DEBUG
	assert(_ccWindow == NULL);
#endif

	if(moduleHandle == NULL) {
		return CC_E_WINDOW_CREATE;
	}

	//initialize struct
	_ccWindow = malloc(sizeof(ccWindow));
	if(_ccWindow == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}

	_ccWindow->supportsRawInput = true; // Raw input is always supported on windows
	_ccWindow->rect = rect;
	_ccWindow->data = malloc(sizeof(ccWindow_win));
	if(_ccWindow->data == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}

	_CC_WINDOW_DATA->eventStackSize = 0;
	_CC_WINDOW_DATA->eventStackPos = -1;
	_CC_WINDOW_DATA->eventStackIndex = 0;
	_CC_WINDOW_DATA->eventStack = NULL;
#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	_CC_WINDOW_DATA->queryXinput = false;
#endif
	_CC_WINDOW_DATA->renderContext = NULL;
	_CC_WINDOW_DATA->lpbSize = 0;
	_CC_WINDOW_DATA->lpb = NULL;
	_CC_WINDOW_DATA->flags = flags;
	_CC_WINDOW_DATA->cursor = CC_CURSOR_ARROW;

	//apply flags
	_CC_WINDOW_DATA->style = WS_OVERLAPPEDWINDOW;
	if(flags & CC_WINDOW_FLAG_NORESIZE) _CC_WINDOW_DATA->style &= ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
	if(flags & CC_WINDOW_FLAG_NOBUTTONS) _CC_WINDOW_DATA->style &= ~WS_SYSMENU;

	windowRect.left = rect.x;
	windowRect.top = rect.y;
	windowRect.right = rect.x + rect.width;
	windowRect.bottom = rect.y + rect.height;
	if(AdjustWindowRectEx(&windowRect, _CC_WINDOW_DATA->style, FALSE, WS_EX_APPWINDOW) == FALSE) {
		return CC_E_WINDOW_CREATE;
	}

	if(!regHinstance(moduleHandle)) return CC_E_WINDOW_CREATE;

	_CC_WINDOW_DATA->winHandle = CreateWindowEx(
			WS_EX_APPWINDOW,
			_CC_WINDOW_CLASS_NAME,
			title,
			_CC_WINDOW_DATA->style,
			windowRect.left, windowRect.top,
			windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			moduleHandle,
			NULL);

	_CC_WINDOW_DATA->style |= WS_VISIBLE;

	if(ShowWindow(_CC_WINDOW_DATA->winHandle, SW_SHOW) != 0) {
		return CC_E_WINDOW_CREATE;
	}

	if(!initializeRawInput()) {
		return CC_E_WINDOW_CREATE;
	}

	if(flags & CC_WINDOW_FLAG_ALWAYSONTOP) {
		RECT rect;

		if(GetWindowRect(_CC_WINDOW_DATA->winHandle, &rect) == FALSE) {
			return CC_E_WINDOW_CREATE;
		}

		if(SetWindowPos(_CC_WINDOW_DATA->winHandle, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW) == FALSE) {
			return CC_E_WINDOW_CREATE;
		}
	}

	return CC_E_NONE;
}

ccError ccWindowFree(void)
{
#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	if(!freeRawInput()) {
		return CC_E_WINDOW_DESTROY;
	}

	if(_CC_WINDOW_DATA->lpbSize != 0) free(_CC_WINDOW_DATA->lpb);

	if(DestroyWindow(_CC_WINDOW_DATA->winHandle) == FALSE) {
		return CC_E_WINDOW_DESTROY;
	}

	if(UnregisterClass(_CC_WINDOW_DATA->winClass, NULL) == FALSE) {
		return CC_E_WINDOW_DESTROY;
	}

	if(_CC_WINDOW_DATA->eventStackSize != 0) free(_CC_WINDOW_DATA->eventStack);

	free(_ccWindow->data);
	free(_ccWindow);

	_ccWindow = NULL;

	return CC_E_NONE;
}

ccError ccWindowSetWindowed(ccRect *rect)
{
	SetWindowLongPtr(_CC_WINDOW_DATA->winHandle, GWL_STYLE, _CC_WINDOW_DATA->style | WS_CAPTION);
	if(ShowWindow(_CC_WINDOW_DATA->winHandle, SW_SHOW) == FALSE) {
		return CC_E_WINDOW_MODE;
	}

	if(rect == NULL){
		return CC_E_NONE;
	}else{
		return ccWindowSetRect(*rect);
	}
}

ccError ccWindowSetMaximized(void)
{
#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	if(SetWindowLongPtr(_CC_WINDOW_DATA->winHandle, GWL_STYLE, _CC_WINDOW_DATA->style | WS_CAPTION | WS_MAXIMIZEBOX) == 0) {
		return CC_E_WINDOW_MODE;
	}

	if(ShowWindow(_CC_WINDOW_DATA->winHandle, SW_MAXIMIZE) == FALSE) {
		return CC_E_WINDOW_MODE;
	}

	if(_CC_WINDOW_DATA->flags & CC_WINDOW_FLAG_NORESIZE) {
		if(SetWindowLongPtr(_CC_WINDOW_DATA->winHandle, GWL_STYLE, GetWindowLongPtr(_CC_WINDOW_DATA->winHandle, GWL_STYLE) &~WS_MAXIMIZEBOX) == 0) {
			return CC_E_WINDOW_MODE;
		}
	}

	return CC_E_NONE;
}

static ccError _ccWindowSetRect(ccRect rect, bool addBorder)
{
#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	if(addBorder) {
		RECT windowRect;

		windowRect.left = rect.x;
		windowRect.top = rect.y;
		windowRect.right = rect.x + rect.width;
		windowRect.bottom = rect.y + rect.height;

		if(AdjustWindowRectEx(&windowRect, _CC_WINDOW_DATA->style, FALSE, WS_EX_APPWINDOW) == FALSE) {
			return CC_E_WINDOW_MODE;
		}

		if(MoveWindow(_CC_WINDOW_DATA->winHandle, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, TRUE) == FALSE) {
			return CC_E_WINDOW_MODE;
		}
	}
	else{
		if(MoveWindow(_CC_WINDOW_DATA->winHandle, rect.x, rect.y, rect.width, rect.height, TRUE) == FALSE) {
			return CC_E_WINDOW_MODE;
		}
	}

	return CC_E_NONE;
}

ccError ccWindowSetFullscreen(int displayCount, ...)
{
#ifdef _DEBUG
	assert(_ccWindow);
#endif

	if(SetWindowLongPtr(_CC_WINDOW_DATA->winHandle, GWL_STYLE, _CC_WINDOW_DATA->style & ~(WS_CAPTION | WS_THICKFRAME)) == 0) {
		return CC_E_WINDOW_MODE;
	}

	if(ShowWindow(_CC_WINDOW_DATA->winHandle, SW_SHOW) == FALSE) {
		return CC_E_WINDOW_MODE;
	}

	if(displayCount == CC_FULLSCREEN_CURRENT_DISPLAY) {
		return _ccWindowSetRect(ccDisplayGetRect(_ccWindow->display), false);
	}
	else{
		va_list displays;
		ccRect* rectList;
		ccRect spanRect;
		int i;

		rectList = malloc(displayCount * sizeof(ccRect));
		if(rectList == NULL){
			return CC_E_MEMORY_OVERFLOW;
		}

		va_start(displays, displayCount);

		for(i = 0; i < displayCount; i++) {
			rectList[i] = ccDisplayGetRect(va_arg(displays, ccDisplay*));
		}

		spanRect = ccRectConcatenate(displayCount, rectList);

		free(rectList);

		va_end(displays);

		return _ccWindowSetRect(spanRect, false);
	}
}

ccError ccWindowSetTitle(const char *title)
{
	return SetWindowText(_CC_WINDOW_DATA->winHandle, title) == TRUE?CC_E_NONE:CC_E_WINDOW_MODE;
}

ccError ccWindowSetRect(ccRect rect)
{
	return _ccWindowSetRect(rect, true);
}

ccError ccWindowSetCentered(void)
{
	RECT windowRect;

#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	if(GetWindowRect(_CC_WINDOW_DATA->winHandle, &windowRect) == FALSE) {
		return CC_E_WINDOW_MODE;
	}

	return _ccWindowSetRect(
			(ccRect){_ccWindow->display->x + ((ccDisplayResolutionGetCurrent(_ccWindow->display)->width - (windowRect.right - windowRect.left)) >> 1),
			_ccWindow->display->y + ((ccDisplayResolutionGetCurrent(_ccWindow->display)->height - (windowRect.bottom - windowRect.top)) >> 1),
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top
			}, false);
}

ccError ccWindowSetBlink(void)
{
	FLASHWINFO flash;
	flash.cbSize = sizeof(FLASHWINFO);
	flash.hwnd = _CC_WINDOW_DATA->winHandle;
	flash.dwFlags = FLASHW_TIMERNOFG | FLASHW_ALL;
	flash.uCount = 0;
	flash.dwTimeout = 0;
	FlashWindowEx(&flash);

	return CC_E_NONE;
}

ccError ccWindowIconSet(ccPoint size, const uint32_t *data)
{
	HICON icon;
	BYTE *bmp;
	int y, dataLen, totalLen;
	uint32_t *row;

	dataLen = size.x * size.y * sizeof(uint32_t);
	totalLen = dataLen + 40 * sizeof(int32_t);
	bmp = malloc(totalLen);
	if(bmp == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}

	struct {
		int32_t headerSize, imageWidth, imageHeight;
		int16_t colorPlane, bitsPerPixel;
		int32_t compressionMode, imageLength, obsolete[4];
	} header = {
		40, size.x, size.y * 2, 1, 32, BI_RGB, dataLen, *(int32_t[]){ 0, 0, 0, 0 }
	};

	memcpy(bmp, &header, 40);

	// Windows flips it's icons vertically

	y = size.y;
	row = data;
	while (y--){
		memcpy(bmp + 40 + y * size.x * sizeof(uint32_t), row, size.x * sizeof(uint32_t));
		row += size.x;
	}

	icon = CreateIconFromResource(bmp, totalLen, TRUE, 0x00030000);

	SendMessage(_CC_WINDOW_DATA->winHandle, WM_SETICON, ICON_SMALL, (LPARAM)icon);
	SendMessage(_CC_WINDOW_DATA->winHandle, WM_SETICON, ICON_BIG, (LPARAM)icon);

	free(bmp);

	return CC_E_NONE;
}

ccError ccWindowMouseSetPosition(ccPoint target)
{
	POINT p;
	p.x = target.x;
	p.y = target.y;

	if(ClientToScreen(_CC_WINDOW_DATA->winHandle, &p) == 0) {
		return CC_E_WINDOW_CURSOR;
	}

	if(SetCursorPos(p.x, p.y) == 0) {
		return CC_E_WINDOW_CURSOR;
	}

	return CC_E_NONE;
}

ccError ccWindowMouseSetCursor(ccCursor cursor)
{
	if(cursor == CC_CURSOR_NONE) {
		if(_CC_WINDOW_DATA->cursor != CC_CURSOR_NONE) ShowCursor(FALSE);
	}
	else{
		HCURSOR hCursor;

		if(_CC_WINDOW_DATA->cursor == CC_CURSOR_NONE) ShowCursor(TRUE);

		hCursor = LoadCursor(NULL, _cc_cursor[cursor]);
		SetCursor(hCursor);
	}

	_CC_WINDOW_DATA->cursor = cursor;

	return CC_E_NONE;
}

ccError ccWindowClipboardSet(const char *data)
{
	HGLOBAL clipboardData;
	int dataLength;

	dataLength = (int)strlen(data) + 1;
	if(dataLength <= 0) {
		return CC_E_WINDOW_CLIPBOARD;
	}

	if(OpenClipboard(NULL) == FALSE) {
		return CC_E_WINDOW_CLIPBOARD;
	}

	if(EmptyClipboard() == FALSE) {
		return CC_E_WINDOW_CLIPBOARD;
		CloseClipboard();
	}

	clipboardData = GlobalAlloc(GMEM_MOVEABLE, dataLength);
	if(clipboardData == NULL) {
		return CC_E_WINDOW_CLIPBOARD;
		goto closeFreeAndFail;
	}

	memcpy(GlobalLock(clipboardData), data, dataLength);
	if(GlobalUnlock(clipboardData)!=0) {
		return CC_E_WINDOW_CLIPBOARD;
		goto closeFreeAndFail;
	}

	SetClipboardData(CF_TEXT, clipboardData);
	if(clipboardData == NULL) {
		return CC_E_WINDOW_CLIPBOARD;

closeFreeAndFail:
		CloseClipboard();
		GlobalFree(clipboardData);
	}

	if(CloseClipboard() == FALSE) {
		return CC_E_WINDOW_CLIPBOARD;
	}

	GlobalFree(clipboardData);

	return CC_E_NONE;
}

char *ccWindowClipboardGet(void)
{
	UINT format = 0;
	bool hasText = false;
	char *str;

	OpenClipboard(NULL);

	while((format = EnumClipboardFormats(format))) {
		if(format == CF_TEXT) {
			hasText = true;
		}
	}

	str = hasText?GetClipboardData(CF_TEXT):NULL;

	CloseClipboard();

	return str;
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
	return true;
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

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
void *ccWindowFramebufferGetPixels()
{	
	return _pixels;
}
#endif
