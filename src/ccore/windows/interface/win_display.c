#include "win_display.h"

static ccDisplay* _displays = 0;
static unsigned short _amount = 0;
static unsigned short _primary = 0;

ccError ccDisplayRevertModes(void)
{
	int i;
	ccError output;

#ifdef _DEBUG
	assert(_displays != NULL);
#endif

	for(i = 0; i < _amount; i++){
		output = ccDisplayResolutionSet(_displays + i, CC_DEFAULT_RESOLUTION);
		if(output != CC_E_NONE){
			return output;
		}
	}

	return CC_E_NONE;
}

ccDisplay *ccDisplayGetDefault(void)
{
#ifdef _DEBUG
	assert(_displays != NULL);
#endif

	return _displays + _primary;
}

ccDisplay *ccDisplayGet(int index)
{
#ifdef _DEBUG
	assert(_displays != NULL);
	assert(index >= 0 && index < _amount);
#endif

	return _displays + index;
}

int ccDisplayGetAmount(void)
{
#ifdef _DEBUG
	assert(_displays != NULL);
#endif

	return _amount;
}

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

#ifdef _DEBUG
	assert(_displays == NULL);
#endif

	dm.dmSize = sizeof(dm);
	device.cb = sizeof(DISPLAY_DEVICE);
	display.cb = sizeof(DISPLAY_DEVICE);
	_amount = 0;

	while(EnumDisplayDevices(NULL, deviceCount, &device, 0)) {
		displayCount = 0;

		while(EnumDisplayDevices(device.DeviceName, displayCount, &display, 0)) {
			if(EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS, &dm) == 0) {
				break;
			}

			_amount++;
			_displays = realloc(_displays, sizeof(ccDisplay)*_amount);
			if(_displays == NULL){
				return CC_E_MEMORY_OVERFLOW;
			}

			currentDisplay = &_displays[_amount - 1];

			currentDisplay->gpuName = malloc(CC_MAXDEVICESTRINGSIZE);
			if(currentDisplay->gpuName == NULL){
				return CC_E_MEMORY_OVERFLOW;
			}
			currentDisplay->monitorName = malloc(CC_MAXDEVICESTRINGSIZE);
			if(currentDisplay->monitorName == NULL){
				return CC_E_MEMORY_OVERFLOW;
			}
			currentDisplay->deviceName = malloc(CC_MAXDEVICENAMESIZE);
			if(currentDisplay->deviceName == NULL){
				return CC_E_MEMORY_OVERFLOW;
			}

			memcpy(currentDisplay->gpuName, device.DeviceString, CC_MAXDEVICESTRINGSIZE);
			memcpy(currentDisplay->monitorName, display.DeviceString, CC_MAXDEVICESTRINGSIZE);
			memcpy(currentDisplay->deviceName, display.DeviceName, CC_MAXDEVICENAMESIZE);
			ccStringTrimToChar(currentDisplay->deviceName, '\\', false);

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

				currentDisplay->resolution = realloc(currentDisplay->resolution, sizeof(ccDisplayData)*(currentDisplay->amount + 1));
				if(currentDisplay->resolution == NULL){
					return CC_E_MEMORY_OVERFLOW;
				}

				if(ccDisplayResolutionEqual(&buffer, &initialBuffer)) {
					currentDisplay->current = currentDisplay->amount;
					currentDisplay->initial = currentDisplay->current;
				}

				currentDisplay->resolution[currentDisplay->amount].width = buffer.width;
				currentDisplay->resolution[currentDisplay->amount].height = buffer.height;
				currentDisplay->resolution[currentDisplay->amount].refreshRate = buffer.refreshRate;
				currentDisplay->resolution[currentDisplay->amount].bitDepth = buffer.bitDepth;

				currentDisplay->amount++;
			}

			if(currentDisplay->x == 0 && currentDisplay->y == 0) _primary = _amount - 1;

			displayCount++;

		}

		deviceCount++;
	}

	return CC_E_NONE;
}

ccError ccDisplayFree(void)
{
	int i;

#ifdef _DEBUG
	assert(_displays != NULL);
#endif

	for(i = 0; i < _amount; i++) {
		free(_displays[i].gpuName);
		free(_displays[i].monitorName);
		free(_displays[i].deviceName);
		free(_displays[i].resolution);
	}
	free(_displays);

	_displays = NULL;

	return CC_E_NONE;
}

ccError ccDisplayResolutionSet(ccDisplay *display, int resolutionIndex)
{
	DEVMODE devMode;
	ccDisplayData displayData;

#ifdef _DEBUG
	assert(display != NULL);
	assert(resolutionIndex < display->amount);
#endif

	if(resolutionIndex == CC_DEFAULT_RESOLUTION) resolutionIndex = display->initial;
	if(resolutionIndex == display->current) return CC_E_NONE;

	ZeroMemory(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);

	if(EnumDisplaySettings(display->deviceName, ENUM_CURRENT_SETTINGS, &devMode) == 0) {
		return CC_E_DISPLAY_RESOLUTIONCHANGE;
	}

	displayData = display->resolution[resolutionIndex];

	devMode.dmPelsWidth = displayData.width;
	devMode.dmPelsHeight = displayData.height;
	devMode.dmBitsPerPel = displayData.bitDepth;
	devMode.dmDisplayFrequency = displayData.refreshRate;

	devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

	if(ChangeDisplaySettingsEx(display->deviceName, &devMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL) {
		return CC_E_DISPLAY_RESOLUTIONCHANGE;
	}

	display->current = (unsigned short)resolutionIndex;

	return CC_E_NONE;
}
