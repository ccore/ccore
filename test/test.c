﻿//__________________________________________________________________________________//
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
//      it under the terms of the GNU General Public License as published by        //
//      the Free Software Foundation.                                               //
//                                                                                  //
//      This program is distributed without any warranty; see the GNU               //
//      General Public License for more details.                                    //
//                                                                                  //
//      You should have received a copy of the GNU General Public License           //
//      along with this program. If not, see <http://www.gnu.org/licenses/>.        //
//__________________________________________________________________________________//

#ifdef _DEBUG

// rand, srand
#include <stdlib.h>
// time
#include <time.h>

#include <ccore/sysinfo.h>
#include <ccore/print.h>
#include <ccore/file.h>
#include <ccore/time.h>
#include <ccore/string.h>
#include <ccore/gamepad.h>
#include <ccore/window.h>
#include <ccore/display.h>

#include "icon.h"

#ifdef WINDOWS
// Check for leaks
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#define err() _err(__LINE__)

void _err(int line)
{
	ccError error;
	bool die;

	die = false;
	while((error = ccErrorPop()) != CC_ERROR_NONE){
		ccPrintf("\nError on line %d:\t\"%s\"\n", line - 1, ccErrorString(error));		
		die = true;
	}

	if(die){
		exit(-1);
	}
}

void reportDiscrepancy(const char *where)
{
	ccPrintf("\n\nDiscrepancy detected between results and expected results:\n\t\"%s\"\n", where);
	err();
	exit(-1);
}

void testGamepad(int *test)
{
	ccPrintf("Test %d: Gamepad\n", ++(*test));

	ccGamepadInitialize();
	err();

	ccPrintf("\tFound %d gamepad(s)\n", ccGamepadGetAmount());
	err();
	ccPrintf("Passed\n");
}

void testWindow(int *test)
{
	bool quit; 
	unsigned long *iconData;

	ccPrintf("Test %d: Window", ++(*test));

	ccDisplayInitialize();
	err();

	ccWindowCreate((ccRect){0, 0, 1, 1}, "ccore test", 0);
	err();
	ccTimeDelay(500);
	ccWindowFree();
	err();

	ccWindowCreate((ccRect){ 0, 0, 1, 1 }, "ccore test", CC_WINDOW_FLAG_ALWAYSONTOP | CC_WINDOW_FLAG_NORESIZE | CC_WINDOW_FLAG_NORAWINPUT | CC_WINDOW_FLAG_NOBUTTONS);
	err();
	ccTimeDelay(500);
	ccWindowFree();
	err();
	
	ccWindowCreate((ccRect){0, 0, 100, 100}, "ccore test", CC_WINDOW_FLAG_ALWAYSONTOP);
	err();
	ccWindowSetCentered();
	err();
	ccTimeDelay(500);
	ccWindowSetMaximized();
	err();
	ccTimeDelay(500);
	ccWindowSetFullscreen(1, ccDisplayGetDefault());
	err();
	ccTimeDelay(500);

	ccWindowClipboardSet("ccore test");
	err();
	ccWindowSetWindowed(&(ccRect){ 0, 0, 300, 100 });
	err();
	ccWindowSetCentered();
	err();
	iconData = iconGetData();
	ccWindowIconSet(iconGetSize(), iconData);
	free(iconData);
	err();
	ccWindowMouseSetPosition((ccPoint){0, 0});
	err();
	ccWindowMouseSetCursor(CC_CURSOR_NONE);
	err();
	ccTimeDelay(500);
	ccWindowMouseSetCursor(CC_CURSOR_ARROW);
	err();
	ccWindowSetTitle("òèçùà12345");
	err();

	quit = false;
	while(!quit){
		while(ccWindowEventPoll()) {
			err();
			switch(ccWindowEventGet().type) {
				case CC_EVENT_WINDOW_QUIT:
					quit = true;	
					break;
				case CC_EVENT_FOCUS_LOST:
					ccWindowSetBlink();
					err();
					break;
				case CC_EVENT_MOUSE_DOWN:
					switch(ccWindowEventGet().mouseButton) {
					case CC_MOUSE_BUTTON_PREVIOUS: printf("Previous\n"); break;
					case CC_MOUSE_BUTTON_NEXT: printf("Next\n"); break;
					}
			}
		}
	}
	
	ccDisplayRevertModes();
	err();

	ccPrintf(" - passed\n");
}

void testDisplay(int *test)
{
	ccDisplay *display;

	ccPrintf("Test %d: Display\n", ++(*test));

	ccDisplayInitialize();
	err();

	ccPrintf("\tFound %d display(s)\n", ccDisplayGetAmount());
	err();
	if(ccDisplayGetAmount() > 0){
		display = ccDisplayGetDefault();
		err();
		if(ccDisplayResolutionGetAmount(display) <= 0){
			reportDiscrepancy("Display has no resolutions");
		}

		srand((unsigned int)time(NULL));
		ccDisplayResolutionSet(display, rand() % ccDisplayResolutionGetAmount(display));

		ccTimeDelay(5000);
		err();

		ccDisplayRevertModes();
		err();
	}

	ccDisplayFree();
	err();
	ccPrintf("Passed\n");
}

void testTime(int *test)
{
	ccPrintf("Test %d: Time delay of 1 second", ++(*test));
	fflush(stdout);

	ccTimeDelay(1000);
	err();
	ccPrintf(" - passed\n");
}

void testDefaultDirectories(int *test)
{
	ccPrintf("Test %d: Default directories\n", ++(*test));
	ccPrintf("\tUser directory: %s\n", ccFileUserDirGet());
	err();
	ccPrintf("\tData directory: %s\n", ccFileDataDirGet());
	err();
	ccPrintf("\tTemp directory: %s\n", ccFileTempDirGet());
	err();
	ccPrintf("Passed\n");
}

void testSysinfo(int *test)
{
	ccPrintf("Test %d: Retrieving system info\n", ++(*test));

	ccSysinfoInitialize();
	err();

	ccPrintf("\tInstalled RAM:\t%lld\n", ccSysinfoGetRamTotal());
	ccPrintf("\tAvailable RAM:\t%lld\n", ccSysinfoGetRamUsable());

	ccPrintf("\tProcessors:\t%d\n", ccSysinfoGetProcessorCount());

	ccPrintf("\tFree RAM:\t%lld\n", ccSysinfoGetRamAvailable());

	ccSysinfoFree();
	err();

	ccPrintf("Passed\n");
}

int main(int argc, char **argv)
{
	int test;

	ccInitialize();
	
	test = 0;
	testSysinfo(&test);
	testDefaultDirectories(&test);
	testTime(&test);
	//testDisplay(&test);
	//testWindow(&test);
	//testGamepad(&test);

	ccFree();

	return 0;
}

#else

#include <stdio.h>

int main(int argc, char **argv)
{
	printf("Please enable the _DEBUG flag\n");
	return -1;
}

#endif

