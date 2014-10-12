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

	ccPrintf("\tFound %d gamepad(s)\n", ccGamepadCount());
	err();
	ccPrintf("Passed\n");
}

void testWindow(int *test)
{
	ccPrintf("Test %d: Window", ++(*test));

	ccDisplayInitialize();
	err();

	ccWindowCreate((ccRect){0, 0, 1, 1}, "ccore test", 0);
	err();

	ccWindowFree();
	err();

	ccWindowCreate((ccRect){0, 0, 100, 100}, "ccore test", 0);
	err();

	ccDisplayFree();
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
#ifndef LINUX //TODO fix bug #15
		if(ccDisplayResolutionGetAmount(display) <= 0){
			reportDiscrepancy("Display has no resolutions");
		}
		if(display->current > 0){
			ccDisplayResolutionSet(display, display->current - 1);
			err();
		}else if(ccDisplayResolutionGetAmount(display) > 0){
			ccDisplayResolutionSet(display, 1);
			err();
		}
#endif
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

void test10BytesFile(int *test)
{
	char *file;

	ccPrintf("Test %d: File with 10 bytes information", ++(*test));

	file = ccStringConcatenate(2, ccFileDataDirGet(), "10bytesfile.txt");
	err();

	if(ccFileInfoGet(file).size != 11){
		reportDiscrepancy("Size of 10bytesfile.txt");
	}
	err();
	ccPrintf(" - passed\n");
}

void testEmptyFile(int *test)
{
	char *file;

	ccPrintf("Test %d: Empty file information", ++(*test));

	file = ccStringConcatenate(2, ccFileDataDirGet(), "emptyfile.txt");
	err();

	if(ccFileInfoGet(file).size != 0){
		reportDiscrepancy("Size of empty.txt");
	}
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

int main(int argc, char **argv)
{
	int test;

	ccInitialize();
	
	test = 0;
	testDefaultDirectories(&test);
	testEmptyFile(&test);
	test10BytesFile(&test);
	testTime(&test);
	testDisplay(&test);
	testWindow(&test);
	testGamepad(&test);

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

