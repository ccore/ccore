#include <ccore/display.h>

bool ccDisplayResolutionEqual(ccDisplayData *resolutionA, ccDisplayData *resolutionB)
{
	return (resolutionA->bitDepth == resolutionB->bitDepth && resolutionA->height == resolutionB->height &&
		resolutionA->refreshRate == resolutionB->refreshRate && resolutionA->width == resolutionB->width);
}

ccRect ccDisplayGetRect(ccDisplay *display)
{
#ifdef _DEBUG
	assert(display != NULL);
#endif

	return (ccRect){ display->x, display->y, display->resolution[display->current].width, display->resolution[display->current].height };
}

bool ccDisplayResolutionExists(ccDisplay *display, ccDisplayData *resolution)
{
	int i;

#ifdef _DEBUG
	assert(display != NULL);
#endif

	for(i = 0; i < display->amount; i++) {
		if(ccDisplayResolutionEqual(&display->resolution[i], resolution)) {
			return true;
		}
	}

	return false;
}

ccError ccDisplayRevertModes(void)
{
	int i;
	ccError output;

#ifdef _DEBUG
	assert(_ccDisplays != NULL);
#endif

	for(i = 0; i < _ccDisplays->amount; i++){
		output = ccDisplayResolutionSet(_ccDisplays->display + i, CC_DEFAULT_RESOLUTION);
		if(output != CC_E_NONE){
			return output;
		}
	}

	return CC_E_NONE;
}

ccDisplay *ccDisplayGetDefault(void)
{
#ifdef _DEBUG
	assert(_ccDisplays != NULL);
#endif
#ifdef _DEBUG
	assert(_ccDisplays->display != NULL);
#endif

	return _ccDisplays->display + _ccDisplays->primary;
}

ccDisplay *ccDisplayGet(int index)
{
#ifdef _DEBUG
	assert(_ccDisplays != NULL);
#endif
#ifdef _DEBUG
	assert(index >= 0 && index < _ccDisplays->amount);
#endif

	return _ccDisplays->display + index;
}

int ccDisplayGetAmount(void)
{
#ifdef _DEBUG
	assert(_ccDisplays != NULL);
#endif

	return _ccDisplays->amount;
}
