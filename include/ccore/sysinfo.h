//__________________________________________________________________________________//
//                               ______                                             //
//                              /  ___ /__  _ __ ___                                //
//                             /  / __/ _ \| '__/ _ \                               //
//                            |  | (_| (_) | | |  __/                               //
//                             \  \___\___/|_|  \___| 1.1                           //
//                              \______\                                            //
//                                                                                  //
//             Copyright (C) 2014 \ Job Talle (job@ccore.org)                       //
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
	uint_least64_t ramTotal;
	uint_least64_t pageSize;
	uint_least64_t pageTotalCount;
	unsigned int processorTotalCount;

	unsigned long fileMaxOpen;
	unsigned long stringMaxLength;
} ccSysinfo;

ccSysinfo *_ccSysinfo;

#define ccSysinfoGetRamTotal() _ccSysinfo->ramTotal
#define ccSysinfoGetPageSize() _ccSysinfo->pageSize
#define ccSysinfoGetPageTotalCount() _ccSysinfo->pageTotalCount
#define ccSysinfoGetProcessorTotalCount() _ccSysinfo->processorTotalCount
#define ccSysinfoGetFileMaxOpen() _ccSysinfo->fileMaxOpen
#define ccSysinfoGetStringMaxLength() _ccSysinfo->stringMaxLength

unsigned long ccSysinfoGetRamAvailable();
unsigned long ccSysinfoGetPageAvailableCount();
unsigned int ccSysinfoGetProcessorAvailableCount();

ccReturn ccSysinfoInitialize(void);
void ccSysinfoFree(void);

#ifdef __cplusplus
}
#endif

#elif defined __GNUC__
#error "The CC_USE_SYSINFO or the CC_USE_ALL flag must be set"
#endif
