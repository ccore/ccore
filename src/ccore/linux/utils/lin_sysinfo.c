#if defined CC_USE_ALL || defined CC_USE_SYSINFO

#include "lin_sysinfo.h"

ccReturn ccSysinfoInitialize(void)
{
	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));

	_ccSysinfo->pageTotalCount = sysconf(_SC_PHYS_PAGES);
	_ccSysinfo->pageSize = sysconf(_SC_PAGESIZE);

	_ccSysinfo->ramTotal = _ccSysinfo->pageTotalCount * _ccSysinfo->pageSize;

	_ccSysinfo->fileMaxOpen = sysconf(_SC_OPEN_MAX);

	return CC_SUCCESS;
}

unsigned long ccSysInfoGetRamAvailable()
{
	return _ccSysInfo->pageSize * sysconf(_SC_AVPHYS_PAGES);
}

unsigned long ccSysInfoGetPageAvailableCount()
{
	return sysconf(_SC_AVPHYS_PAGES);
}

#endif
