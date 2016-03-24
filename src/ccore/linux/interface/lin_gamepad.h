#pragma once

#if defined CC_USE_ALL || defined CC_USE_GAMEPAD

#include <ccore/gamepad.h>

typedef struct {
	int id, fd, fffd, ffid;
} ccGamepad_lin;

typedef struct {
	int fd, watch;
} ccGamepads_lin;

#define GAMEPAD_DATA(gamepad) ((ccGamepad_lin*)(gamepad)->data)
#define GAMEPADS_DATA() ((ccGamepads_lin*)(_ccGamepads)->data)

ccGamepadEvent ccGamepadEventPoll(void);

#endif
