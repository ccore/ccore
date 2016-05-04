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
	EXIT_ON_E(ccDisplayInitialize());

	EXIT_ON_E(ccWindowCreate((ccRect){0, 0, 500, 300}, "ccore examples: window", CC_WINDOW_FLAG_NORESIZE | CC_WINDOW_FLAG_ALWAYSONTOP));

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
							EXIT_ON_E(ccWindowSetTitle("ccWindowSetCentered"));
							EXIT_ON_E(ccWindowSetCentered());
							break;
						case 1:
							EXIT_ON_E(ccWindowSetTitle("ccWindowResizeMove"));
							EXIT_ON_E(ccWindowResizeMove((ccRect){0, 0, 300, 200}));
							break;
						case 2:
							EXIT_ON_E(ccWindowSetTitle("ccWindowSetBlink"));
							EXIT_ON_E(ccWindowSetBlink());
							break;
						case 3:
							EXIT_ON_E(ccWindowSetTitle("ccWindowSetMaximized"));
							EXIT_ON_E(ccWindowSetMaximized());
							break;
						case 4:
							EXIT_ON_E(ccWindowSetTitle("ccWindowSetCursor"));
							EXIT_ON_E(ccWindowMouseSetCursor(CC_CURSOR_BEAM));
							break;
						case 5:
							EXIT_ON_E(ccWindowSetTitle("ccWindowSetCursor"));
							EXIT_ON_E(ccWindowMouseSetCursor(CC_CURSOR_NONE));
							break;
						case 6:
							EXIT_ON_E(ccWindowMouseSetCursor(CC_CURSOR_ARROW));

							EXIT_ON_E(ccWindowSetTitle("ccWindowClipboardSet"));
							EXIT_ON_E(ccWindowClipboardSet("ccore window clipboard test"));
							break;
						case 7:
							EXIT_ON_E(ccWindowSetTitle("ccWindowClipboardGet"));
							printf("Clipboard contents: \"%s\"\n", ccWindowClipboardGet());
							break;
						case 8:
							EXIT_ON_E(ccWindowMouseSetPosition((ccPoint){150, 150}));
							break;
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
