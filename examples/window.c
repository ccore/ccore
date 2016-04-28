#include <stdio.h>

#include <ccore/core.h>
#include <ccore/time.h>
#include <ccore/window.h>
#include <ccore/display.h>

#include "icon.h"

#define EXIT_ON_E(x) {\
	ccError e = x; \
	if(e != CC_E_NONE){ \
		fprintf(stderr, "Line %d error: %s\n\t" #x ";\n", __LINE__, ccErrorString(e)); \
		exit(1); \
	} \
}

int main(int argc, char **argv)
{
	ccError e;

	ccDisplayInitialize();

	EXIT_ON_E(ccWindowCreate((ccRect){0, 0, 300, 100}, "☐ccore examples: window", CC_WINDOW_FLAG_NORESIZE | CC_WINDOW_FLAG_ALWAYSONTOP));

	uint32_t *icondata = iconGetData();
	EXIT_ON_E(ccWindowIconSet(iconGetSize(), icondata));
	free(icondata);

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
							EXIT_ON_E(ccWindowSetCentered());
							break;
						case 1:
							EXIT_ON_E(ccWindowResizeMove((ccRect){0, 0, 300, 200}));
							break;
						case 2:
							EXIT_ON_E(ccWindowSetTitle("☑ccore examples: window"));
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
