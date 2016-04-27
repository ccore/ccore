#include <stdio.h>

#include <ccore/core.h>
#include <ccore/time.h>
#include <ccore/window.h>
#include <ccore/display.h>

int main(int argc, char **argv)
{
	ccError e;

	ccDisplayInitialize();

	if((e = ccWindowCreate((ccRect){0, 0, 300, 100}, "☐ccore examples: window", CC_WINDOW_FLAG_NORESIZE | CC_WINDOW_FLAG_ALWAYSONTOP)) != CC_E_NONE){
		fprintf(stderr, "%s\n", ccErrorString(e));
	}

	int events = 0;
	bool loop = true;
	while(loop){	
		while(ccWindowEventPoll()){
			ccEvent ev = ccWindowEventGet();
			switch(ev.type){
				case CC_EVENT_WINDOW_QUIT:
					switch(events){
						case 0:
							ccWindowSetCentered();
							break;
						case 1:
							ccWindowResizeMove((ccRect){0, 0, 300, 200});
							break;
						default:
							loop = false;
					}
					events++;
					break;
				default: break;
			}
		}
	}

	ccFree();
}
