#pragma once

#include <ccore/display.h>
#include <ccore/assert.h>
#include <ccore/print.h>

#include <dirent.h>
#include <string.h>

#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xinerama.h>

typedef struct {
	RRMode XMode;
} ccDisplayData_x11;

typedef struct {
	int XScreen, XineramaScreen;
	RROutput XOutput;
	RRMode XOldMode;
} ccDisplay_x11;