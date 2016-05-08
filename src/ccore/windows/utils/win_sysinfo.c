#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include <ccore/sysinfo.h>
#include <ccore/assert.h>

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

static uint_fast64_t _ramTotal = 0;
static unsigned int _processorCount = 0;
static unsigned int _fileMaxOpen = 0;

ccError ccSysinfoGetRamAvailable(uint_fast64_t *ram)
{	
	MEMORYSTATUSEX memstat;
	memstat.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memstat);

	*ram = memstat.ullAvailPhys;

	return CC_E_NONE;
}

ccError ccSysinfoGetRamTotal(uint_fast64_t *ram)
{
	if(CC_UNLIKELY(_ramTotal == 0)){
		if(GetPhysicallyInstalledSystemMemory(&_ramTotal) == FALSE){
			return CC_E_OS;
		}
		_ramTotal <<= 10;
	}

	*ram = _ramTotal;

	return CC_E_NONE;
}

ccError ccSysinfoGetProcessorCount(unsigned int *processors)
{
	if(CC_UNLIKELY(_processorCount == 0)){
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		_processorCount = sysinfo.dwNumberOfProcessors;
	}

	*processors = _processorCount;

	return CC_E_NONE;
}

ccError ccSysinfoGetFileMaxOpen(unsigned int *maxFiles)
{
	if(CC_UNLIKELY(_fileMaxOpen == 0)){
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		_fileMaxOpen = _getmaxstdio();
	}

	*maxFiles = _fileMaxOpen;

	return CC_E_NONE;
}

#endif
