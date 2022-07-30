#include "stdafx.h"
#include "XboxController.h"
unsigned short getControllerState() {
	XINPUT_STATE state;
	XInputGetState(0, &state);
	return state.Gamepad.wButtons;
}