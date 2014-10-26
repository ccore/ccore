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

ccFileDir ccFileDirOpen(const char *dir)
{
	ccFileDir fileDir;
	fileDir.data = malloc(sizeof(ccFileDir_lin));

	return fileDir;
}

ccReturn ccFileDirClose(ccFileDir dir)
{
	free(dir.data);
}

char *ccFileDirFind(ccFileDir dir)
{

}

ccReturn ccFileDirSeek(ccFileDir dir, unsigned int pos)
{

}

#endif
