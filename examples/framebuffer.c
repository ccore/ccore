#ifndef CC_USE_FRAMEBUFFER
#define CC_USE_FRAMEBUFFER
#endif

#include <stdio.h>

#include <ccore/core.h>
#include <ccore/time.h>
#include <ccore/window.h>
#include <ccore/display.h>

#define CYCLE_DURATION 10

int main(int argc, char **argv)
{
	ccDisplayInitialize();
	ccWindowCreate((ccRect){0, 0, 300, 100}, "☐ccore examples: framebuffer", 0);

	void *pixels;
	ccFramebufferFormat format;
	ccError err;
	if((err = ccWindowFramebufferCreate(&format)) != CC_E_NONE){
		fprintf(stderr, "Something went wrong creating a framebuffer:\n");
		fprintf(stderr, "%s\n", ccErrorString(err));
		exit(1);
	}

	int nbytes = format / 8;
	int npixels = ccWindowGetRect().width * ccWindowGetRect().height;

	int cycles = 0;
	bool loop = true;
	while(loop){
		// Resize and rename after a second
		if(cycles == 1000 / CYCLE_DURATION){
			ccRect r = ccWindowGetRect();
			r.width += 100;
			r.height += 100;
			ccWindowResizeMove(r);

			ccWindowSetTitle("☑ccore examples: framebuffer");
		}

		while(ccWindowEventPoll()){
			ccEvent ev = ccWindowEventGet();
			switch(ev.type){
				case CC_EVENT_WINDOW_QUIT:
					loop = false;
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

		ccError err;
		if((err = ccWindowFramebufferUpdate()) != CC_E_NONE){
			fprintf(stderr, "Something went wrong updating the framebuffer:\n");
			fprintf(stderr, "%s\n", ccErrorString(err));
			exit(1);
		}

		ccTimeDelay(CYCLE_DURATION);
		cycles++;
	}

	ccFree();
}
