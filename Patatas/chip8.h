#ifndef PTS_CHIP8
#define PTS_CHIP8
#pragma once
#include "stdafx.h"

#define VID_WIDTH 64
#define VID_HEIGHT 32
#define VID_SIZE (VID_WIDTH * VID_HEIGHT) / 8
#define VID_BYTES_PER_ROW VID_WIDTH / 8

#define MEMORY_SIZE 0x1000
#define STACK_SIZE 0x10

enum c8_Key {
	CK_0, CK_1, CK_2, CK_3,
	CK_4, CK_5, CK_6, CK_7,
	CK_8, CK_9, CK_A, CK_B,
	CK_C, CK_D, CK_E, CK_F,
	CHIP8_KEY_COUNT,
	NULL_KEY
};

struct c8_State {
	byte memory [MEMORY_SIZE];
	word stack  [STACK_SIZE];
	byte display[VID_SIZE];
	bool keys   [CHIP8_KEY_COUNT];

	byte V[0x10];
	word I, PC, SP;
	byte ST, DT;

	bool displayUpdate;
	bool keyWait;
};
extern c8_State c8;

extern uint cyclesPerSecond;
extern double cycleFreq;

bool Chip8_Init();
bool Chip8_LoadProgram(const char* path);
void Chip8_Reset();
void Chip8_Cycle();

void Chip8_TestProgram();
#endif