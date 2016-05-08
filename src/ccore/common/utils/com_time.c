#include <ccore/time.h>

#if defined CC_USE_ALL || defined CC_USE_TIME

uint64_t ccTimeMicroseconds(void)
{
	return ccTimeNanoseconds() / _CC_TO_MICROSECONDS;
}

uint64_t ccTimeMilliseconds(void)
{
	return ccTimeNanoseconds() / _CC_TO_MILLISECONDS;
}

uint64_t ccTimeSeconds(void)
{
	return ccTimeNanoseconds() / _CC_TO_SECONDS;
}

#endif
