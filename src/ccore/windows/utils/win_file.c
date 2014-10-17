#if defined CC_USE_ALL || defined CC_USE_FILE

#include "win_file.h"

#define USERHOME "%HOMEPATH%/"

static char *userDir;
static char *dataDir;
static char *tempDir;

static void scanDirs(void)
{
	HMODULE hModule = GetModuleHandleW(NULL);
	char path[MAX_PATH];
	int pathlength;

	//Fetch absolue .exe path
	pathlength = GetModuleFileName(hModule, path, MAX_PATH);
	
	dataDir = calloc(pathlength + 1, sizeof(char));
	memcpy(dataDir, path, pathlength);
	ccStringTrimToChar(dataDir, '\\', true);
	ccStringReplaceChar(dataDir, '\\', '/');

	//User dir
	userDir = USERHOME;

	//Temp directory
	pathlength = GetTempPath(MAX_PATH, path);

	tempDir = calloc(pathlength + 1, sizeof(char));
	memcpy(tempDir, path, pathlength);
	ccStringReplaceChar(tempDir, '\\', '/');
}

char *ccFileUserDirGet(void)
{
	if(userDir == NULL) scanDirs();
	return userDir;
}

char *ccFileDataDirGet(void)
{
	if(userDir == NULL) scanDirs();
	return dataDir;
}

char *ccFileTempDirGet(void)
{
	if(userDir == NULL) scanDirs();
	return tempDir;
}

void _ccFileFree(void)
{
	if(userDir == NULL) return;
	free(dataDir);
	free(tempDir);
	userDir = NULL;
}

#endif
