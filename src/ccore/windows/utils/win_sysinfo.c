#include "win_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

ccReturn ccSysinfoInitialize(void)
{
	SYSTEM_INFO sysinfo;
	MEMORYSTATUSEX memstat;
	memstat.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memstat);

	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));

	if(GetPhysicallyInstalledSystemMemory(&_ccSysinfo->ramTotal) == FALSE) goto fail;
	_ccSysinfo->ramTotal <<= 10;

	GetSystemInfo(&sysinfo);
	_ccSysinfo->processorCount = sysinfo.dwNumberOfProcessors;

	_ccSysinfo->fileMaxOpen = _getmaxstdio();

	return CC_SUCCESS;

fail:
	free(_ccSysinfo);
	return CC_FAIL;
}

void ccSysinfoFree(void)
{
	free(_ccSysinfo);
}

#endif
