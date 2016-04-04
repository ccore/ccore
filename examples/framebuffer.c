#ifndef CC_USE_FRAMEBUFFER
#define CC_USE_FRAMEBUFFER
#endif

#include <stdio.h>

#include <ccore/core.h>
#include <ccore/sysinfo.h>
#include <ccore/print.h>
#include <ccore/file.h>
#include <ccore/time.h>
#include <ccore/string.h>
#include <ccore/gamepad.h>
#include <ccore/window.h>
#include <ccore/display.h>

int main(int argc, char **argv)
{
	ccDisplayInitialize();
	ccWindowCreate((ccRect){0, 0, 400, 400}, "ccore examples: framebuffer", 0);

	void *pixels;
	ccFramebufferFormat format;
	if(ccWindowFramebufferCreate(&pixels, &format) != CC_SUCCESS){
		fprintf(stderr, "Something went wrong creating a framebuffer:\n");
		fprintf(stderr, "%s\n", ccErrorString(ccErrorPop()));
		exit(1);
	}

	int nbytes = format / 8;
	int npixels = ccWindowGetRect().width * ccWindowGetRect().height;

	int cycles = 10;
	while(cycles--){
		for(int i = 0; i < npixels * nbytes; i++){
			((char*)pixels)[i] = 0;
		}

		if(ccWindowFramebufferUpdate() != CC_SUCCESS){
			fprintf(stderr, "Something went wrong updating the framebuffer:\n");
			fprintf(stderr, "%s\n", ccErrorString(ccErrorPop()));
			exit(1);
		}
		ccTimeDelay(500);
	}

	ccFree();
}
