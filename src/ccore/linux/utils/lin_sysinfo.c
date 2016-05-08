#include "lin_sysinfo.h"

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

static uint_fast64_t _ramTotal = 0;
static unsigned int _processorCount = 0;
static unsigned int _fileMaxOpen = 0;

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

ccError ccSysinfoGetRamAvailable(uint_fast64_t *ram)
{
	struct sysinfo meminfo;
	sysinfo(&meminfo);

	*ram = meminfo.freeram * meminfo.mem_unit;

	return CC_E_NONE;
}

ccError ccSysinfoGetRamTotal(uint_fast64_t *ram)
{
	if(CC_UNLIKELY(_ramTotal == 0)){
		long kb = getKBValueFromProc("/proc/meminfo", "MemTotal");
		if(kb == -1){
			return CC_E_OS;
		}
		_ramTotal = ((uint_fast64_t)kb) * 1000;
	}

	*ram = _ramTotal;

	return CC_E_NONE;
}

ccError ccSysinfoGetProcessorCount(unsigned int *processors)
{
	if(CC_UNLIKELY(_processorCount == 0)){
		_processorCount = sysconf(_SC_NPROCESSORS_CONF);
	}

	*processors = _processorCount;

	return CC_E_NONE;
}

ccError ccSysinfoGetFileMaxOpen(unsigned int *maxFiles)
{
	if(CC_UNLIKELY(_fileMaxOpen == 0)){
		_fileMaxOpen = sysconf(_SC_OPEN_MAX);
	}

	*maxFiles = _fileMaxOpen;

	return CC_E_NONE;
}

#endif
