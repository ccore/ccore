#include <ccore/file.h>

#if defined CC_USE_ALL || defined CC_USE_FILE

#include <sys/stat.h>
#ifdef WINDOWS
#define ccStat _stat
#elif defined LINUX
#define ccStat stat
#endif

ccFileInfo ccFileInfoGet(const char *file)
{
	struct ccStat sb;
	ccFileInfo info;

	if(ccStat(file, &sb) != 0){
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
