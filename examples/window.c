#include <stdio.h>

#include <ccore/core.h>
#include <ccore/time.h>
#include <ccore/window.h>
#include <ccore/display.h>

int main(int argc, char **argv)
{
	ccError e;

	ccDisplayInitialize();

	if((e = ccWindowCreate((ccRect){0, 0, 300, 100}, "☐ccore examples: framebuffer", 0)) != CC_E_NONE){
		fprintf(stderr, "%s\n", ccErrorString(e));
	}

	bool loop = true;
	while(loop){	
		while(ccWindowEventPoll()){
			ccEvent ev = ccWindowEventGet();
			switch(ev.type){
				case CC_EVENT_WINDOW_QUIT:
					loop = false;
					break;
				default: break;
			}
		}
	}

	ccFree();
}
