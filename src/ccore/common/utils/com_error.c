#include <ccore/error.h>

static ccError *ccErrorStack;
static int ccErrorStackIndex;
static int ccErrorStackSize;

void ccErrorPush(ccError error)
{
	if(ccErrorStackSize <= ccErrorStackIndex) {
		ccErrorStackSize++;
		ccErrorStack = realloc(ccErrorStack, sizeof(ccError)* ccErrorStackSize);
	}
	ccErrorStack[ccErrorStackIndex] = error;
	ccErrorStackIndex++;
}

ccError ccErrorPop(void)
{
	if(ccErrorStackIndex == 0) {
		return CC_E_NONE;
	}
	else {
		ccErrorStackIndex--;
		return ccErrorStack[ccErrorStackIndex];
	}
}

void _ccErrorFree(void)
{
	if(ccErrorStackSize != 0)
	{
		free(ccErrorStack);
		ccErrorStackSize = 0;
	}
}

