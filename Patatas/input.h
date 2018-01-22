#ifndef PTS_INPUT
#define PTS_INPUT
#pragma once
#include "stdafx.h"
#include "chip8.h"

extern uint keyBinds[CHIP8_KEY_COUNT];

bool Input_Init();
void Input_KeyEvent(uint type, uint key);

#endif