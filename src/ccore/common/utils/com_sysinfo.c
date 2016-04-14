#include <ccore/sysinfo.h>

#ifdef CC_USE_SYSINFO

ccError ccSysinfoFree(void)
{
	ccAssert(_ccSysinfo != NULL);

	free(_ccSysinfo);

	_ccSysinfo = NULL;

	return CC_E_NONE;
}

#endif
