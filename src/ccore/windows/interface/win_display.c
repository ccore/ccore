#include "win_display.h"

ccError ccDisplayInitialize(void)
{
	DISPLAY_DEVICE device;
	DISPLAY_DEVICE display;
	DEVMODE dm;
	ccDisplay *currentDisplay;
	ccDisplayData buffer;
	ccDisplayData initialBuffer;
	int deviceCount = 0;
	int displayCount;
	int i;

	ccAssert(_ccDisplays == NULL);

	ccMalloc(_ccDisplays, sizeof(ccDisplays));
	_ccDisplays->display = NULL;

	dm.dmSize = sizeof(dm);
	device.cb = sizeof(DISPLAY_DEVICE);
	display.cb = sizeof(DISPLAY_DEVICE);
	_ccDisplays->amount = 0;

	while(EnumDisplayDevices(NULL, deviceCount, &device, 0)) {
		displayCount = 0;

		while(EnumDisplayDevices(device.DeviceName, displayCount, &display, 0)) {
			_ccDisplays->amount++;

			ccRealloc(_ccDisplays->display, sizeof(ccDisplay)*_ccDisplays->amount);

			EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS, &dm);

			currentDisplay = &_ccDisplays->display[_ccDisplays->amount - 1];

			ccMalloc(currentDisplay->gpuName, CC_MAXDEVICESTRINGSIZE);
			ccMalloc(currentDisplay->monitorName, CC_MAXDEVICESTRINGSIZE);
			ccMalloc(currentDisplay->deviceName, CC_MAXDEVICENAMESIZE);

			memcpy(currentDisplay->gpuName, device.DeviceString, CC_MAXDEVICESTRINGSIZE);
			memcpy(currentDisplay->monitorName, display.DeviceString, CC_MAXDEVICESTRINGSIZE);
			memcpy(currentDisplay->deviceName, display.DeviceName, CC_MAXDEVICENAMESIZE);
			ccStrTrimToChar(currentDisplay->deviceName, '\\', false);

			currentDisplay->x = dm.dmPosition.x;
			currentDisplay->y = dm.dmPosition.y;

			currentDisplay->amount = 0;
			currentDisplay->resolution = NULL;

			initialBuffer.bitDepth = dm.dmBitsPerPel;
			initialBuffer.refreshRate = dm.dmDisplayFrequency;
			initialBuffer.width = dm.dmPelsWidth;
			initialBuffer.height = dm.dmPelsHeight;

			i = 0;
			while(EnumDisplaySettings(device.DeviceName, i, &dm)) {
				i++;

				buffer.bitDepth = dm.dmBitsPerPel;
				buffer.refreshRate = dm.dmDisplayFrequency;
				buffer.width = dm.dmPelsWidth;
				buffer.height = dm.dmPelsHeight;

				if(ccDisplayResolutionExists(currentDisplay, &buffer)) continue;

				ccRealloc(currentDisplay->resolution, sizeof(ccDisplayData)*(currentDisplay->amount + 1));
				
				if(ccDisplayResolutionEqual(&buffer, &initialBuffer)) {
					currentDisplay->current = currentDisplay->amount;
					currentDisplay->initial = currentDisplay->current;
				}
				memcpy(&currentDisplay->resolution[currentDisplay->amount], &buffer, sizeof(ccDisplayData));

				currentDisplay->amount++;
			}

			if(currentDisplay->x == 0 && currentDisplay->y == 0) _ccDisplays->primary = _ccDisplays->amount - 1;

			displayCount++;

		}
		deviceCount++;
	}

	return CC_ERROR_NONE;
}

ccError ccDisplayFree(void) {
	int i;

	ccAssert(_ccDisplays != NULL);

	for(i = 0; i < _ccDisplays->amount; i++) {
		free(_ccDisplays->display[i].gpuName);
		free(_ccDisplays->display[i].monitorName);
		free(_ccDisplays->display[i].deviceName);
		free(_ccDisplays->display[i].resolution);
	}
	free(_ccDisplays->display);
	free(_ccDisplays);
	
	_ccDisplays = NULL;

	return CC_ERROR_NONE;
}

ccError ccDisplaySetResolution(ccDisplay *display, int resolutionIndex)
{
	DEVMODE devMode;
	ccDisplayData displayData;

	ccAssert(display != NULL);
	ccAssert(resolutionIndex < display->amount);

	if(resolutionIndex == CC_DEFAULT_RESOLUTION) resolutionIndex = display->initial;
	if(resolutionIndex == display->current) return CC_ERROR_NONE;

	ZeroMemory(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(display->deviceName, ENUM_CURRENT_SETTINGS, &devMode);

	displayData = display->resolution[resolutionIndex];

	devMode.dmPelsWidth = displayData.width;
	devMode.dmPelsHeight = displayData.height;
	devMode.dmBitsPerPel = displayData.bitDepth;
	devMode.dmDisplayFrequency = displayData.refreshRate;
	devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

	if(ChangeDisplaySettingsEx(display->deviceName, &devMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL) {
		return CC_ERROR_RESOLUTION_CHANGE;
	}

	display->current = resolutionIndex;

	return CC_ERROR_NONE;
}