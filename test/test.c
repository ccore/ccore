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

#include <ccore/display.h>
#include <ccore/window.h>
#include <ccore/event.h>
#include <ccore/opengl.h>
#include <ccore/time.h>
#include <ccore/file.h>
#include <ccore/string.h>
#include <ccore/thread.h>
#include <ccore/print.h>
#include <ccore/gamepad.h>

#include "icon.h"

#ifdef WINDOWS
// Check for leaks
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

void checkErrors()
{
	ccError error;
	bool die;

	while((error = ccErrorPop()) != CC_ERROR_NONE){
		ccPrintf("\nError:\t\"%s\"\n", ccErrorString(error));		
		die = true;
	}

	if(die){
		exit(-1);
	}
}

void reportDiscrepancy(const char *where)
{
	ccPrintf("\nDiscrepancy detected between results and expected results:\n\t\"%s\"\n", where);
	exit(-1);
}

void test10BytesFile(int *test)
{
	char *file;

	ccPrintf("Test %d: File with 10 bytes information\n", ++(*test));

	file = ccStringConcatenate(2, ccFileDataDirGet(), "10bytesfile.txt");
	checkErrors();

	ccPrintf("\tGetting filesize of: %s\n", file);
	if(ccFileInfoGet(file).size != 11){
		reportDiscrepancy("Size of 10bytesfile.txt");
	}
	checkErrors();
}

void testEmptyFile(int *test)
{
	char *file;

	ccPrintf("Test %d: Empty file information\n", ++(*test));

	file = ccStringConcatenate(2, ccFileDataDirGet(), "emptyfile.txt");
	checkErrors();

	ccPrintf("\tGetting filesize of: %s\n", file);
	if(ccFileInfoGet(file).size != 0){
		reportDiscrepancy("Size of empty.txt");
	}
	checkErrors();
}

void testDefaultDirectories(int *test)
{
	ccPrintf("Test %d: Default directories\n", ++(*test));
	ccPrintf("\tUser directory: %s\n", ccFileUserDirGet());
	ccPrintf("\tData directory: %s\n", ccFileDataDirGet());
	ccPrintf("\tTemp directory: %s\n", ccFileTempDirGet());

	checkErrors();
}

int main(int argc, char **argv)
{
	int test;

	ccInitialize();
	
	test = 0;
	testDefaultDirectories(&test);
	testEmptyFile(&test);
	test10BytesFile(&test);

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

