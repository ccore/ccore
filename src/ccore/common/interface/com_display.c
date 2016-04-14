#include <ccore/display.h>

bool ccDisplayResolutionEqual(ccDisplayData *resolutionA, ccDisplayData *resolutionB)
{
	return (resolutionA->bitDepth == resolutionB->bitDepth && resolutionA->height == resolutionB->height &&
		resolutionA->refreshRate == resolutionB->refreshRate && resolutionA->width == resolutionB->width);
}

ccRect ccDisplayGetRect(ccDisplay *display)
{
	ccAssert(display != NULL);

	return (ccRect){ display->x, display->y, display->resolution[display->current].width, display->resolution[display->current].height };
}

bool ccDisplayResolutionExists(ccDisplay *display, ccDisplayData *resolution)
{
	int i;

	ccAssert(display != NULL);

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

	ccAssert(_ccDisplays != NULL);

	for(i = 0; i < _ccDisplays->amount; i++){
		output = ccDisplayResolutionSet(_ccDisplays->display + i, CC_DEFAULT_RESOLUTION);
		if(output != CC_SUCCESS){
			return output;
		}
	}

	return CC_E_NONE;
}

ccDisplay *ccDisplayGetDefault(void)
{
	ccAssert(_ccDisplays != NULL);
	ccAssert(_ccDisplays->display != NULL);

	return _ccDisplays->display + _ccDisplays->primary;
}

ccDisplay *ccDisplayGet(int index)
{
	ccAssert(_ccDisplays != NULL);
	ccAssert(index >= 0 && index < _ccDisplays->amount);

	return _ccDisplays->display + index;
}

int ccDisplayGetAmount(void)
{
	ccAssert(_ccDisplays != NULL);

	return _ccDisplays->amount;
}
