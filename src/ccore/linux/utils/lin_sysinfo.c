#include "lin_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

ccReturn ccSysinfoInitialize(void)
{
	ccAssert(_ccSysinfo == NULL);

	ccMalloc(_ccSysinfo, sizeof(ccSysinfo));

	_ccSysinfo->pageCount = sysconf(_SC_PHYS_PAGES);
	_ccSysinfo->pageSize = sysconf(_SC_PAGESIZE);
	_ccSysinfo->ram = _ccSysinfo->pageCount * _ccSysinfo->pageSize;

	return CC_SUCCESS;
}

#endif
