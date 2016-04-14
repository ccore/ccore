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

#if defined CC_USE_ALL || defined CC_USE_SYSINFO

#include <stdint.h>

#include "core.h"

#include "types.h"
#include "error.h"
#include "assert.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	uint_fast64_t ramTotal;

	unsigned int processorCount;

	unsigned int fileMaxOpen;
} ccSysinfo;

ccSysinfo *_ccSysinfo;

#define ccSysinfoGetProcessorCount() _ccSysinfo->processorCount
#define ccSysinfoGetFileMaxOpen() _ccSysinfo->fileMaxOpen

#define ccSysinfoGetRamTotal() _ccSysinfo->ramTotal

ccError ccSysinfoInitialize(void);
void ccSysinfoFree(void);

#ifdef __cplusplus
}
#endif

#endif
