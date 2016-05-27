#include <ccore/core.h>
#include <ccore/display.h>

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
	return CC_E_NONE;
}

ccError ccDisplayFree(void)
{
	return CC_E_NONE;
}

ccError ccDisplayResolutionSet(ccDisplay *display, int resolutionIndex)
{
	return CC_E_NONE;
}
