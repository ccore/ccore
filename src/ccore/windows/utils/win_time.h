#pragma once

#if defined CC_USE_ALL || defined CC_USE_TIME

#pragma comment(lib, "Winmm.lib")

#include <windows.h>

#include <ccore/time.h>

double _ticksToNanoSeconds = -1.0;

#endif
