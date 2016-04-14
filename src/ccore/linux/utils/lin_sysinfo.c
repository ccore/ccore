#include "lin_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

static long parseLineKB(char *line)
{
	int i;
	long value;

	i = strlen(line);
	while(*line < '0' || *line > '9'){
		line++;
	}
	line[i - 3] = '\0';

	return atoi(line);
}

static long getKBValueFromProc(char *proc, char *value)
{
	FILE *file;
	long result;
	int valueLen;
	char line[128];

	result = -1;
	file = fopen(proc, "r");
	if(!file){
		return result;
	}
	valueLen = strlen(value) - 1;
	while(fgets(line, 128, file) != NULL){
		if(strncmp(line, value, valueLen) == 0){
			result = parseLineKB(line);
			break;
		}
	}
	fclose(file);

	return result;
}

ccError ccSysinfoInitialize(void)
{
	long value;

	ccAssert(_ccSysinfo == NULL);

	_ccSysinfo = malloc(sizeof(ccSysinfo));
	if(_ccSysinfo == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}

	value = getKBValueFromProc("/proc/meminfo", "MemTotal");
	if(value == -1){
		return CC_E_OS;
	}
	_ccSysinfo->ramTotal = ((uint_fast64_t)value) * 1000;

	_ccSysinfo->processorCount = sysconf(_SC_NPROCESSORS_CONF);

	_ccSysinfo->fileMaxOpen = sysconf(_SC_OPEN_MAX);

	return CC_E_NONE;
}

uint_fast64_t ccSysinfoGetRamAvailable(void)
{
	struct sysinfo memInfo;
	unsigned long available;

	sysinfo(&memInfo);

	available = memInfo.freeram;
	return available * memInfo.mem_unit;
}

void ccSysinfoFree(void)
{
	free(_ccSysinfo);
}

#endif
