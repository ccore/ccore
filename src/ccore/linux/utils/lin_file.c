#if defined CC_USE_ALL || defined CC_USE_FILE

#include "lin_file.h"

static char *dataDir = NULL;

char *ccFileUserDirGet(void)
{
	return CC_USER_LOCATION;
}

char *ccFileDataDirGet(void)
{
#ifndef CC_DATA_LOCATION
	int len;

	if(dataDir == NULL){
		dataDir = malloc(PATH_MAX);
		len = readlink("/proc/self/exe", dataDir, PATH_MAX);
		if(len > 0){
			dataDir[len] = '\0';
			dataDir = dirname(dataDir);
			strcat(dataDir, "/");
		}
	}
	return dataDir;
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

}

ccReturn ccFileDirFindFirst(ccFileDir *dir, const char *dirPath)
{
	dir->dir = opendir(dirPath);
	if(!dir->dir){
		ccErrorPush(CC_ERROR_FILE_OPEN);
		return CC_FAIL;
	}

	return ccFileDirFind(dir);
}

ccReturn ccFileDirFind(ccFileDir *dir)
{
	if(CC_UNLIKELY(!dir->dir)){
		ccErrorPush(CC_ERROR_FILE_OPEN);
		return CC_FAIL;
	}

	if((dir->entry = readdir(dir->dir)) == NULL){
		dir->name = NULL;
		return CC_FAIL;
	}

	dir->name = dir->entry->d_name;
	dir->isDirectory = dir->entry->d_type == DT_DIR;

	return CC_SUCCESS;
}

ccReturn ccFileDirClose(ccFileDir *dir)
{
	if(dir->entry){
		free(dir->entry->d_name);
		free(dir->entry);
	}
	closedir(dir->dir);

	return CC_SUCCESS;
}

#endif
