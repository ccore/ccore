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

#define MAX_CYCLES 500
#define CYCLE_DURATION 5

int main(int argc, char **argv)
{
	ccDisplayInitialize();
	ccWindowCreate((ccRect){0, 0, 405, 400}, "ccore examples: framebuffer", 0);

	void *pixels;
	ccFramebufferFormat format;
	if(ccWindowFramebufferCreate(&pixels, &format) != CC_SUCCESS){
		fprintf(stderr, "Something went wrong creating a framebuffer:\n");
		fprintf(stderr, "%s\n", ccErrorString(ccErrorPop()));
		exit(1);
	}

	int nbytes = format / 8;
	int npixels = ccWindowGetRect().width * ccWindowGetRect().height;

	int cycles = 0;
	while(cycles++ < MAX_CYCLES){
		// Resize after a quarter of the time
		if(cycles == MAX_CYCLES / 4){
			ccRect r = ccWindowGetRect();
			r.x -= 100;
			r.y -= 100;
			r.width += 200;
			r.height += 200;
			ccWindowResizeMove(r);
		}

		while(ccWindowEventPoll()){
			ccEvent ev = ccWindowEventGet();
			switch(ev.type){
				// Loop terminates when cycles == MAX_CYCLES
				case CC_EVENT_WINDOW_QUIT:
					cycles = MAX_CYCLES;
					break;
				// Recalculate the amount of pixels for the framebuffer on a resize event
				case CC_EVENT_WINDOW_RESIZE:
					npixels = ccWindowGetRect().width * ccWindowGetRect().height;
					break;
				default: break;
			}
		}

		int i;
		for(i = 0; i < npixels * nbytes; i += nbytes){
			((char*)pixels)[i] = i / 4 + cycles;
			((char*)pixels)[i + 1] = i / 7 + cycles;
			((char*)pixels)[i + 2] = i / 200 + cycles;
		}

		if(ccWindowFramebufferUpdate(&pixels) != CC_SUCCESS){
			fprintf(stderr, "Something went wrong updating the framebuffer:\n");
			fprintf(stderr, "%s\n", ccErrorString(ccErrorPop()));
			exit(1);
		}

		ccTimeDelay(CYCLE_DURATION);
	}

	ccFree();
}
