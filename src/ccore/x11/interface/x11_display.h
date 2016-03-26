#pragma once

#include <X11/extensions/Xrandr.h>

typedef struct {
	RRMode XMode;
} ccDisplayData_x11;

typedef struct {
	int XScreen, XineramaScreen;
	RROutput XOutput;
	RRMode XOldMode;
} ccDisplay_x11;

#define DISPLAY_DATA(display) ((ccDisplay_x11 *)display->data)
