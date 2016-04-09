#ifndef CC_USE_FRAMEBUFFER
#define CC_USE_FRAMEBUFFER
#endif

#include <stdio.h>

#include <ccore/core.h>
#include <ccore/time.h>
#include <ccore/window.h>
#include <ccore/display.h>

#define MAX_CYCLES 500
#define CYCLE_DURATION 10

int main(int argc, char **argv)
{
	ccDisplayInitialize();
	ccWindowCreate((ccRect){0, 0, 100, 100}, "ccore examples: framebuffer", 0);

	void *pixels;
	ccFramebufferFormat format;
	if(ccWindowFramebufferCreate(&format) != CC_SUCCESS){
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
			r.width += 100;
			r.height += 100;
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
		char* pixels = (char*)ccWindowFramebufferGetPixels();
		for(i = 0; i < npixels * nbytes; i += nbytes){
			pixels[i] = i / 4 + cycles;
			pixels[i + 1] = i / 7 + cycles;
			pixels[i + 2] = i / 200 + cycles;
		}

		if(ccWindowFramebufferUpdate() != CC_SUCCESS){
			fprintf(stderr, "Something went wrong updating the framebuffer:\n");
			fprintf(stderr, "%s\n", ccErrorString(ccErrorPop()));
			exit(1);
		}

		ccTimeDelay(CYCLE_DURATION);
	}

	ccFree();
}
