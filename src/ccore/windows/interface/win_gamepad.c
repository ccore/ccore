#include "win_gamepad.h"

#if defined CC_USE_ALL || defined CC_USE_GAMEPAD

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

ccReturn ccGamepadOutputSet(ccGamepad *gamepad, int hapticIndex, int force)
{	
	ccAssert(gamepad != NULL);
	
	if(hapticIndex >= gamepad->outputAmount) {
		ccErrorPush(CC_ERROR_GAMEPAD_HAPTICNONE);
		return CC_FAIL;
	}
	
	if(((ccGamepad_win*)gamepad->data)->inputType == CC_GAMEPAD_INPUT_XINPUT) {
		XINPUT_VIBRATION vibration;
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

		gamepad->output[hapticIndex] = force;

		vibration.wLeftMotorSpeed = (WORD)(ccGamepad_win*)gamepad->output[0];
		vibration.wRightMotorSpeed = (WORD)(ccGamepad_win*)gamepad->output[1];

		if(XInputSetState(((ccGamepad_win*)gamepad->data)->xinputIndex, &vibration) != ERROR_SUCCESS) {
			ccErrorPush(CC_ERROR_GAMEPAD_HAPTICNONE);
			return CC_FAIL;
		}
	}

	return CC_SUCCESS;
}

ccReturn ccGamepadInitialize(void)
{
	int i;
	ccAssert(_ccWindow != NULL);

	ccMalloc(_ccGamepads, sizeof(ccGamepads));
	ccMalloc(_ccGamepads->data, sizeof(ccGamepads_win));

	_ccGamepads->amount = 0;
	_ccGamepads->gamepad = NULL;
	
	_CC_WINDOW_DATA->queryXinput = true;

	for(i = 0; i < XUSER_MAX_COUNT; i++) {
		_CC_GAMEPADS_DATA->xInputConnected[i] = -1;
	}

	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_GAMEPAD].usUsagePage = 1;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_GAMEPAD].usUsage = HID_USAGE_GENERIC_JOYSTICK;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_GAMEPAD].dwFlags = 0;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_GAMEPAD].hwndTarget = _CC_WINDOW_DATA->winHandle;

	if(RegisterRawInputDevices(&_CC_WINDOW_DATA->rid[_CC_RAWINPUT_GAMEPAD], _CC_RAWINPUT_GAMEPADCOUNT, sizeof(RAWINPUTDEVICE)) == TRUE) {
		return CC_SUCCESS;
	}
	else{
		ccErrorPush(CC_ERROR_GAMEPAD_NONE);
		return CC_FAIL;
	}
}

ccReturn ccGamepadFree(void)
{
	ccAssert(_ccGamepads != NULL);
	
	_CC_WINDOW_DATA->queryXinput = false;

	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_GAMEPAD].dwFlags = RIDEV_REMOVE;
	_CC_WINDOW_DATA->rid[_CC_RAWINPUT_GAMEPAD].hwndTarget = NULL;
	
	RegisterRawInputDevices(&_CC_WINDOW_DATA->rid[_CC_RAWINPUT_GAMEPAD], _CC_RAWINPUT_GAMEPADCOUNT, sizeof(RAWINPUTDEVICE));

	if(ccGamepadGetAmount() != 0) {
		int i;
		for(i = 0; i < ccGamepadGetAmount(); i++) {
			if(((ccGamepad_win*)_ccGamepads->gamepad[i].data)->inputType == CC_GAMEPAD_INPUT_RAW) {
				free(((ccGamepad_win*)_ccGamepads->gamepad[i].data)->raw->buttonCaps);
				free(((ccGamepad_win*)_ccGamepads->gamepad[i].data)->raw->valueCaps);
				free(((ccGamepad_win*)_ccGamepads->gamepad[i].data)->raw->axisFactor);
				free(((ccGamepad_win*)_ccGamepads->gamepad[i].data)->raw->axisNegativeComponent);
				free(((ccGamepad_win*)_ccGamepads->gamepad[i].data)->raw->preparsedData);
				free(((ccGamepad_win*)_ccGamepads->gamepad[i].data)->raw);
			}
			free(_ccGamepads->gamepad[i].data);
			free(_ccGamepads->gamepad[i].button);
			free(_ccGamepads->gamepad[i].axis);
			if(_ccGamepads->gamepad[i].outputAmount != 0)
				free(_ccGamepads->gamepad[i].output);
		}
		free(_ccGamepads->gamepad);
	}

	free(_ccGamepads->data);
	free(_ccGamepads);

	_ccGamepads = NULL;

	return CC_SUCCESS;
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
			if(_CC_GAMEPADS_DATA->xInputConnected[i] == -1 || ccGamepadGet(_CC_GAMEPADS_DATA->xInputConnected[i])->plugged == false) {
				if(_CC_GAMEPADS_DATA->xInputConnected[i] == -1) {
					// Allocate memory for newly connected gamepad
					_ccGamepads->amount++;
					_ccGamepads->gamepad = realloc(_ccGamepads->gamepad, ccGamepadGetAmount() * sizeof(ccGamepad));

					_CC_GAMEPADS_DATA->xInputConnected[i] = ccGamepadGetAmount() - 1;
					currentGamepad = &_ccGamepads->gamepad[_CC_GAMEPADS_DATA->xInputConnected[i]];
					currentGamepad->data = malloc(sizeof(ccGamepad_win));

					// Create connect event
					event.gamepadEvent.id = ccGamepadGetAmount() - 1;
					event.gamepadEvent.type = CC_GAMEPAD_CONNECT;
					_ccEventStackPush(event);

					// Fill new gamepad data
					_CC_GAMEPAD_DATA->inputType = CC_GAMEPAD_INPUT_XINPUT;
					_CC_GAMEPAD_DATA->xinputIndex = i;

					currentGamepad->name = "Xbox gamepad";
					currentGamepad->plugged = true;
					currentGamepad->outputAmount = _CC_GAMEPAD_XINPUT_HAPTICAMOUNT;
					currentGamepad->buttonAmount = _CC_GAMEPAD_XINPUT_BUTTONCOUNT;
					currentGamepad->axisAmount = _CC_GAMEPAD_XINPUT_AXISCOUNT;
					currentGamepad->button = calloc(_CC_GAMEPAD_XINPUT_BUTTONCOUNT, sizeof(bool));
					currentGamepad->axis = malloc(_CC_GAMEPAD_XINPUT_AXISCOUNT * sizeof(int));
					currentGamepad->output = calloc(_CC_GAMEPAD_XINPUT_HAPTICAMOUNT, sizeof(int));
				}
				else if(ccGamepadGet(_CC_GAMEPADS_DATA->xInputConnected[i])->plugged == false) {
					// Reconnect a previously disconnected gamepad
					ccGamepadGet(_CC_GAMEPADS_DATA->xInputConnected[i])->plugged = true;

					event.gamepadEvent.id = _CC_GAMEPADS_DATA->xInputConnected[i];
					event.gamepadEvent.type = CC_GAMEPAD_CONNECT;
					_ccEventStackPush(event);
				}
			}

			currentGamepad = &_ccGamepads->gamepad[_CC_GAMEPADS_DATA->xInputConnected[i]];

			// Update gamepad
			event.gamepadEvent.id = _CC_GAMEPADS_DATA->xInputConnected[i];

			for(j = 0; j < _CC_GAMEPAD_XINPUT_BUTTONCOUNT; j++) {
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

			for(j = 0; j < _CC_GAMEPAD_XINPUT_AXISCOUNT; j++) {
				switch(j) {
				case _CC_GAMEPAD_XINPUT_ILEFTTRIGGER:
					axisValue = (state.Gamepad.bLeftTrigger + CHAR_MIN) * _CC_GAMEPAD_XINPUT_TRIGGER_FACTOR;
					break;
				case _CC_GAMEPAD_XINPUT_IRIGHTTRIGGER:
					axisValue = (state.Gamepad.bRightTrigger + CHAR_MIN) * _CC_GAMEPAD_XINPUT_TRIGGER_FACTOR;
					break;
				case _CC_GAMEPAD_XINPUT_ITHUMBLX:
					axisValue = state.Gamepad.sThumbLX;
					break;
				case _CC_GAMEPAD_XINPUT_ITHUMBLY:
					axisValue = state.Gamepad.sThumbLY;
					break;
				case _CC_GAMEPAD_XINPUT_ITHUMBRX:
					axisValue = state.Gamepad.sThumbRX;
					break;
				case _CC_GAMEPAD_XINPUT_ITHUMBRY:
					axisValue = state.Gamepad.sThumbRY;
					break;
				}

				if(axisValue > CC_GAMEPAD_AXIS_MAX) {
					axisValue = CC_GAMEPAD_AXIS_MAX;
				}
				else if(axisValue < CC_GAMEPAD_AXIS_MIN) {
					axisValue = CC_GAMEPAD_AXIS_MIN;
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
			if(_CC_GAMEPADS_DATA->xInputConnected[i] != -1 && ccGamepadGet(_CC_GAMEPADS_DATA->xInputConnected[i])->plugged != false) {
				ccGamepadGet(_CC_GAMEPADS_DATA->xInputConnected[i])->plugged = false;
				event.gamepadEvent.id = _CC_GAMEPADS_DATA->xInputConnected[i];
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

	for(i = 0; i < ccGamepadGetAmount(); i++) {
		if(((ccGamepad_win*)(_ccGamepads->gamepad[i].data))->inputType == CC_GAMEPAD_INPUT_RAW && ((ccGamepad_win*)(_ccGamepads->gamepad[i].data))->raw->handle == raw->header.hDevice) {
			currentGamepad = &_ccGamepads->gamepad[i];
			event.gamepadEvent.id = i;
			break;
		}
	}
	if(currentGamepad == NULL) {
		USHORT capsLength;

		_ccGamepads->amount++;
		_ccGamepads->gamepad = realloc(_ccGamepads->gamepad, ccGamepadGetAmount() * sizeof(ccGamepad));

		currentGamepad = &_ccGamepads->gamepad[ccGamepadGetAmount() - 1];
		event.gamepadEvent.id = ccGamepadGetAmount() - 1;

		// Initialize current gamepad
		currentGamepad->data = malloc(sizeof(ccGamepad_win));
		((ccGamepad_win*)currentGamepad->data)->raw = malloc(sizeof(ccGamepad_win_raw));

		_CC_GAMEPAD_DATA->inputType = CC_GAMEPAD_INPUT_RAW;

		GetRawInputDeviceInfo(raw->header.hDevice, RIDI_PREPARSEDDATA, NULL, &_CC_GAMEPAD_DATA->raw->preparsedDataSize);
		_CC_GAMEPAD_DATA->raw->preparsedData = malloc(_CC_GAMEPAD_DATA->raw->preparsedDataSize);
		GetRawInputDeviceInfo(raw->header.hDevice, RIDI_PREPARSEDDATA, _CC_GAMEPAD_DATA->raw->preparsedData, &_CC_GAMEPAD_DATA->raw->preparsedDataSize);
		
		currentGamepad->name = "Gamepad"; //TODO: can I fetch this?
		currentGamepad->plugged = true; //TODO: use this properly
		_CC_GAMEPAD_DATA->raw->handle = raw->header.hDevice;
		HidP_GetCaps(_CC_GAMEPAD_DATA->raw->preparsedData, &_CC_GAMEPAD_DATA->raw->caps);

		currentGamepad->outputAmount = _CC_GAMEPAD_DATA->raw->caps.NumberOutputValueCaps;
		currentGamepad->output = calloc(currentGamepad->outputAmount, sizeof(int));

		_CC_GAMEPAD_DATA->raw->buttonCaps = malloc(sizeof(HIDP_BUTTON_CAPS)* _CC_GAMEPAD_DATA->raw->caps.NumberInputButtonCaps);
		_CC_GAMEPAD_DATA->raw->valueCaps = malloc(sizeof(HIDP_VALUE_CAPS)* _CC_GAMEPAD_DATA->raw->caps.NumberInputValueCaps);

		capsLength = _CC_GAMEPAD_DATA->raw->caps.NumberInputButtonCaps;
		HidP_GetButtonCaps(HidP_Input, _CC_GAMEPAD_DATA->raw->buttonCaps, &capsLength, _CC_GAMEPAD_DATA->raw->preparsedData);
		capsLength = _CC_GAMEPAD_DATA->raw->caps.NumberInputValueCaps;
		HidP_GetValueCaps(HidP_Input, _CC_GAMEPAD_DATA->raw->valueCaps, &capsLength, _CC_GAMEPAD_DATA->raw->preparsedData);

		currentGamepad->buttonAmount = _CC_GAMEPAD_DATA->raw->buttonCaps->Range.UsageMax - _CC_GAMEPAD_DATA->raw->buttonCaps->Range.UsageMin + 1;
		if(currentGamepad->buttonAmount > _CC_GAMEPAD_MAXBUTTONS) currentGamepad->buttonAmount = _CC_GAMEPAD_MAXBUTTONS;
		currentGamepad->axisAmount = _CC_GAMEPAD_DATA->raw->caps.NumberInputValueCaps;
		
		currentGamepad->button = calloc(currentGamepad->buttonAmount, sizeof(bool));
		currentGamepad->axis = malloc(sizeof(int)* currentGamepad->axisAmount);

		_CC_GAMEPAD_DATA->raw->axisFactor = malloc(sizeof(double)* currentGamepad->axisAmount);
		_CC_GAMEPAD_DATA->raw->axisNegativeComponent = malloc(sizeof(int)* currentGamepad->axisAmount);
		
		for(i = 0; i < currentGamepad->axisAmount; i++) {
			_CC_GAMEPAD_DATA->raw->axisFactor[i] = (double)(CC_GAMEPAD_AXIS_MAX - CC_GAMEPAD_AXIS_MIN) / (_CC_GAMEPAD_DATA->raw->valueCaps[i].PhysicalMax - _CC_GAMEPAD_DATA->raw->valueCaps[i].PhysicalMin);
			_CC_GAMEPAD_DATA->raw->axisNegativeComponent[i] = ((_CC_GAMEPAD_DATA->raw->valueCaps[i].PhysicalMax - _CC_GAMEPAD_DATA->raw->valueCaps[i].PhysicalMin) >> 1) - _CC_GAMEPAD_DATA->raw->valueCaps[i].PhysicalMin;
		}

		event.gamepadEvent.type = CC_GAMEPAD_CONNECT;
		_ccEventStackPush(event);
	}
	
	// Get buttons
	usageLength = currentGamepad->buttonAmount;
	HidP_GetUsages(HidP_Input, _CC_GAMEPAD_DATA->raw->buttonCaps->UsagePage, 0, _CC_GAMEPADS_DATA->usage, &usageLength, _CC_GAMEPAD_DATA->raw->preparsedData, raw->data.hid.bRawData, raw->data.hid.dwSizeHid);
	
	for(i = 0; i < (int)usageLength; i++)
	{
		int index = _CC_GAMEPADS_DATA->usage[i] - _CC_GAMEPAD_DATA->raw->buttonCaps->Range.UsageMin;

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
				if(currentGamepad->button[_CC_GAMEPADS_DATA->usage[j] - _CC_GAMEPAD_DATA->raw->buttonCaps->Range.UsageMin] == true) {
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
	
	for(i = 0; i < currentGamepad->axisAmount; i++)
	{
		HidP_GetUsageValue(HidP_Input, _CC_GAMEPAD_DATA->raw->valueCaps[i].UsagePage, 0, _CC_GAMEPAD_DATA->raw->valueCaps[i].NotRange.Usage, &newInt, _CC_GAMEPAD_DATA->raw->preparsedData, raw->data.hid.bRawData, raw->data.hid.dwSizeHid);
		if(_CC_GAMEPAD_DATA->raw->valueCaps[i].Range.UsageMin != HID_USAGE_GENERIC_HATSWITCH) {
			newInt = (int)((newInt - _CC_GAMEPAD_DATA->raw->axisNegativeComponent[i]) * _CC_GAMEPAD_DATA->raw->axisFactor[i]);
			if(newInt < CC_GAMEPAD_AXIS_MIN) {
				newInt = CC_GAMEPAD_AXIS_MIN;
			}
			else if(newInt > CC_GAMEPAD_AXIS_MAX) {
				newInt = CC_GAMEPAD_AXIS_MAX;
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

#endif CC_USE_GAMEPAD