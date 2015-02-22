#include <ccore/sysinfo.h>

#ifdef CC_USE_SYSINFO

void ccSysinfoFree(void)
{
	ccAssert(_ccSysinfo != NULL);

	free(_ccSysinfo);

	_ccSysinfo = NULL;
}

#endif