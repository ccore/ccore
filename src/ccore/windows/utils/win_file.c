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

ccReturn ccFileDirFindFirst(ccFileDir *dir, char **filename, const char *dirPath)
{
	WIN32_FIND_DATA findData;
	unsigned int strLength;
	char *buffer;
	char *pathStr;
	pathStr = ccStringConcatenate(2, dirPath, "*");

	*dir = FindFirstFile(pathStr, &findData);

	free(pathStr);

	if(*dir == INVALID_HANDLE_VALUE) {
		return CC_FAIL;
	}
	
	strLength = strlen(findData.cFileName);
	buffer = malloc(strLength + 1);
	memcpy(buffer, findData.cFileName, strLength);
	buffer[strLength] = '\0';
	
	*filename = buffer;

	return CC_SUCCESS;
}

ccReturn ccFileDirFind(ccFileDir *dir, char **filename)
{
	WIN32_FIND_DATA findData;
	unsigned int strLength;
	char *buffer;

	if(FindNextFile(*dir, &findData) == 0) {
		if(GetLastError() == ERROR_NO_MORE_FILES) {
			*filename = NULL;
			return CC_SUCCESS;
		}
		return CC_FAIL;
	}

	strLength = strlen(findData.cFileName);
	buffer = malloc(strLength + 1);
	memcpy(buffer, findData.cFileName, strLength);
	buffer[strLength] = '\0';

	*filename = buffer;

	return CC_SUCCESS;
}

ccReturn ccFileDirClose(ccFileDir *dir)
{
	return FindClose(*dir) == 0?CC_FAIL:CC_SUCCESS;
}

#endif
