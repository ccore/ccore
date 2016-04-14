#include "win_time.h"

#if defined CC_USE_ALL || defined CC_USE_TIME

static void calculateConversionFactor(void)
{
	LARGE_INTEGER ticksPerSecond;

	timeBeginPeriod(1);

	QueryPerformanceFrequency(&ticksPerSecond);
	_ticksToNanoSeconds = (double)(_CC_TO_SECONDS / ticksPerSecond.QuadPart);
}

ccError ccTimeDelay(int ms)
{
	Sleep(ms);

	return CC_E_NONE;
}

uint64_t ccTimeNanoseconds(void)
{
	LARGE_INTEGER time;

	if(_ticksToNanoSeconds == -1.0) {
		calculateConversionFactor();
	}

	QueryPerformanceCounter(&time);
	return (uint64_t)(time.QuadPart * _ticksToNanoSeconds);
}

#endif