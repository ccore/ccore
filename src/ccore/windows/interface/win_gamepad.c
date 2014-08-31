#include "win_gamepad.h"

static int _gamepadXinputButtons[] =
{
	XINPUT_GAMEPAD_A,
	XINPUT_GAMEPAD_B,
	XINPUT_GAMEPAD_Y,
	XINPUT_GAMEPAD_X,
	XINPUT_GAMEPAD_DPAD_LEFT,
	XINPUT_GAMEPAD_DPAD_UP,
	XINPUT_GAMEPAD_DPAD_RIGHT,
	XINPUT_GAMEPAD_DPAD_DOWN,
	XINPUT_GAMEPAD_LEFT_THUMB,
	XINPUT_GAMEPAD_RIGHT_THUMB,
	XINPUT_GAMEPAD_LEFT_SHOULDER,
	XINPUT_GAMEPAD_RIGHT_SHOULDER,
	XINPUT_GAMEPAD_BACK,
	XINPUT_GAMEPAD_START
};

ccError ccGamepadOutputSet(ccGamepad *gamepad, int hapticIndex, int force)
{
	int i;
	XINPUT_VIBRATION vibration;

	ccAssert(gamepad != NULL);

	if(hapticIndex >= gamepad->outputAmount) return CC_ERROR_NOHAPTIC;

	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	if(((ccGamepad_win*)gamepad->data)->inputType == CC_GAMEPAD_INPUT_XINPUT) {
		(ccGamepad_win*)gamepad->output[hapticIndex] = force;

		vibration.wLeftMotorSpeed = (WORD)(ccGamepad_win*)gamepad->output[0];
		vibration.wRightMotorSpeed = (WORD)(ccGamepad_win*)gamepad->output[1];

		if(XInputSetState(((ccGamepad_win*)gamepad->data)->xinputIndex, &vibration) != ERROR_SUCCESS) return CC_ERROR_NOHAPTIC;
	}

	return CC_ERROR_NONE;
}

ccError ccGamepadInitialize(void)
{
	int i;
	ccAssert(_window != NULL);

	ccMalloc(_gamepads, sizeof(ccGamepads));
	ccMalloc(_gamepads->data, sizeof(ccGamepads_win));
	_gamepads->amount = 0;
	_gamepads->gamepad = NULL;
	
	WINDOW_DATA->queryXinput = true;

	for(i = 0; i < XUSER_MAX_COUNT; i++) {
		GAMEPADS_DATA->xInputConnected[i] = -1;
	}

	WINDOW_DATA->rid[RAWINPUT_GAMEPAD].usUsagePage = 1;
	WINDOW_DATA->rid[RAWINPUT_GAMEPAD].usUsage = 4;
	WINDOW_DATA->rid[RAWINPUT_GAMEPAD].dwFlags = 0;
	WINDOW_DATA->rid[RAWINPUT_GAMEPAD].hwndTarget = WINDOW_DATA->winHandle;

	return RegisterRawInputDevices(&WINDOW_DATA->rid[RAWINPUT_GAMEPAD], 1, sizeof(RAWINPUTDEVICE)) == TRUE?CC_ERROR_NONE:CC_ERROR_NOGAMEPAD;
}

void ccGamepadFree(void)
{
	ccAssert(_gamepads != NULL);
	
	WINDOW_DATA->queryXinput = false;

	WINDOW_DATA->rid[RAWINPUT_GAMEPAD].dwFlags = RIDEV_REMOVE;
	WINDOW_DATA->rid[RAWINPUT_GAMEPAD].hwndTarget = NULL;
	
	RegisterRawInputDevices(&WINDOW_DATA->rid[RAWINPUT_GAMEPAD], 1, sizeof(RAWINPUTDEVICE));

	if(ccGamepadCount() != 0) {
		int i;
		for(i = 0; i < ccGamepadCount(); i++) {
			if(((ccGamepad_win*)_gamepads->gamepad[i].data)->inputType == CC_GAMEPAD_INPUT_RAW) {
				free(((ccGamepad_win*)_gamepads->gamepad[i].data)->raw->buttonCaps);
				free(((ccGamepad_win*)_gamepads->gamepad[i].data)->raw->valueCaps);
				free(((ccGamepad_win*)_gamepads->gamepad[i].data)->raw->outputValueCaps);
				free(((ccGamepad_win*)_gamepads->gamepad[i].data)->raw->axisFactor);
				free(((ccGamepad_win*)_gamepads->gamepad[i].data)->raw->axisNegativeComponent);
				free(((ccGamepad_win*)_gamepads->gamepad[i].data)->raw->preparsedData);
				free(((ccGamepad_win*)_gamepads->gamepad[i].data)->raw);
			}
			free(_gamepads->gamepad[i].data);
			free(_gamepads->gamepad[i].button);
			free(_gamepads->gamepad[i].axis);
			if(_gamepads->gamepad[i].outputAmount != 0)
				free(_gamepads->gamepad[i].output);
		}
		free(_gamepads->gamepad);
	}

	free(_gamepads->data);
	free(_gamepads);

	_gamepads = NULL;
}

void _queryXinput()
{
	int i, j;
	ccGamepad *currentGamepad;

	for(i = 0; i < XUSER_MAX_COUNT; i++) {
		XINPUT_STATE state;
		DWORD result;
		ccEvent event;

		event.type = CC_EVENT_GAMEPAD;

		ZeroMemory(&state, sizeof(XINPUT_STATE));
		result = XInputGetState(i, &state);
		if(result == ERROR_SUCCESS) {
			int axisValue;

			// Handle creating or reconnecting
			if(GAMEPADS_DATA->xInputConnected[i] == -1 || ccGamepadGet(GAMEPADS_DATA->xInputConnected[i])->plugged == false) {
				if(GAMEPADS_DATA->xInputConnected[i] == -1) {
					// Allocate memory for newly connected gamepad
					_gamepads->amount++;
					_gamepads->gamepad = realloc(_gamepads->gamepad, ccGamepadCount() * sizeof(ccGamepad));

					GAMEPADS_DATA->xInputConnected[i] = ccGamepadCount() - 1;
					currentGamepad = &_gamepads->gamepad[GAMEPADS_DATA->xInputConnected[i]];
					currentGamepad->data = malloc(sizeof(ccGamepad_win));

					// Create connect event
					event.gamepadEvent.id = ccGamepadCount() - 1;
					event.gamepadEvent.type = CC_GAMEPAD_CONNECT;
					_ccEventStackPush(event);

					// Fill new gamepad data
					GAMEPAD_DATA->inputType = CC_GAMEPAD_INPUT_XINPUT;
					GAMEPAD_DATA->xinputIndex = i;

					currentGamepad->name = "Xbox gamepad";
					currentGamepad->plugged = true;
					currentGamepad->outputAmount = GAMEPAD_XINPUT_HAPTICAMOUNT;
					currentGamepad->buttonAmount = GAMEPAD_XINPUT_BUTTONCOUNT;
					currentGamepad->axisAmount = GAMEPAD_XINPUT_AXISCOUNT;
					currentGamepad->button = calloc(GAMEPAD_XINPUT_BUTTONCOUNT, sizeof(bool));
					currentGamepad->axis = malloc(GAMEPAD_XINPUT_AXISCOUNT * sizeof(int));
					currentGamepad->output = calloc(GAMEPAD_XINPUT_HAPTICAMOUNT, sizeof(int));
				}
				else if(ccGamepadGet(GAMEPADS_DATA->xInputConnected[i])->plugged == false) {
					// Reconnect a previously disconnected gamepad
					ccGamepadGet(GAMEPADS_DATA->xInputConnected[i])->plugged = true;

					event.gamepadEvent.id = GAMEPADS_DATA->xInputConnected[i];
					event.gamepadEvent.type = CC_GAMEPAD_CONNECT;
					_ccEventStackPush(event);
				}
			}

			currentGamepad = &_gamepads->gamepad[GAMEPADS_DATA->xInputConnected[i]];

			// Update gamepad
			event.gamepadEvent.id = GAMEPADS_DATA->xInputConnected[i];

			for(j = 0; j < GAMEPAD_XINPUT_BUTTONCOUNT; j++) {
				if(currentGamepad->button[j] == false && state.Gamepad.wButtons & _gamepadXinputButtons[j]) {
					currentGamepad->button[j] = true;

					event.gamepadEvent.type = CC_GAMEPAD_BUTTON_DOWN;
					event.gamepadEvent.buttonId = j;
					_ccEventStackPush(event);
				}
				else if(currentGamepad->button[j] == true && !(state.Gamepad.wButtons & _gamepadXinputButtons[j])) {
					currentGamepad->button[j] = false;

					event.gamepadEvent.type = CC_GAMEPAD_BUTTON_UP;
					event.gamepadEvent.buttonId = j;
					_ccEventStackPush(event);
				}
			}

			for(j = 0; j < GAMEPAD_XINPUT_AXISCOUNT; j++) {
				switch(j) {
				case GAMEPAD_XINPUT_ILEFTTRIGGER:
					axisValue = (state.Gamepad.bLeftTrigger + CHAR_MIN) * GAMEPAD_XINPUT_TRIGGER_FACTOR;
					break;
				case GAMEPAD_XINPUT_IRIGHTTRIGGER:
					axisValue = (state.Gamepad.bRightTrigger + CHAR_MIN) * GAMEPAD_XINPUT_TRIGGER_FACTOR;
					break;
				case GAMEPAD_XINPUT_ITHUMBLX:
					axisValue = state.Gamepad.sThumbLX;
					break;
				case GAMEPAD_XINPUT_ITHUMBLY:
					axisValue = state.Gamepad.sThumbLY;
					break;
				case GAMEPAD_XINPUT_ITHUMBRX:
					axisValue = state.Gamepad.sThumbRX;
					break;
				case GAMEPAD_XINPUT_ITHUMBRY:
					axisValue = state.Gamepad.sThumbRY;
					break;
				}

				if(axisValue > GAMEPAD_AXIS_MAX) {
					axisValue = GAMEPAD_AXIS_MAX;
				}
				else if(axisValue < GAMEPAD_AXIS_MIN) {
					axisValue = GAMEPAD_AXIS_MIN;
				}

				if(currentGamepad->axis[j] != axisValue) {
					currentGamepad->axis[j] = axisValue;

					event.gamepadEvent.type = CC_GAMEPAD_AXIS_MOVE;
					event.gamepadEvent.axisId = j;
					_ccEventStackPush(event);
				}
			}
		}
		else {
			if(GAMEPADS_DATA->xInputConnected[i] != -1 && ccGamepadGet(GAMEPADS_DATA->xInputConnected[i])->plugged != false) {
				ccGamepadGet(GAMEPADS_DATA->xInputConnected[i])->plugged = false;
				event.gamepadEvent.id = GAMEPADS_DATA->xInputConnected[i];
				event.gamepadEvent.type = CC_GAMEPAD_DISCONNECT;
				_ccEventStackPush(event);
			}
		}
	}
}

void _generateGamepadEvents(RAWINPUT *raw)
{
	ccEvent event;
	ULONG usageLength;
	int i, j;

	int newInt;
	
	// Find the current gamepad or create it
	ccGamepad *currentGamepad = NULL;

	event.type = CC_EVENT_GAMEPAD;

	for(i = 0; i < ccGamepadCount(); i++) {
		if(((ccGamepad_win*)(_gamepads->gamepad[i].data))->inputType == CC_GAMEPAD_INPUT_RAW && ((ccGamepad_win*)(_gamepads->gamepad[i].data))->raw->handle == raw->header.hDevice) {
			currentGamepad = &_gamepads->gamepad[i];
			event.gamepadEvent.id = i;
			break;
		}
	}
	if(currentGamepad == NULL) {
		USHORT capsLength;

		_gamepads->amount++;
		_gamepads->gamepad = realloc(_gamepads->gamepad, ccGamepadCount() * sizeof(ccGamepad));

		currentGamepad = &_gamepads->gamepad[ccGamepadCount() - 1];
		event.gamepadEvent.id = ccGamepadCount() - 1;

		// Initialize current gamepad
		currentGamepad->data = malloc(sizeof(ccGamepad_win));
		((ccGamepad_win*)currentGamepad->data)->raw = malloc(sizeof(ccGamepad_win_raw));

		GAMEPAD_DATA->inputType = CC_GAMEPAD_INPUT_RAW;

		GetRawInputDeviceInfo(raw->header.hDevice, RIDI_PREPARSEDDATA, NULL, &GAMEPAD_DATA->raw->preparsedDataSize);
		GAMEPAD_DATA->raw->preparsedData = malloc(GAMEPAD_DATA->raw->preparsedDataSize);
		GetRawInputDeviceInfo(raw->header.hDevice, RIDI_PREPARSEDDATA, GAMEPAD_DATA->raw->preparsedData, &GAMEPAD_DATA->raw->preparsedDataSize);

		currentGamepad->name = "Gamepad"; //TODO: can I fetch this?
		currentGamepad->plugged = true; //TODO: use this properly
		GAMEPAD_DATA->raw->handle = raw->header.hDevice;
		HidP_GetCaps(GAMEPAD_DATA->raw->preparsedData, &GAMEPAD_DATA->raw->caps);

		GAMEPAD_DATA->raw->buttonCaps = malloc(sizeof(HIDP_BUTTON_CAPS)* GAMEPAD_DATA->raw->caps.NumberInputButtonCaps);
		GAMEPAD_DATA->raw->valueCaps = malloc(sizeof(HIDP_VALUE_CAPS)* GAMEPAD_DATA->raw->caps.NumberInputValueCaps);
		GAMEPAD_DATA->raw->outputValueCaps = malloc(sizeof(HIDP_VALUE_CAPS)* GAMEPAD_DATA->raw->caps.NumberOutputValueCaps);

		capsLength = GAMEPAD_DATA->raw->caps.NumberInputButtonCaps;
		HidP_GetButtonCaps(HidP_Input, GAMEPAD_DATA->raw->buttonCaps, &capsLength, GAMEPAD_DATA->raw->preparsedData);
		capsLength = GAMEPAD_DATA->raw->caps.NumberInputValueCaps;
		HidP_GetValueCaps(HidP_Input, GAMEPAD_DATA->raw->valueCaps, &capsLength, GAMEPAD_DATA->raw->preparsedData);
		capsLength = GAMEPAD_DATA->raw->caps.NumberOutputValueCaps;
		HidP_GetValueCaps(HidP_Output, GAMEPAD_DATA->raw->outputValueCaps, &capsLength, GAMEPAD_DATA->raw->preparsedData);

		currentGamepad->buttonAmount = GAMEPAD_DATA->raw->buttonCaps->Range.UsageMax - GAMEPAD_DATA->raw->buttonCaps->Range.UsageMin + 1;
		currentGamepad->axisAmount = GAMEPAD_DATA->raw->caps.NumberInputValueCaps;
		currentGamepad->outputAmount = GAMEPAD_DATA->raw->caps.NumberOutputValueCaps;

		printf("output count %d\n", currentGamepad->outputAmount);
		
		currentGamepad->button = calloc(currentGamepad->buttonAmount, sizeof(bool));
		currentGamepad->axis = malloc(sizeof(int)* currentGamepad->axisAmount);
		currentGamepad->output = calloc(currentGamepad->outputAmount, sizeof(int));

		GAMEPAD_DATA->raw->axisFactor = malloc(sizeof(double)* currentGamepad->axisAmount);
		GAMEPAD_DATA->raw->axisNegativeComponent = malloc(sizeof(int)* currentGamepad->axisAmount);
		
		for(i = 0; i < currentGamepad->axisAmount; i++) {
			GAMEPAD_DATA->raw->axisFactor[i] = (double)(GAMEPAD_AXIS_MAX - GAMEPAD_AXIS_MIN) / (GAMEPAD_DATA->raw->valueCaps[i].PhysicalMax - GAMEPAD_DATA->raw->valueCaps[i].PhysicalMin);
			GAMEPAD_DATA->raw->axisNegativeComponent[i] = ((GAMEPAD_DATA->raw->valueCaps[i].PhysicalMax - GAMEPAD_DATA->raw->valueCaps[i].PhysicalMin) >> 1) - GAMEPAD_DATA->raw->valueCaps[i].PhysicalMin;
		}

		event.gamepadEvent.type = CC_GAMEPAD_CONNECT;
		_ccEventStackPush(event);
	}
	
	// Get buttons
	usageLength = currentGamepad->buttonAmount;
	HidP_GetUsages(HidP_Input, GAMEPAD_DATA->raw->buttonCaps->UsagePage, 0, GAMEPADS_DATA->usage, &usageLength, GAMEPAD_DATA->raw->preparsedData, raw->data.hid.bRawData, raw->data.hid.dwSizeHid);
	
	for(i = 0; i < (int)usageLength; i++)
	{
		int index = GAMEPADS_DATA->usage[i] - GAMEPAD_DATA->raw->buttonCaps->Range.UsageMin;
		if(currentGamepad->button[index] == false) {
			currentGamepad->button[index] = true;
			
			event.gamepadEvent.type = CC_GAMEPAD_BUTTON_DOWN;
			event.gamepadEvent.buttonId = index;
			_ccEventStackPush(event);
		}
	}
	for(i = 0; i < currentGamepad->buttonAmount; i++) {
		if(currentGamepad->button[i] == true) {
			for(j = 0; j < (int)usageLength; j++) {
				if(currentGamepad->button[GAMEPADS_DATA->usage[j] - GAMEPAD_DATA->raw->buttonCaps->Range.UsageMin] == true) {
					goto pressed;
				}
			}
			currentGamepad->button[i] = false;

			event.gamepadEvent.type = CC_GAMEPAD_BUTTON_UP;
			event.gamepadEvent.buttonId = i;
			_ccEventStackPush(event);
		}
	pressed:;
	}

	// Try to send output
	/*
	if(HidP_SetUsageValue(HidP_Output, GAMEPAD_DATA->raw->outputValueCaps[0].UsagePage, 0, GAMEPAD_DATA->raw->valueCaps[0].NotRange.Usage,
		SHRT_MAX, GAMEPAD_DATA->raw->preparsedData, raw->data.hid.bRawData, raw->data.hid.dwSizeHid) == HIDP_STATUS_SUCCESS) {
		abort();
	}
	*/

	for(i = 0; i < currentGamepad->axisAmount; i++)
	{
		HidP_GetUsageValue(HidP_Input, GAMEPAD_DATA->raw->valueCaps[i].UsagePage, 0, GAMEPAD_DATA->raw->valueCaps[i].NotRange.Usage, &newInt, GAMEPAD_DATA->raw->preparsedData, raw->data.hid.bRawData, raw->data.hid.dwSizeHid);
		if(GAMEPAD_DATA->raw->valueCaps[i].Range.UsageMin != HID_USAGE_GENERIC_HATSWITCH) {
			newInt = (int)((newInt - GAMEPAD_DATA->raw->axisNegativeComponent[i]) * GAMEPAD_DATA->raw->axisFactor[i]);
			if(newInt < GAMEPAD_AXIS_MIN) {
				newInt = GAMEPAD_AXIS_MIN;
			}
			else if(newInt > GAMEPAD_AXIS_MAX) {
				newInt = GAMEPAD_AXIS_MAX;
			}
		}

		if(newInt != currentGamepad->axis[i]) {
			currentGamepad->axis[i] = newInt;
			event.gamepadEvent.type = CC_GAMEPAD_AXIS_MOVE;
			event.gamepadEvent.axisId = i;
			_ccEventStackPush(event);
		}
	}
}