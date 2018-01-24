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
enum c8_Reg {
	V0, V1, V2, V3, V4, V5, V6,
	V7, V8, V9, VA, VB, VC, VD,
	VE, VF, DT, ST, I, PC, SP,
	CHIP8_REG_COUNT
};

struct c8_State {
	byte memory [MEMORY_SIZE];
	word stack  [STACK_SIZE];
	byte display[VID_SIZE];
	bool keys   [CHIP8_KEY_COUNT];

	byte V[0x10];
	byte DT, ST;
	word I, PC, SP;

	bool displayUpdate;
	bool tickBlock;
	bool keyBlock;
};
extern c8_State c8;

struct c8_Settings {
	bool shiftInPlace;
	bool fixedMemoryAccess;
	bool emptyDrawSync;
};
extern c8_Settings opcodeSettings;

extern uint cyclesPerSecond;
extern double cycleFreq;
extern const double tickFreq;

bool Chip8_Init();
bool Chip8_LoadProgram(const char* path);
void Chip8_Reset();
void Chip8_Reload();
void Chip8_Cycle();
void Chip8_Tick();
void Chip8_SetSpeed(uint cps);
void Chip8_TestProgram();

extern void PauseEmulation();
#endif