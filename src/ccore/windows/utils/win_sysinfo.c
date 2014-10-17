#include "win_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

ccReturn ccSysinfoInitialize(void)
{
	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));
	printf("-----%d\n", sizeof(unsigned long long));
	GetPhysicallyInstalledSystemMemory(&_ccSysinfo->ramTotal);
	_ccSysinfo->ramTotal *= 1000;

	return CC_SUCCESS;
}

#endif