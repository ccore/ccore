#if defined CC_USE_ALL || defined CC_USE_TIME

#include "lin_time.h"

ccError ccTimeDelay(int ms)
{
	usleep(ms * _CC_TO_MICROSECONDS);

	return CC_E_NONE;
}

uint64_t ccTimeNanoseconds(void)
{
	struct timespec time;

	clock_gettime(CLOCK_REALTIME, &time);

	return (uint64_t)(time.tv_nsec + time.tv_sec * _CC_TO_SECONDS);
}

#endif