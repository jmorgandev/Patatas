#include "input.h"
#include "stdafx.h"
#include <memory>
#include <Windows.h>
#include "console.h"

uint keyBinds[CHIP8_KEY_COUNT];

static void LoadDefaultKeyBinds() {
	keyBinds[CK_0] = VK_X;
	keyBinds[CK_1] = VK_1;
	keyBinds[CK_2] = VK_2;
	keyBinds[CK_3] = VK_3;
	keyBinds[CK_C] = VK_4;
	keyBinds[CK_4] = VK_Q;
	keyBinds[CK_5] = VK_W;
	keyBinds[CK_6] = VK_E;
	keyBinds[CK_D] = VK_R;
	keyBinds[CK_7] = VK_A;
	keyBinds[CK_8] = VK_S;
	keyBinds[CK_9] = VK_D;
	keyBinds[CK_E] = VK_F;
	keyBinds[CK_A] = VK_Z;
	keyBinds[CK_B] = VK_C;
	keyBinds[CK_F] = VK_V;
}

static uint GetChip8Key(uint key) {
	for (uint i = 0; i < CHIP8_KEY_COUNT; ++i)
		if (keyBinds[i] == key) return i;
	return NULL_KEY;
}

bool Input_Init() {
	LoadDefaultKeyBinds();
	std::memset(c8.keys, 0, CHIP8_KEY_COUNT);

	return true;
}

void Input_KeyEvent(uint type, uint key) {
	uint chip8Key = GetChip8Key(key);
	if (chip8Key != NULL)
		c8.keys[chip8Key] = (type == WM_KEYDOWN) ? true : false;

	if (key == VK_K)
		Con_Print("SEND HELP FATHER");
}