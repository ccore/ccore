//__________________________________________________________________________________//
//                               ______                                             //
//                              /  ___ /__  _ __ ___                                //
//                             /  / __/ _ \| '__/ _ \                               //
//                            |  | (_| (_) | | |  __/                               //
//                             \  \___\___/|_|  \___| 1.1                           //
//                              \______\                                            //
//                                                                                  //
//             Copyright (C) 2014 \ Job Talle (jobtalle@hotmail.com)                //
//                                 \ Thomas Versteeg (thomas@ccore.org)             //
//__________________________________________________________________________________//
//                                                                                  //
//      This program is free software: you can redistribute it and/or modify        //
//      it under the terms of the 3-clause BSD license.                             //
//                                                                                  //
//      You should have received a copy of the 3-clause BSD License along with      //
//      this program. If not, see <http://opensource.org/licenses/>.                //
//__________________________________________________________________________________//

#pragma once

#if defined CC_USE_ALL || defined CC_USE_FILE

#include "core.h"
#include "types.h"

#include <stdint.h>

#ifdef WINDOWS
#include <windows.h>
#elif defined LINUX
#include <sys/time.h>
#include <dirent.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	uint64_t size;
	time_t modified;
	time_t access;
} ccFileInfo;

typedef struct {
	char *name;
	bool isDirectory;
#ifdef WINDOWS
	HANDLE handle;
#elif defined LINUX
	DIR *dir;
	struct dirent *entry;
#endif
} ccFileDir;

// These functions can be used to get OS specific directories to store program data
char *ccFileUserDirGet(void);
char *ccFileDataDirGet(void);
char *ccFileTempDirGet(void);

// The directory functions can be used to read all files in a directory
ccError ccFileDirFindFirst(ccFileDir *dir, const char *dirPath);
ccError ccFileDirFind(ccFileDir *dir);
ccError ccFileDirClose(ccFileDir *dir);

ccFileInfo ccFileInfoGet(const char *file); 

void _ccFileFree(void);

#ifdef __cplusplus
}
#endif

#endif
