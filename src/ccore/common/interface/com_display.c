#include <ccore/display.h>

ccRect ccDisplayGetRect(const ccDisplay *display)
{
#ifdef _DEBUG
	assert(display != NULL);
#endif

	return (ccRect){
		display->x, display->y, display->resolution[display->current].width, display->resolution[display->current].height };
}

bool ccDisplayResolutionExists(const ccDisplay *display, const ccDisplayData *resolution)
{
#ifdef _DEBUG
	assert(display != NULL);
#endif

	int i;
	for(i = 0; i < display->amount; i++) {
		if(ccDisplayResolutionEqual(&display->resolution[i], resolution)) {
			return true;
		}
	}

	return false;
}

ccDisplayData *ccDisplayResolutionGetCurrent(const ccDisplay *display)
{
#ifdef _DEBUG
	assert(display != NULL);
#endif

	return display->resolution + display->current;
}

ccDisplayData *ccDisplayResolutionGet(const ccDisplay *display, int index)
{
#ifdef _DEBUG
	assert(display != NULL);
#endif

	return display->resolution + index;
}

int ccDisplayResolutionGetAmount(const ccDisplay *display)
{
#ifdef _DEBUG
	assert(display != NULL);
#endif

	return display->amount;	
}

bool ccDisplayResolutionEqual(const ccDisplayData *resolutionA, const ccDisplayData *resolutionB)
{
#ifdef _DEBUG
	assert(resolutionA != NULL);
	assert(resolutionB != NULL);
#endif

	return (resolutionA->bitDepth == resolutionB->bitDepth && resolutionA->height == resolutionB->height &&
		resolutionA->refreshRate == resolutionB->refreshRate && resolutionA->width == resolutionB->width);
}


