#include "win_window.h"

static void updateWindowDisplay()
{
	RECT winRect;
	GetWindowRect(WINDOW_DATA->winHandle, &winRect);

	_window->rect.x = winRect.left;
	_window->rect.y = winRect.top;

	ccUpdateWindowDisplay();
}

static void updateWindowResolution()
{
	RECT winRect;
	
	GetClientRect(WINDOW_DATA->winHandle, &winRect);

	if(winRect.right - winRect.left == _window->rect.width && winRect.bottom - winRect.top == _window->rect.height) {
		return;
	}

	_window->rect.width = winRect.right - winRect.left;
	_window->rect.height = winRect.bottom - winRect.top;
	_window->aspect = (float)_window->rect.width / _window->rect.height;
	WINDOW_DATA->specialEvents |= CC_WIN32_EVENT_RESIZED;
}

static bool initializeRawInput()
{
	WINDOW_DATA->rid[RAWINPUT_KEYBOARD].usUsagePage = 0x01;
	WINDOW_DATA->rid[RAWINPUT_KEYBOARD].usUsage = 0x06;
	WINDOW_DATA->rid[RAWINPUT_KEYBOARD].dwFlags = RIDEV_NOLEGACY;
	WINDOW_DATA->rid[RAWINPUT_KEYBOARD].hwndTarget = WINDOW_DATA->winHandle;

	WINDOW_DATA->rid[RAWINPUT_MOUSE].usUsagePage = 0x01;
	WINDOW_DATA->rid[RAWINPUT_MOUSE].usUsage = 0x02;
	WINDOW_DATA->rid[RAWINPUT_MOUSE].dwFlags = 0;
	WINDOW_DATA->rid[RAWINPUT_MOUSE].hwndTarget = WINDOW_DATA->winHandle;

	return RegisterRawInputDevices(WINDOW_DATA->rid, NRAWINPUTDEVICES, sizeof(WINDOW_DATA->rid[0]));
}

static void freeRawInput()
{
	WINDOW_DATA->rid[RAWINPUT_KEYBOARD].dwFlags = RIDEV_REMOVE;
	WINDOW_DATA->rid[RAWINPUT_KEYBOARD].hwndTarget = NULL;

	WINDOW_DATA->rid[RAWINPUT_MOUSE].dwFlags = RIDEV_REMOVE;
	WINDOW_DATA->rid[RAWINPUT_MOUSE].hwndTarget = NULL;

	RegisterRawInputDevices(WINDOW_DATA->rid, NRAWINPUTDEVICES, sizeof(WINDOW_DATA->rid[0]));
}

static void processRid(HRAWINPUT rawInput)
{
	GetRawInputData(rawInput, RID_INPUT, NULL, &WINDOW_DATA->dwSize, sizeof(RAWINPUTHEADER));

	if(WINDOW_DATA->dwSize > WINDOW_DATA->lpbSize) {
		WINDOW_DATA->lpb = WINDOW_DATA->lpbSize == 0?malloc(WINDOW_DATA->dwSize):realloc(WINDOW_DATA->lpb, WINDOW_DATA->dwSize);
		WINDOW_DATA->lpbSize = WINDOW_DATA->dwSize;
	}

	GetRawInputData(rawInput, RID_INPUT, WINDOW_DATA->lpb, &WINDOW_DATA->dwSize, sizeof(RAWINPUTHEADER));

	RAWINPUT* raw = (RAWINPUT*)WINDOW_DATA->lpb;

	if(raw->header.dwType == RIM_TYPEMOUSE) {
		USHORT buttonFlags = raw->data.mouse.usButtonFlags;
		
		if(buttonFlags == 0) {
			_window->event.type = CC_EVENT_MOUSE_MOVE;
			_window->event.mouseVector.x = raw->data.mouse.lLastX;
			_window->event.mouseVector.y = raw->data.mouse.lLastY;
		}
		else if(buttonFlags & RI_MOUSE_WHEEL) {
			_window->event.type = CC_EVENT_MOUSE_SCROLL;
			_window->event.scrollDelta = (float)((short)raw->data.mouse.usButtonData) / WHEEL_DELTA;
		}
		else if(buttonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
			_window->event.type = CC_EVENT_MOUSE_DOWN;
			_window->event.mouseButton = CC_MOUSE_BUTTON_LEFT;
		}
		else if(buttonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
			_window->event.type = CC_EVENT_MOUSE_UP;
			_window->event.mouseButton = CC_MOUSE_BUTTON_LEFT;
		}
		else if(buttonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
			_window->event.type = CC_EVENT_MOUSE_DOWN;
			_window->event.mouseButton = CC_MOUSE_BUTTON_RIGHT;
		}
		else if(buttonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
			_window->event.type = CC_EVENT_MOUSE_UP;
			_window->event.mouseButton = CC_MOUSE_BUTTON_RIGHT;
		}
		else if(buttonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
			_window->event.type = CC_EVENT_MOUSE_DOWN;
			_window->event.mouseButton = CC_MOUSE_BUTTON_MIDDLE;
		}
		else if(buttonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
			_window->event.type = CC_EVENT_MOUSE_UP;
			_window->event.mouseButton = CC_MOUSE_BUTTON_MIDDLE;
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
		_window->event.type = raw->data.keyboard.Message == WM_KEYDOWN?CC_EVENT_KEY_DOWN:CC_EVENT_KEY_UP;
		_window->event.keyCode = vkCode;
	}
}

static LRESULT CALLBACK wndProc(HWND winHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	_window->event.type = CC_EVENT_SKIP;

	switch(message) {
	case WM_INPUT:
		processRid((HRAWINPUT)lParam);
		break;
	case WM_CLOSE:
		_window->event.type = CC_EVENT_WINDOW_QUIT;
		break;
	case WM_SIZE:
		updateWindowResolution();
		break;
	case WM_MOVE:
		updateWindowDisplay(_window);
		break;
	case WM_MOUSEMOVE:
		_window->mouse.x = (unsigned short)lParam & 0x0000FFFF;
		_window->mouse.y = (unsigned short)((lParam & 0xFFFF0000) >> 16);
		break;
	case WM_SETFOCUS:
		WINDOW_DATA->specialEvents |= CC_WIN32_EVENT_FOCUS_GAINED;
		break;
	case WM_KILLFOCUS:
		WINDOW_DATA->specialEvents |= CC_WIN32_EVENT_FOCUS_LOST;
		break;
	default:
		return DefWindowProc(winHandle, message, wParam, lParam);
		break;
	}
	return 0;
}

static void regHinstance(HINSTANCE instanceHandle)
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
	winClass.lpszClassName = "ccWindow";
	winClass.hIconSm = NULL;

	RegisterClassEx(&winClass);
}

bool ccPollEvent()
{
	ccAssert(_window != NULL);

	if(WINDOW_DATA->specialEvents) {
		if(WINDOW_DATA->specialEvents & CC_WIN32_EVENT_RESIZED) {
			WINDOW_DATA->specialEvents &= ~CC_WIN32_EVENT_RESIZED;
			_window->event.type = CC_EVENT_WINDOW_RESIZE;
			return true;
		}
		else if(WINDOW_DATA->specialEvents & CC_WIN32_EVENT_FOCUS_GAINED) {
			WINDOW_DATA->specialEvents &= ~CC_WIN32_EVENT_FOCUS_GAINED;
			_window->event.type = CC_EVENT_FOCUS_GAINED;
			return true;
		}
		else if(WINDOW_DATA->specialEvents & CC_WIN32_EVENT_FOCUS_LOST) {
			WINDOW_DATA->specialEvents &= ~CC_WIN32_EVENT_FOCUS_LOST;
			_window->event.type = CC_EVENT_FOCUS_LOST;
			return true;
		}
	}
	
	if(PeekMessage(&WINDOW_DATA->msg, NULL, 0, 0, PM_REMOVE)){
		TranslateMessage(&WINDOW_DATA->msg);
		DispatchMessage(&WINDOW_DATA->msg);
		return true;
	}

	return false;
}

ccError ccNewWindow(ccRect rect, const char* title, int flags)
{
	HMODULE moduleHandle = GetModuleHandle(NULL);
	RECT windowRect;

	ccAssert(_window == NULL);

	//initialize struct
	ccMalloc(_window, sizeof(ccWindow));

	_window->rect = rect;
	ccMalloc(_window->data, sizeof(ccWindow_win));

	WINDOW_DATA->specialEvents = 0;

	WINDOW_DATA->lpbSize = 0;

	//apply flags
	WINDOW_DATA->style = WS_OVERLAPPEDWINDOW;
	if((flags & CC_WINDOW_FLAG_NORESIZE) == CC_WINDOW_FLAG_NORESIZE) WINDOW_DATA->style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
	if((flags & CC_WINDOW_FLAG_NOBUTTONS) == CC_WINDOW_FLAG_NOBUTTONS)WINDOW_DATA->style &= ~WS_SYSMENU;

	windowRect.left = rect.x;
	windowRect.top = rect.y;
	windowRect.right = rect.x + rect.width;
	windowRect.bottom = rect.y + rect.height;
	if(AdjustWindowRectEx(&windowRect, WINDOW_DATA->style, FALSE, WS_EX_APPWINDOW) == FALSE) return CC_ERROR_WINDOWCREATION;
	
	regHinstance(moduleHandle);
	WINDOW_DATA->winHandle = CreateWindowEx(
		WS_EX_APPWINDOW,
		"ccWindow",
		title,
		WINDOW_DATA->style,
		windowRect.left, windowRect.top,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		moduleHandle,
		NULL);

	WINDOW_DATA->style |= WS_VISIBLE;

	ShowWindow(WINDOW_DATA->winHandle, SW_SHOW);
	
	initializeRawInput();

	if((flags & CC_WINDOW_FLAG_ALWAYSONTOP) == CC_WINDOW_FLAG_ALWAYSONTOP) {
		RECT rect;
		if(GetWindowRect(WINDOW_DATA->winHandle, &rect) == FALSE) return CC_ERROR_WINDOWCREATION;
		if(SetWindowPos(WINDOW_DATA->winHandle, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW) == FALSE) return CC_ERROR_WINDOWCREATION;
	}

	return CC_ERROR_NONE;
}

ccError ccFreeWindow()
{
	ccAssert(_window != NULL);

	freeRawInput();

	if(WINDOW_DATA->lpbSize != 0) free(WINDOW_DATA->lpb);

	ReleaseDC(WINDOW_DATA->winHandle, WINDOW_DATA->hdc);
	if(DestroyWindow(WINDOW_DATA->winHandle) == FALSE) return CC_ERROR_WINDOWDESTRUCTION;

	free(_window->data);
	free(_window);

	_window = NULL;

	return CC_ERROR_NONE;
}

ccError ccSetWindowed()
{
	ccAssert(_window != NULL);

	SetWindowLongPtr(WINDOW_DATA->winHandle, GWL_STYLE, WINDOW_DATA->style | WS_CAPTION);
	if(ShowWindow(WINDOW_DATA->winHandle, SW_SHOW) == FALSE) return CC_ERROR_WINDOW_MODE;
	ccResizeMoveWindow(ccDisplayGetRect(_window->display), true);

	return CC_ERROR_NONE;
}

ccError ccSetMaximized()
{
	ccAssert(_window != NULL);

	SetWindowLongPtr(WINDOW_DATA->winHandle, GWL_STYLE, WINDOW_DATA->style | WS_CAPTION);
	if(ShowWindow(WINDOW_DATA->winHandle, SW_MAXIMIZE) == FALSE) return CC_ERROR_WINDOW_MODE;

	return CC_ERROR_NONE;
}

ccError ccSetFullscreen(int displayCount, ...)
{
	ccAssert(_window != NULL);

	SetWindowLongPtr(WINDOW_DATA->winHandle, GWL_STYLE, WINDOW_DATA->style & ~(WS_CAPTION | WS_THICKFRAME));
	if(ShowWindow(WINDOW_DATA->winHandle, SW_SHOW) == FALSE) return CC_ERROR_WINDOW_MODE;

	if(displayCount == 0) {
		return ccResizeMoveWindow(ccDisplayGetRect(_window->display), false);
	}
	else{
		va_list displays;
		ccRect* rectList;
		ccRect spanRect;
		int i;

		ccMalloc(rectList, displayCount * sizeof(ccRect));

		va_start(displays, displayCount);

		for(i = 0; i < displayCount; i++) {
			rectList[i] = ccDisplayGetRect(va_arg(displays, ccDisplay*));
		}

		spanRect = ccRectConcatenate(displayCount, rectList);
		
		free(rectList);

		va_end(displays);

		return ccResizeMoveWindow(spanRect, false);
	}
}

ccError ccResizeMoveWindow(ccRect rect, bool addBorder)
{
	ccAssert(_window != NULL);

	if(addBorder) {
		RECT windowRect;
		windowRect.left = rect.x;
		windowRect.top = rect.y;
		windowRect.right = rect.x + rect.width;
		windowRect.bottom = rect.y + rect.height;
		if(AdjustWindowRectEx(&windowRect, WINDOW_DATA->style, FALSE, WS_EX_APPWINDOW) == FALSE) return CC_ERROR_WINDOW_MODE;

		if(MoveWindow(WINDOW_DATA->winHandle, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, FALSE) == FALSE) return CC_ERROR_WINDOW_MODE;
	}
	else{
		if(MoveWindow(WINDOW_DATA->winHandle, rect.x, rect.y, rect.width, rect.height, FALSE) == FALSE) return CC_ERROR_WINDOW_MODE;
	}

	return CC_ERROR_NONE;
}

ccError ccCenterWindow()
{
	RECT windowRect;

	ccAssert(_window != NULL);

	if(GetWindowRect(WINDOW_DATA->winHandle, &windowRect) == FALSE) return CC_ERROR_WINDOW_MODE;
	return ccResizeMoveWindow(
		(ccRect){_window->display->x + ((ccDisplayGetResolutionCurrent(_window->display)->width - (windowRect.right - windowRect.left)) >> 1),
				 _window->display->y + ((ccDisplayGetResolutionCurrent(_window->display)->height - (windowRect.bottom - windowRect.top)) >> 1),
				 windowRect.right - windowRect.left,
				 windowRect.bottom - windowRect.top
	}, false);
}