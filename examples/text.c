#ifndef CC_USE_TEXT
#define CC_USE_TEXT
#endif

#include <stdio.h>

#include <ccore/core.h>
#include <ccore/text.h>
#include <ccore/window.h>
#include <ccore/display.h>

int main(int argc, char **argv)
{
	ccDisplayInitialize();
	ccWindowCreate((ccRect){0, 0, 100, 100}, "ccore examples: text", 0);

	ccTextInputStart();

	ccTextInputRect((ccRect){0, 0, 100, 100});

	ccTextInputStop();

	ccFree();
	printf("Hi\n");
}
