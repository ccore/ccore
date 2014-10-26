#pragma once

#if defined CC_USE_ALL || defined CC_USE_FILE

#include <Windows.h>
#include <string.h>

#include <ccore/file.h>

#include <ccore/string.h>

#define _CC_FILE_DIR_DATA(dir) ((ccFileDir_win*)dir->data)

typedef struct {
	int nothing;
} ccFileDir_win;

#endif
