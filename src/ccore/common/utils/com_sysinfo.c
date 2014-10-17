#include <ccore/sysinfo.h>

void ccSysinfoFree(void)
{
	ccAssert(_ccSysinfo != NULL);

	free(_ccSysinfo);

	_ccSysinfo = NULL;
}