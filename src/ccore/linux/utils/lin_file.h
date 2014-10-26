#pragma once

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

#endif
