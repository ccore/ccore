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

	int cur = 0;
	bool loop = true;
	while(loop){	
		while(ccWindowEventPoll()){
			ccEvent ev = ccWindowEventGet();
			if(ev.type != CC_EVENT_SKIP){
				printf("Event: %s\n", ccEventString(ev));
			}
			switch(ev.type){
				case CC_EVENT_WINDOW_QUIT:
					loop = false;
					break;
				case CC_EVENT_MOUSE_DOWN:
					switch(cur){
						case 0:
							ccWindowSetCentered();
							break;
						case 1:
							ccWindowResizeMove((ccRect){0, 0, 300, 200});
							break;
						case 2:
							ccWindowSetTitle("☑ccore examples: window");
						default:
							cur = -1;
					}
					cur++;
				default: break;
			}
		}
	}

	ccFree();
}
