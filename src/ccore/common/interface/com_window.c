#include <ccore/window.h>

ccEvent ccWindowEventGet(void)
{
#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	return _ccWindow->event;
}

ccRect ccWindowGetRect(void)
{
#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	return _ccWindow->rect;
}

ccPoint ccWindowGetMouse(void)
{
#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	return _ccWindow->mouse;
}

ccDisplay *ccWindowGetDisplay(void)
{
#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif
#ifdef _DEBUG
	assert(_ccWindow->display != NULL);
#endif

	return _ccWindow->display;
}

bool ccWindowExists(void)
{
	return _ccWindow != NULL;
}

void ccWindowUpdateDisplay(void)
{
	int i;
	int area, largestArea;
	ccRect displayRect;

#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	largestArea = 0;
	for(i = 0; i < ccDisplayGetAmount(); i++) {
		displayRect = ccDisplayGetRect(ccDisplayGet(i));
		area = ccRectIntersectionArea(&displayRect, &_ccWindow->rect);
		if(area > largestArea) {
			largestArea = area;
			_ccWindow->display = ccDisplayGet(i);
		}
	}
}

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
void *ccWindowFramebufferGetPixels()
{
#ifdef _DEBUG
	assert(_ccWindow != NULL);
#endif

	return _ccWindow->pixels;
}
#endif
