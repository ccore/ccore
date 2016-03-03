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

#include "core.h"
#include "error.h"
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
	CC_TEXT_UNHANDLED = 0,
	CC_TEXT_INPUT,
	CC_TEXT_EDITTING
} ccTextEventType;

typedef struct {
	ccTextEventType type;

	bool active;

	char text[32];
	int start, length;
} ccTextEvent;

ccReturn ccTextInputStart();
ccReturn ccTextInputStop();
ccReturn ccTextInputRect(ccRect rect);
bool ccTextInputIsActive();

#ifdef __cplusplus
}
#endif
