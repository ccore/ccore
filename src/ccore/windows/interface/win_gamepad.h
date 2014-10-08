#pragma once

#ifdef CC_USE_GAMEPAD

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "XInput9_1_0.lib")

#include <Windows.h>
#include <hidsdi.h>
#include <Xinput.h>

#include <ccore/gamepad.h>

#include <ccore/assert.h>

#include "win_window.h"

#define GAMEPAD_MAXBUTTONS 128

#define GAMEPAD_XINPUT_BUTTONCOUNT 14
#define GAMEPAD_XINPUT_AXISCOUNT 6

#define GAMEPAD_XINPUT_ILEFTTRIGGER 0
#define GAMEPAD_XINPUT_IRIGHTTRIGGER 1
#define GAMEPAD_XINPUT_ITHUMBLX 2
#define GAMEPAD_XINPUT_ITHUMBLY 3
#define GAMEPAD_XINPUT_ITHUMBRX 4
#define GAMEPAD_XINPUT_ITHUMBRY 5

#define GAMEPAD_XINPUT_HAPTICAMOUNT 2

#define GAMEPAD_XINPUT_TRIGGER_FACTOR (SHRT_MAX / CHAR_MAX)

void _generateGamepadEvents(RAWINPUT *raw);
void _queryXinput();

typedef enum {
	CC_GAMEPAD_INPUT_RAW,
	CC_GAMEPAD_INPUT_XINPUT
} ccGamepadInputType;

typedef struct {
	HIDP_CAPS caps;
	PHIDP_BUTTON_CAPS buttonCaps;
	PHIDP_VALUE_CAPS valueCaps;
	PHIDP_PREPARSED_DATA preparsedData;
	HANDLE handle;
	int preparsedDataSize;
	int *axisNegativeComponent;
	double *axisFactor;
} ccGamepad_win_raw;

typedef struct {
	union {
		ccGamepad_win_raw *raw;
		int xinputIndex;
	};
	ccGamepadInputType inputType;
} ccGamepad_win;

typedef struct {
	USAGE usage[GAMEPAD_MAXBUTTONS];
	int xInputConnected[XUSER_MAX_COUNT];
} ccGamepads_win;

#define GAMEPAD_DATA ((ccGamepad_win*)currentGamepad->data)
#define GAMEPADS_DATA ((ccGamepads_win*)_ccGamepads->data)

#endif CC_USE_GAMEPAD