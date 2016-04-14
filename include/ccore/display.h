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

#include <string.h>

#include "core.h"

#include "types.h"
#include "assert.h"

#define CC_DEFAULT_RESOLUTION -1

#ifdef __cplusplus
extern "C"
{
#endif

// Stores display properties
typedef struct {
	int width, height, refreshRate, bitDepth;

	void *data;
} ccDisplayData;

// Display
typedef struct {
	ccDisplayData *resolution;
	int x, y;
	unsigned short dpi, amount, current, initial;
	char *gpuName;
	char *monitorName;

	char *deviceName;
	void *data;
} ccDisplay;

// List of all displays currently connected and active
typedef struct {
	ccDisplay* display;
	unsigned short amount, primary;
} ccDisplays;

// Only access through getters
ccDisplays *_ccDisplays;

#define ccDisplayResolutionGetCurrent(display) (&display->resolution[display->current])
#define ccDisplayResolutionGet(display, index) (&display->resolution[index])
#define ccDisplayResolutionGetAmount(display) (display->amount)

// Display
ccError ccDisplayInitialize(void); // Get all displays currently connected and active
ccError ccDisplayFree(void);
ccError ccDisplayRevertModes(void);

// Getters
ccRect ccDisplayGetRect(ccDisplay *display);
int ccDisplayGetAmount(void);
ccDisplay *ccDisplayGet(int index);
ccDisplay *ccDisplayGetDefault(void);

// Resolution
ccError ccDisplayResolutionSet(ccDisplay *display, int resolutionIndex);
bool ccDisplayResolutionExists(ccDisplay *display, ccDisplayData *resolution);
bool ccDisplayResolutionEqual(ccDisplayData *resolutionA, ccDisplayData *resolutionB);

#ifdef __cplusplus
}
#endif
