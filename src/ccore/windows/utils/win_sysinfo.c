#include "win_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

ccReturn ccSysinfoInitialize(void)
{
	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));

	if(GetPhysicallyInstalledSystemMemory(&_ccSysinfo->ramTotal) == FALSE) goto fail;
	_ccSysinfo->ramTotal <<= 10;



	return CC_SUCCESS;

fail:
	free(_ccSysinfo);
	return CC_FAIL;
}

#endif