#include "../../core/interface/window.h"

static ccDisplays _displays;

ccKeyCode translateKey(WPARAM wParam)
{
	if((wParam >= 'A' && wParam <= 'Z') || (wParam >= '0' && wParam <= '9')) {
		return wParam;
	}

	switch(wParam)
	{
	case VK_BACK:
		return CC_KEY_BACKSPACE;
		break;
	case VK_TAB:
		return CC_KEY_TAB;
		break;
	case VK_RETURN:
		return CC_KEY_RETURN;
		break;
	case VK_ESCAPE:
		return CC_KEY_ESCAPE;
		break;
	case VK_SPACE:
		return CC_KEY_SPACE;
		break;
	case VK_LSHIFT:
		return CC_KEY_LSHIFT;
		break;
	case VK_RSHIFT:
		return CC_KEY_RSHIFT;
		break;
	case VK_LCONTROL:
		return CC_KEY_LCONTROL;
		break;
	case VK_RCONTROL:
		return CC_KEY_RCONTROL;
		break;
	case VK_LEFT:
		return CC_KEY_LEFT;
		break;
	case VK_RIGHT:
		return CC_KEY_RIGHT;
		break;
	case VK_UP:
		return CC_KEY_UP;
		break;
	case VK_DOWN:
		return CC_KEY_DOWN;
		break;
	}

	return CC_KEY_UNDEFINED;
}

void updateWindowDisplay(ccWindow *window)
{
	int i;
	int area, largestArea;
	ccRect displayRect;
	RECT winRect;
	
	GetWindowRect(window->winHandle, &winRect);

	window->rect.x = winRect.left;
	window->rect.y = winRect.top;

	largestArea = 0;

	for(i = 0; i < _displays.amount; i++)
	{
		ccGetDisplayRect(&_displays.display[i], &displayRect);
		area = ccRectIntersectionArea(&displayRect, &window->rect);
		if(area > largestArea) {
			largestArea = area;
			window->display = &_displays.display[i];
		}
	}
}

LRESULT CALLBACK wndProc(HWND winHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	ccWindow *activeWindow = (ccWindow*)GetWindowLong(winHandle, GWL_USERDATA);
	if(activeWindow == NULL) goto skipevent;

	activeWindow->event.type = CC_EVENT_SKIP;

	switch(message) {
	case WM_CLOSE:
		activeWindow->event.type = CC_EVENT_WINDOW_QUIT;
		break;
	case WM_SIZE:
		activeWindow->rect.width = lParam & 0x0000FFFF;
		activeWindow->rect.height = (lParam & 0xFFFF0000) >> 16;
		activeWindow->aspect = (float) activeWindow->rect.width / activeWindow->rect.height;
		activeWindow->sizeChanged = true;
	case WM_MOVE:
		updateWindowDisplay(activeWindow);
		break;
	case WM_KEYDOWN:
		activeWindow->event.type = CC_EVENT_KEY_DOWN;
		activeWindow->event.key = translateKey(wParam);
		break;
	case WM_KEYUP:
		activeWindow->event.type = CC_EVENT_KEY_UP;
		activeWindow->event.key = translateKey(wParam);
		break;
	case WM_MOUSEMOVE:
		activeWindow->mouse.x = (unsigned short)lParam & 0x0000FFFF;
		activeWindow->mouse.y = (unsigned short)((lParam & 0xFFFF0000) >> 16);
		activeWindow->event.type = CC_EVENT_MOUSE_MOVE;
		break;
	case WM_LBUTTONDOWN:
		activeWindow->event.type = CC_EVENT_MOUSE_DOWN;
		activeWindow->event.mouseButton = CC_MOUSE_BUTTON_LEFT;
		break;
	case WM_MBUTTONDOWN:
		activeWindow->event.type = CC_EVENT_MOUSE_DOWN;
		activeWindow->event.mouseButton = CC_MOUSE_BUTTON_MIDDLE;
		break;
	case WM_RBUTTONDOWN:
		activeWindow->event.type = CC_EVENT_MOUSE_DOWN;
		activeWindow->event.mouseButton = CC_MOUSE_BUTTON_RIGHT;
		break;
	case WM_LBUTTONUP:
		activeWindow->event.type = CC_EVENT_MOUSE_UP;
		activeWindow->event.mouseButton = CC_MOUSE_BUTTON_LEFT;
		break;
	case WM_MBUTTONUP:
		activeWindow->event.type = CC_EVENT_MOUSE_UP;
		activeWindow->event.mouseButton = CC_MOUSE_BUTTON_MIDDLE;
		break;
	case WM_RBUTTONUP:
		activeWindow->event.type = CC_EVENT_MOUSE_UP;
		activeWindow->event.mouseButton = CC_MOUSE_BUTTON_RIGHT;
		break;
	case WM_MOUSEWHEEL:
		activeWindow->event.type = CC_EVENT_MOUSE_SCROLL;
		activeWindow->event.scrollDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
		break;
	case WM_SETFOCUS:
		activeWindow->event.type = CC_EVENT_FOCUS_GAINED;
		break;
	case WM_KILLFOCUS:
		activeWindow->event.type = CC_EVENT_FOCUS_LOST;
		break;
	default:
	skipevent:
		return DefWindowProc(winHandle, message, wParam, lParam);
		break;
	}
	return 0;
}

void regHinstance(HINSTANCE instanceHandle)
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

bool ccPollEvent(ccWindow *window)
{
	if(window->sizeChanged) {
		window->sizeChanged = false;
		window->event.type = CC_EVENT_WINDOW_RESIZE;
		return true;
	}

	if(PeekMessage(&window->msg, window->winHandle, 0, 0, PM_REMOVE)){
		TranslateMessage(&window->msg);
		DispatchMessage(&window->msg);
		return window->event.type==CC_EVENT_SKIP?false:true;
	}
	else{
		return false;
	}
}

ccWindow *ccNewWindow(ccRect rect, const char* title, int flags)
{
	ccWindow *newWindow = malloc(sizeof(ccWindow));

	//Struct initialisation
	memcpy(&newWindow->rect, &rect, sizeof(ccRect));
	newWindow->sizeChanged = true;

	//Window creation
	HMODULE moduleHandle = GetModuleHandle(NULL);
	
	regHinstance(moduleHandle);
	newWindow->winHandle = CreateWindowEx(
		WS_EX_APPWINDOW,
		"ccWindow",
		title,
		WS_OVERLAPPEDWINDOW,
		rect.x, rect.y,
		rect.width, rect.height,
		NULL,
		NULL,
		moduleHandle,
		NULL);

	SetWindowLong(newWindow->winHandle, GWL_USERDATA, (LONG)newWindow);
	
	//apply flags
	if((flags & CC_WINDOW_FLAG_NORESIZE) == CC_WINDOW_FLAG_NORESIZE) {
		LONG lStyle = GetWindowLong(newWindow->winHandle, GWL_STYLE);
		lStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
		SetWindowLong(newWindow->winHandle, GWL_STYLE, lStyle);
	}
	if((flags & CC_WINDOW_FLAG_NOBUTTONS) == CC_WINDOW_FLAG_NOBUTTONS) {
		LONG lStyle = GetWindowLong(newWindow->winHandle, GWL_STYLE);
		lStyle &= ~WS_SYSMENU;
		SetWindowLong(newWindow->winHandle, GWL_STYLE, lStyle);
	}
	if((flags & CC_WINDOW_FLAG_ALWAYSONTOP) == CC_WINDOW_FLAG_ALWAYSONTOP) {
		RECT rect;
		GetWindowRect(newWindow->winHandle, &rect);
		SetWindowPos(newWindow->winHandle, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
	}

	return newWindow;
}

void ccFreeWindow(ccWindow *window)
{
	wglDeleteContext(window->renderContext);
	ReleaseDC(window->winHandle, window->hdc);

	DestroyWindow(window->winHandle);
	free(window);
}

void ccGLMakeCurrent(ccWindow *window)
{
	wglMakeCurrent(window->hdc, window->renderContext);
}

void ccGLBindContext(ccWindow *window, int glVersionMajor, int glVersionMinor)
{
	int pixelFormatIndex;
	window->hdc = GetDC(window->winHandle);

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		16,
		0, 0, 0, 0, 0, 0, 0
	};

	pixelFormatIndex = ChoosePixelFormat(window->hdc, &pfd);
	SetPixelFormat(window->hdc, pixelFormatIndex, &pfd);

	window->renderContext = wglCreateContext(window->hdc);
	if(window->renderContext == NULL) ccAbort("openGL could not be initialized.\nThis could happen because your openGL version is too old.");
	
	ccGLMakeCurrent(window);

	//Fetch extentions after context creation
	if(glewInit() != GLEW_OK) ccAbort("GLEW could not be initialized.");
}

void ccGLSwapBuffers(ccWindow *window)
{
	SwapBuffers(window->hdc);
}

void ccChangeWM(ccWindow *window, ccWindowMode mode)
{
	switch(mode)
	{
	case CC_WINDOW_MODE_VISIBLE:
		ShowWindow(window->winHandle, SW_SHOW);
		break;
	case CC_WINDOW_MODE_INVISIBLE:
		ShowWindow(window->winHandle, SW_HIDE);
		break;
	case CC_WINDOW_MODE_MINIMIZED:
		ShowWindow(window->winHandle, SW_MINIMIZE);
		break;
	case CC_WINDOW_MODE_WINDOW:
		SetWindowLongPtr(window->winHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
		break;
	case CC_WINDOW_MODE_FULLSCREEN:
		window->rect.width = ccGetResolution(window->display).width;
		window->rect.height = ccGetResolution(window->display).height;

		SetWindowLongPtr(window->winHandle, GWL_STYLE, WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
		ccResizeWindow(window, (ccRect){ window->display->x, window->display->y, window->rect.width, window->rect.height });
		break;
	case CC_WINDOW_MODE_MAXIMIZED:
		ShowWindow(window->winHandle, SW_MAXIMIZE);
		break;
	}
}

void ccResizeWindow(ccWindow *window, ccRect rect)
{
	memcpy(&window->rect, &rect, sizeof(ccRect));
	MoveWindow(window->winHandle, rect.x, rect.y, rect.width, rect.height, TRUE);
}

void ccCenterWindow(ccWindow *window)
{
	ccResizeWindow(window,
		(ccRect){window->display->x + ((ccGetResolution(window->display).width - window->rect.width) >> 1),
			     window->display->y + ((ccGetResolution(window->display).height - window->rect.height) >> 1),
				 window->rect.width,
				 window->rect.height
	});
}

void ccFindDisplays()
{
	DISPLAY_DEVICE device;
	DISPLAY_DEVICE display;
	DEVMODE dm;
	ccDisplay *currentDisplay;
	ccDisplayData buffer;
	ccDisplayData current;
	int deviceCount = 0;
	int displayCount;
	int i;

	dm.dmSize = sizeof(dm);
	device.cb = sizeof(DISPLAY_DEVICE);
	display.cb = sizeof(DISPLAY_DEVICE);
	_displays.amount = 0;

	while(EnumDisplayDevices(NULL, deviceCount, &device, 0)) {
		displayCount = 0;

		while(EnumDisplayDevices(device.DeviceName, displayCount, &display, 0)) {
			_displays.amount++;
			
			if(_displays.amount == 1) {
				_displays.display = malloc(sizeof(ccDisplay));
			}
			else{
				_displays.display = realloc(_displays.display, sizeof(ccDisplay)*_displays.amount);
			}

			EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS, &dm);

			currentDisplay = &_displays.display[_displays.amount - 1];
			memcpy(currentDisplay->gpuName, device.DeviceString, 128);
			memcpy(currentDisplay->monitorName, display.DeviceString, 128);
			memcpy(currentDisplay->deviceName, display.DeviceName, 128);
			ccStrTrimToChar(currentDisplay->deviceName, '\\', false);
			
			currentDisplay->x = dm.dmPosition.x;
			currentDisplay->y = dm.dmPosition.y;

			currentDisplay->amount = 0;

			current.bitDepth = dm.dmBitsPerPel;
			current.refreshRate = dm.dmDisplayFrequency;
			current.width = dm.dmPelsWidth;
			current.height = dm.dmPelsHeight;

			printf("%dx%d\n", current.width, current.height);

			i = 0;
			while(EnumDisplaySettings(device.DeviceName, i, &dm)) {
				i++;

				buffer.bitDepth = dm.dmBitsPerPel;
				buffer.refreshRate = dm.dmDisplayFrequency;
				buffer.width = dm.dmPelsWidth;
				buffer.height = dm.dmPelsHeight;

				if(ccResolutionExists(currentDisplay, &buffer)) continue;

				if(currentDisplay->amount == 0) {
					currentDisplay->resolution = malloc(sizeof(ccDisplayData));
				}
				else{
					currentDisplay->resolution = realloc(currentDisplay->resolution, sizeof(ccDisplayData)*(currentDisplay->amount + 1));
				}

				if(memcmp(&current, &buffer, sizeof(ccDisplayData)) == 0) currentDisplay->current = currentDisplay->amount;

				memcpy(&currentDisplay->resolution[currentDisplay->amount], &buffer, sizeof(ccDisplayData));

				currentDisplay->amount++;
			}

			if(currentDisplay->x == 0 && currentDisplay->y == 0) _displays.primary = _displays.amount - 1;
			
			displayCount++;

		}
		deviceCount++;

	}
}

void ccFreeDisplays() {
	int i;
	for(i = 0; i < _displays.amount; i++) {
		free(_displays.display[i].resolution);
	}
	free(_displays.display);
}

ccDisplays *ccGetDisplays()
{
	return &_displays;
}

ccDisplay *ccGetDefaultDisplay()
{
	return &_displays.display[_displays.primary];
}

ccDisplay *ccGetDisplay(int index)
{
	return &_displays.display[index];
}

void ccGetDisplayRect(ccDisplay *display, ccRect *rect)
{
	rect->x = display->x;
	rect->y = display->y;
	rect->width = display->resolution[display->current].width;
	rect->height = display->resolution[display->current].height;
}

int ccGetDisplayAmount()
{
	return _displays.amount;
}

ccDisplay *ccGetPrimaryDisplay()
{
	return &_displays.display[_displays.primary];
}

void ccUpdateDisplays()
{
	ccFreeDisplays();
	ccFindDisplays();
}

bool ccResolutionExists(ccDisplay *display, ccDisplayData *resolution)
{
	int i;

	for(i = 0; i < display->amount; i++) {
		if(memcmp(&display->resolution[i], resolution, sizeof(ccDisplayData)) == 0) {
			return true;
		}
	}

	return false;
}

void ccSetResolution(ccDisplay *display, ccDisplayData *displayData)
{
	DEVMODE devMode;
	ZeroMemory(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(display->deviceName, ENUM_CURRENT_SETTINGS, &devMode);

	devMode.dmPelsWidth = displayData->width;
	devMode.dmPelsHeight = displayData->height;
	devMode.dmBitsPerPel = displayData->bitDepth;
	devMode.dmDisplayFrequency = displayData->refreshRate;
	devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

	if(ChangeDisplaySettingsEx(display->deviceName, &devMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL) {
		ccAbort("Error changing resolution!");
	}
}