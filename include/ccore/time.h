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

#if defined CC_USE_ALL || defined CC_USE_TIME

#include <stdint.h>

#include "core.h"
#include "types.h"

#define _TO_SECONDS 1000000000LL
#define _TO_MILLISECONDS 1000000LL
#define _TO_MICROSECONDS 1000LL

#define ccTimeSeconds() (ccTimeNanoseconds() / _TO_SECONDS)
#define ccTimeMilliseconds() (ccTimeNanoseconds() / _TO_MILLISECONDS)
#define ccTimeMicroseconds() (ccTimeNanoseconds() / _TO_MICROSECONDS)

#ifdef __cplusplus
extern "C"
{
#endif

ccReturn ccTimeDelay(int ms);
uint64_t ccTimeNanoseconds(void);

#ifdef __cplusplus
}
#endif

#elif defined __GNUC__
#error "The CC_USE_TIME or the CC_USE_ALL flag must be set"
#endif
