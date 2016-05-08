#if defined CC_USE_ALL || defined CC_USE_FILE

#include <windows.h>
#include <string.h>

#include <ccore/file.h>
#include <ccore/string.h>

#define USERHOME "%HOMEPATH%/"

static char *_userDir = NULL;
static char *_dataDir = NULL;
static char *_tempDir = NULL;

static void scanDirs(void)
{
	HMODULE hModule = GetModuleHandleW(NULL);
	char path[MAX_PATH];
	int pathlength;

	//Fetch absolue .exe path
	pathlength = GetModuleFileName(hModule, path, MAX_PATH);
	
	_dataDir = calloc(pathlength + 1, sizeof(char));
	memcpy(_dataDir, path, pathlength);
	ccStringTrimToChar(_dataDir, '\\', true);
	ccStringReplaceChar(_dataDir, '\\', '/');

	//User dir
	_userDir = USERHOME;

	//Temp directory
	pathlength = GetTempPath(MAX_PATH, path);

	_tempDir = calloc(pathlength + 1, sizeof(char));
	memcpy(_tempDir, path, pathlength);
	ccStringReplaceChar(_tempDir, '\\', '/');
}

char *ccFileUserDirGet(void)
{
	if(_userDir == NULL) scanDirs();
	return _userDir;
}

char *ccFileDataDirGet(void)
{
	if(_userDir == NULL) scanDirs();
	return _dataDir;
}

char *ccFileTempDirGet(void)
{
	if(_userDir == NULL) scanDirs();
	return _tempDir;
}

void _ccFileFree(void)
{
	if(_userDir == NULL) return;
	free(_dataDir);
	free(_tempDir);
	_userDir = NULL;
}

ccError ccFileDirFindFirst(ccFileDir *dir, const char *dirPath)
{
	WIN32_FIND_DATA findData;
	unsigned int strLength;
	char *buffer;
	char *pathStr;
	pathStr = ccStringConcatenate(2, dirPath, "*");

	dir->handle = FindFirstFile(pathStr, &findData);

	free(pathStr);

	if(dir->handle == INVALID_HANDLE_VALUE) {
		return CC_E_FILE_OPEN;
	}
	
	strLength = (unsigned int)strlen(findData.cFileName);
	buffer = malloc(strLength + 1);
	memcpy(buffer, findData.cFileName, strLength);
	buffer[strLength] = '\0';
	
	dir->isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)?true:false;

	dir->name = buffer;

	return CC_E_NONE;
}

ccError ccFileDirFind(ccFileDir *dir)
{
	WIN32_FIND_DATA findData;
	unsigned int strLength;
	char *buffer;

	if(FindNextFile(dir->handle, &findData) == 0) {
		if(GetLastError() == ERROR_NO_MORE_FILES) {
			dir->name = NULL;
			return CC_E_NONE;
		}
		return CC_E_FILE_OPEN;
	}

	strLength = (unsigned int)strlen(findData.cFileName);
	buffer = malloc(strLength + 1);
	memcpy(buffer, findData.cFileName, strLength);
	buffer[strLength] = '\0';

	dir->isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)?true:false;

	dir->name = buffer;

	return CC_E_NONE;
}

ccError ccFileDirClose(ccFileDir *dir)
{
	return FindClose(dir->handle) == 0?CC_E_FILE_OPEN:CC_E_NONE;
}

#endif
