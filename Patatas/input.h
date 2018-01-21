#ifndef PTS_INPUT
#define PTS_INPUT
#pragma once
#include "stdafx.h"

bool Input_Init();
void Input_KeyEvent(uint type, uint key);

enum systemKeys {
	PTS_PAUSE,
	PTS_RESUME,
	PTS_STEP,
	PTS_RESET,

};

#endif