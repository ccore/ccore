#include "lin_time.h"

ccReturn ccTimeDelay(int ms)
{
	usleep(ms * _TO_MICROSECONDS);

	return CC_SUCCESS;
}

uint64_t ccTimeNanoseconds(void)
{
	struct timespec time;

	clock_gettime(CLOCK_REALTIME, &time);

	return (uint64_t)(time.tv_nsec + time.tv_sec * _TO_SECONDS);
}
