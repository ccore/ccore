#pragma once

#include <Windows.h>
#include <hidsdi.h>

#include <ccore/gamepad.h>

#include <ccore/assert.h>

#include "win_window.h"

#define GAMEPAD_MAXBUTTONS 128

ccGamepadEvent _generateGamepadEvent(RAWINPUT *raw);

typedef struct {
	HIDP_CAPS caps;
	PHIDP_BUTTON_CAPS buttonCaps;
	PHIDP_VALUE_CAPS valueCaps;
	int *axisNegativeComponent;
	double *axisFactor;
} ccGamepad_win;

typedef struct {
	int preparsedDataSize;
	PHIDP_PREPARSED_DATA preparsedData;
	USAGE usage[GAMEPAD_MAXBUTTONS];
} ccGamepads_win;

#define GAMEPAD_DATA ((ccGamepad_win*)currentGamepad->data)
#define GAMEPADS_DATA ((ccGamepads_win*)_gamepads->data)