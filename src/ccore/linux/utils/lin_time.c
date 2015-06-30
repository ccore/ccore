#if defined CC_USE_ALL || defined CC_USE_TIME

#include "lin_time.h"

ccReturn ccTimeDelay(int ms)
{
	usleep(ms * _CC_TO_MICROSECONDS);

	return CC_SUCCESS;
}

uint64_t ccTimeNanoseconds(void)
{
	struct timespec time;

	clock_gettime(CLOCK_REALTIME, &time);

	return (uint64_t)(time.tv_nsec + time.tv_sec * _CC_TO_SECONDS);
}

#endif