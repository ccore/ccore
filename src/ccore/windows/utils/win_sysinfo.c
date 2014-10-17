#include "win_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

ccReturn ccSysinfoInitialize(void)
{
	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));

	GetPhysicallyInstalledSystemMemory(&_ccSysinfo->ramTotal);

	_ccSysinfo->ramTotal *= 1000UL;

	return CC_SUCCESS;
}

#endif