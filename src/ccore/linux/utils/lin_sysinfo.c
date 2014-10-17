#if defined CC_USE_ALL || defined CC_USE_SYSINFO

#include "lin_sysinfo.h"

ccReturn ccSysinfoInitialize(void)
{
	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));

	_ccSysinfo->pageTotalCount = sysconf(_SC_PHYS_PAGES);
	_ccSysinfo->pageSize = sysconf(_SC_PAGESIZE);

	_ccSysinfo->ramTotal = _ccSysinfo->pageTotalCount * _ccSysinfo->pageSize;

	_ccSysinfo->processorTotalCount = sysconf(_SC_NPROCESSORS_CONF);

	_ccSysinfo->fileMaxOpen = sysconf(_SC_OPEN_MAX);

	_ccSysinfo->stringMaxLength = sysconf(_SC_BC_STRING_MAX);

	return CC_SUCCESS;
}

unsigned long ccSysinfoGetRamAvailable()
{
	return _ccSysinfo->pageSize * sysconf(_SC_AVPHYS_PAGES);
}

unsigned long ccSysinfoGetPageAvailableCount()
{
	return sysconf(_SC_AVPHYS_PAGES);
}

unsigned int ccSysinfoGetProcessorAvailableCount()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

#endif
