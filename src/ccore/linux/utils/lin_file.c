#if defined CC_USE_ALL || defined CC_USE_FILE

#include "lin_file.h"

#ifndef CC_DATA_LOCATION
static char *datadir = NULL;
#endif

char *ccFileUserDirGet(void)
{
	return CC_USER_LOCATION;
}

char *ccFileDataDirGet(void)
{
#ifndef CC_DATA_LOCATION
	if(datadir == NULL){
		datadir = malloc(PATH_MAX);
		int len = readlink("/proc/self/exe", datadir, PATH_MAX);
		if(len > 0){
			datadir[len] = '\0';
			datadir = dirname(datadir);
			strcat(datadir, "/");
		}
	}
	return datadir;
#else
	return CC_DATA_LOCATION;
#endif
}

char *ccFileTempDirGet(void)
{
	return CC_TEMP_LOCATION;
}

void _ccFileFree(void)
{
#ifndef CC_DATA_LOCATION
	free(datadir);
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
