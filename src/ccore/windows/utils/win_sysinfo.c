#if defined CC_USE_ALL || defined CC_USE_SYSINFO

#include "win_sysinfo.h"

ccReturn ccSysinfoInitialize(void)
{
	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));

	return CC_SUCCESS;
}

#endif