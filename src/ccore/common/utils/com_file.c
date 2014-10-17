#include <ccore/file.h>

#if defined CC_USE_ALL || defined CC_USE_FILE

#ifdef WINDOWS

#define ccStat _stat

#elif defined LINUX

#define ccStat stat

#endif

ccFileInfo ccFileInfoGet(char *file)
{
	struct ccStat sb;
	ccFileInfo info;

	if(ccStat(file, &sb) != 0){
		ccErrorPush(CC_ERROR_FILE_OPEN);
		info.size = 0;
		info.modified = 0;
		return info;

	}
	
	info.size = (uint64_t)sb.st_size;
	info.modified = (time_t)sb.st_mtime;
	info.access = (time_t)sb.st_atime;

	return info;
}

#endif