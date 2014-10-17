#include "win_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

#define _CC_GETMEMSTAT MEMORYSTATUSEX memstat; \
	memstat.dwLength = sizeof(MEMORYSTATUSEX); \
	GlobalMemoryStatusEx(&memstat)

ccReturn ccSysinfoInitialize(void)
{
	SYSTEM_INFO sysinfo;
	_CC_GETMEMSTAT;

	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));

	if(GetPhysicallyInstalledSystemMemory(&_ccSysinfo->ramTotal) == FALSE) goto fail;
	_ccSysinfo->ramTotal <<= 10;
	
	_ccSysinfo->ramUsable = memstat.ullTotalPhys;

	GetSystemInfo(&sysinfo);
	_ccSysinfo->processorCount = sysinfo.dwNumberOfProcessors;

	_ccSysinfo->fileMaxOpen = _getmaxstdio();

	return CC_SUCCESS;

fail:
	free(_ccSysinfo);
	return CC_FAIL;
}

uint_fast64_t ccSysinfoGetRamAvailable()
{
	_CC_GETMEMSTAT;

	return (uint_fast64_t)memstat.ullAvailPhys;
}

#undef _CC_GETMEMSTAT

#endif