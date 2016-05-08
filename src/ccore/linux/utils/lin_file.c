#if defined CC_USE_ALL || defined CC_USE_FILE

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

#include <ccore/file.h>
#include <ccore/string.h>

#ifndef CC_USER_LOCATION
#define CC_USER_LOCATION "~/.config/"
#endif

#ifndef CC_TEMP_LOCATION
#define CC_TEMP_LOCATION "/tmp/"
#endif

#ifndef CC_DATA_LOCATION
static char *_dataDir = NULL;
#endif

char *ccFileUserDirGet(void)
{
	return CC_USER_LOCATION;
}

char *ccFileDataDirGet(void)
{
#ifndef CC_DATA_LOCATION
	if(_dataDir == NULL){
		_dataDir = malloc(PATH_MAX);
		int len = readlink("/proc/self/exe", _dataDir, PATH_MAX);
		if(len > 0){
			_dataDir[len] = '\0';
			_dataDir = dirname(_dataDir);
			strcat(_dataDir, "/");
		}
	}
	return _dataDir;
#else
	return CC_DATA_LOCATION;
#endif
}

char *ccFileTempDirGet(void)
{
	return CC_TEMP_LOCATION;
}

void ccFileFree(void)
{
#ifndef CC_DATA_LOCATION
	free(_dataDir);
#endif
}

ccError ccFileDirFindFirst(ccFileDir *dir, const char *dirpath)
{
	dir->dir = opendir(dirpath);
	if(!dir->dir){
		return CC_E_FILE_OPEN;
	}

	return ccFileDirFind(dir);
}

ccError ccFileDirFind(ccFileDir *dir)
{
	if(CC_UNLIKELY(!dir->dir)){
		return CC_E_INVALID_ARGUMENT;
	}

	if((dir->entry = readdir(dir->dir)) == NULL){
		dir->name = NULL;
		return CC_E_FILE_OPEN;
	}

	dir->name = dir->entry->d_name;
	dir->isDirectory = dir->entry->d_type == DT_DIR;

	return CC_E_NONE;
}

ccError ccFileDirClose(ccFileDir *dir)
{
	if(dir->entry){
		free(dir->entry->d_name);
		free(dir->entry);
	}
	closedir(dir->dir);

	return CC_E_NONE;
}

#endif
