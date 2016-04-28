#include "lin_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

static long parseLineKB(char *line)
{
	int i = strlen(line);
	while(*line < '0' || *line > '9'){
		line++;
	}
	line[i - 3] = '\0';

	return atoi(line);
}

static long getKBValueFromProc(const char *proc, const char *field)
{
	FILE *file = fopen(proc, "r");
	if(!file){
		return -1;
	}
	int fieldlen = strlen(field) - 1;
	char line[128];
	long result = -1;
	while(fgets(line, 128, file) != NULL){
		if(strncmp(line, field, fieldlen) == 0){
			result = parseLineKB(line);
			break;
		}
	}
	fclose(file);

	return result;
}

ccError ccSysinfoInitialize(void)
{
	ccAssert(!_ccSysinfo);

	_ccSysinfo = malloc(sizeof(ccSysinfo));
	if(_ccSysinfo == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}

	long kb = getKBValueFromProc("/proc/meminfo", "MemTotal");
	if(kb == -1){
		return CC_E_OS;
	}
	_ccSysinfo->ramTotal = ((uint_fast64_t)kb) * 1000;

	_ccSysinfo->processorCount = sysconf(_SC_NPROCESSORS_CONF);

	_ccSysinfo->fileMaxOpen = sysconf(_SC_OPEN_MAX);

	return CC_E_NONE;
}

uint_fast64_t ccSysinfoGetRamAvailable(void)
{
	struct sysinfo meminfo;
	sysinfo(&meminfo);

	return meminfo.freeram * meminfo.mem_unit;
}

void ccSysinfoFree(void)
{
	ccAssert(_ccSysinfo);

	free(_ccSysinfo);
}

#endif
