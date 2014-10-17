#include "lin_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

ccReturn ccSysinfoInitialize(void)
{
	struct sysinfo memInfo;

	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));

	_ccSysinfo->pageTotalCount = sysconf(_SC_PHYS_PAGES);
	_ccSysinfo->pageSize = sysconf(_SC_PAGESIZE);

	sysinfo(&memInfo);
	_ccSysinfo->ramTotal = memInfo.totalram;

	_ccSysinfo->processorTotalCount = sysconf(_SC_NPROCESSORS_CONF);

	return CC_SUCCESS;
}

uint_fast64_t ccSysinfoGetRamAvailable();
{
	struct sysinfo memInfo;

	sysinfo(&memInfo);

	return memInfo.freeram;
}

#endif
