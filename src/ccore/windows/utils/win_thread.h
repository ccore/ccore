#pragma once

#if defined CC_USE_ALL || defined CC_USE_THREAD

#include <Windows.h>

#include <ccore/thread.h>

#define _CC_CRITICAL_SECTION_SPIN_COUNT 400

#endif