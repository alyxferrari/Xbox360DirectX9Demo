#include "stdafx.h"
#include "XboxController.h"
unsigned short getControllerButtons() {
	XINPUT_STATE state;
	XInputGetState(0, &state);
	return state.Gamepad.wButtons;
}
ThumbStickState* getLeftThumbStick() {
	XINPUT_STATE state;
	XInputGetState(0, &state);
	ThumbStickState stick = {
		state.Gamepad.sThumbLX,
		state.Gamepad.sThumbLY,
	};
	return &stick;
}
ThumbStickState* getRightThumbStick() {
	XINPUT_STATE state;
	XInputGetState(0, &state);
	ThumbStickState stick = {
		state.Gamepad.sThumbRX,
		state.Gamepad.sThumbRY,
	};
	return &stick;
}