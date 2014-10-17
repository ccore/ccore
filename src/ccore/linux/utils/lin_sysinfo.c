#if defined CC_USE_ALL || defined CC_USE_SYSINFO

#include "lin_sysinfo.h"

ccReturn ccSysinfoInitialize(void)
{

}

void ccSysinfoFree(void)
{
	ccAssert(_ccSysinfo != NULL);

	free(_ccSysinfo);
}

#endif