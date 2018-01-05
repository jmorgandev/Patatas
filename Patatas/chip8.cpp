#include "chip8.h"
#include "c8font.h"
#include <memory>
#include <fstream>
#include <ctime>

#define PROGRAM_START 0x200
#define PROGRAM_SIZE (MEMORY_SIZE - PROGRAM_START)

#define STACK_SIZE 16

byte c8_Memory[MEMORY_SIZE];
byte c8_Display[VID_SIZE];
word stack[STACK_SIZE];
bool keys[CHIP8_KEY_COUNT];

byte V[16];
word I, PC, SP;
byte ST, DT;

const char* loadedProgram = nullptr;

uint cyclesPerSecond = 500;
double cycleFreq = 1000.0 / cyclesPerSecond;

bool displayUpdate = false;
bool keyBlock = false;

bool Chip8_Init() {
	Chip8_Reset();

	memcpy(c8_Memory, CHIP8_FONT, CHIP8_FONT_SIZE);

	srand(time(NULL));

	return true;
}

void Chip8_Reset() {
	memset(c8_Memory, NULL, MEMORY_SIZE);
	memset(c8_Display, NULL, VID_SIZE);
	memset(stack, NULL, STACK_SIZE);
	memset(V, NULL, 16);

	PC = PROGRAM_START;
	ST = 0;
	DT = 0;
	SP = 0;
	I = 0;
	displayUpdate = true;
	keyBlock = false;
}

bool Chip8_LoadProgram(const char* path) {
	Chip8_Reset();

	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) return false;

	file.seekg(std::ios::end);
	uint length = file.tellg();
	if (length > PROGRAM_SIZE) return false;

	file.seekg(std::ios::beg);
	file.read((char*)c8_Memory + PROGRAM_START, length);
	file.close();
}

void Chip8_Cycle() {
	if (keyBlock) return;
	word opcode = (c8_Memory[PC] << 8) | (c8_Memory[PC + 1]);
	
	switch (opcode >> 12) {
	case 0x0:
		if (opcode == 0x00E0) {
			//Clear the screen
			memset(c8_Display, NULL, VID_SIZE);
		}
		else if(opcode == 0x00EE) {
			//Return from a subroutine
			PC = stack[SP--];
			if (SP < 0) {
				//exception
			}
		}
		else {
			//exception
		}
		break;
	case 0x1:
		//Jump to address NNN
		PC = opcode & 0xFFF;
		break;
	case 0x2:
		//Execute subroutine starting at address NNN
		stack[SP++] = PC;
		PC = opcode & 0xFFF;
		break;
	case 0x3:
		//Skip the following instruction if the value of VX equals NN
		if (V[(opcode & 0xF00) >> 8] == opcode & 0xFF) PC += 2;
		break;
	case 0x4:
		//Skip the following instruction if the value of VX is not equal to NN
		if (V[(opcode & 0xF00) >> 8] != opcode & 0xFF) PC += 2;
		break;
	case 0x5:
		if (opcode & 0xF == 0x0) {
			//Skip the following instruction if the value of VX is equal
			//to the value of VY
			if (V[(opcode & 0xF00) >> 8] == V[(opcode & 0xF0) >> 4]) PC += 2;
		}
		else {
			//exception
		}
		break;
	case 0x6:
		//Store the number NN in VX
		V[(opcode & 0xF00) >> 8] = opcode & 0xFF;
		break;
	case 0x7:
		//Add the value NN to VX
		V[(opcode & 0xF00) >> 8] += opcode & 0xFF;
		break;
	case 0x8:
		switch (opcode & 0xF) {
		case 0x0:
			//Store the value of VY in VX
			V[(opcode & 0xF00) >> 8] = V[(opcode & 0xF0) >> 4];
			break;
		case 0x1:
			//Set VX to VX OR VY
			V[(opcode & 0xF00) >> 8] |= V[(opcode & 0xF0) >> 4];
			break;
		case 0x2:
			//Set VX to VX AND VY
			V[(opcode & 0xF00) >> 8] &= V[(opcode & 0xF0) >> 4];
			break;
		case 0x3:
			//Set VX to VX XOR VY
			V[(opcode & 0xF00) >> 8] ^= V[(opcode & 0xF0) >> 4];
			break;
		case 0x4:
			//Add the value of VY to VX. Set VF to 1 if a carry occurs,
			//otherwise set VF to 0
			V[0xF] = (V[(opcode & 0xF0) >> 4] > (0xFF - V[(opcode & 0xF00) >> 8])) ? 1 : 0;
			V[(opcode & 0xF00) >> 8] += V[(opcode & 0xF0) >> 4];
			break;
		case 0x5:
			//Set VX to the value of VX minus VY. Set VF to 0 if a borrow
			//occurs, otherwise set VF to 1
			V[0xF] = (V[(opcode & 0xF0) >> 4] > V[(opcode & 0xF00) >> 8]) ? 0 : 1;
			V[(opcode & 0xF00) >> 8] -= V[(opcode & 0xF0) >> 4];
			break;
		case 0x6:
			//Store the value of VY shifted right one bit in VX.
			//Set VF to the least significant bit prior to the shift
			V[0xF] = (V[(opcode & 0xF0) >> 4] & 1);
			V[(opcode & 0xF00) >> 8] = V[(opcode & 0xF0) >> 4] >> 1;
			break;
		case 0x7:
			//Set VX to the value of VY minus VX. Set VF to 0 if a borrow
			//occurs, otherwise set VF to 1
			V[0xF] = (V[(opcode & 0xF00) >> 8] > V[(opcode & 0xF0) >> 4]) ? 0 : 1;
			V[(opcode & 0xF00) >> 8] = V[(opcode & 0xF0) >> 4] - V[(opcode & 0xF00) >> 8];
			break;
		case 0xE:
			//Store the value of VY shifted left one bit in VX.
			//Set VF to the most significant bit prior to the shift
			V[0xF] = (V[(opcode & 0xF0) >> 4] & 128);
			V[(opcode & 0xF00) >> 8] = V[(opcode & 0xF0) >> 4] << 1;
			break;
		default:
			//exception
			break;
		}
		break;
	case 0x9:
		if (opcode & 0xF == 0x0) {
			//Skip the following instruction if the value of register VX is
			//not equal to the value of register VY
			if (V[(opcode & 0xF00) >> 8] != V[(opcode & 0xF0) >> 4]) PC += 2;
		}
		else {
			//exception
		}
		break;
	case 0xA:
		//Store memory address NNN in I
		I = opcode & 0xFFF;
		break;
	case 0xB:
		//Jump to address NNN + V0
		PC = (opcode & 0xFFF) + V[0];
		break;
	case 0xC:
		//Set VX to a random number with a mask of NN
		V[(opcode & 0xF00) >> 8] = (rand() % 0xFF) & (opcode & 0xFF);
		break;
	case 0xD: {
		//Draw sprite at position VX, VY with N bytes of sprite data starting
		//at address stored in I. Set VF to 1 if any existing pixels are overwritten.
		V[0xF] = 0;
		uint xPos = V[(opcode & 0xF00) >> 8];
		uint yPos = V[(opcode & 0xF0) >> 4];
		uint bytes = opcode & 0xF;

		if (bytes == 0) break; //Or throw exception...

		for (uint row = 0; row < bytes; row++) {
			uint pixelStrip = c8_Memory[I + row];
			for (uint bit = 0; bit < 8; bit++) {
				if((pixelStrip & (0x80 >> bit))) {
					uint x = xPos + bit;
					uint y = yPos + row;
					uint index = (y * VID_BYTES_PER_ROW) + (x / 8);
					
					if ((c8_Display[index] >> (x % 8)) & 0x1) V[0xF] = 1;
					c8_Display[index] ^= (0x80 >> (x % 8));
				}
			}
		}

		displayUpdate = true;
	}   break;
	case 0xE:
		if (opcode & 0xFF == 0x9E) {
			//Skip the following instruction if the key corresponding to
			//the hex value stored in VX is pressed
			if (keys[V[(opcode & 0xF00) >> 8]]) PC += 2;
		}
		else if (opcode & 0xFF == 0xA1) {
			//Skip the following instruction if the key corresponding to
			//the hex value stored in VX is NOT pressed
			if (!keys[V[(opcode & 0xF00) >> 8]]) PC += 2;
		}
		else {
			//exception
		}
		break;
	case 0xF:
		switch (opcode & 0xFF) {
		case 0x07:
			//Store the current value of the delay timer in VX
			V[(opcode & 0xF00) >> 8] = DT;
			break;
		case 0x0A:
			//Wait for a keypress and store the result in VX
			keyBlock = true;
			break;
		case 0x15:
			//Set the delay timer to the value of VX
			DT = V[(opcode & 0xF00) >> 8];
			break;
		case 0x18:
			//Set the sound timer to the value of VX
			ST = V[(opcode & 0xF00) >> 8];
			break;
		case 0x1E:
			//Add the value stored in VX to I
			I += V[(opcode & 0xF00) >> 8];
			break;
		case 0x29:
			//Set I to the memory address of the sprite data corresponding
			//to the hex digit stored in register VX
			I = V[(opcode & 0xF00) >> 8] * CHIP8_FONT_HEIGHT;
			break;
		case 0x33: {
			//Store the binary-coded decimal equivalent of the value stored
			//in register VX at addresses I, I+1, and I+2
			uint reg = (opcode & 0xF00) >> 8;
			c8_Memory[I] = (V[reg] / 100);
			c8_Memory[I + 1] = (V[reg] / 10) % 10;
			c8_Memory[I + 2] = (V[reg] % 10);
		}   break;
		case 0x55:
			//Store the values of registers V0 to VX inclusive in memory starting
			//at address I. I is set to I + X + 1 after operation.
			for (uint i = 0; i <= (opcode & 0xF00) >> 8; ++i) {
				c8_Memory[I + i] = V[i];
			}
			break;
		case 0x65:
			//Fill registers V0 to VX inclusive with the values stored in memory
			//starting at address I. I is set to I + X + 1 after operation.
			for (uint i = 0; i <= (opcode & 0xF00) >> 8; ++i) {
				V[i] = c8_Memory[I + i];
			}
			break;
		default:
			//exception
			break;
		}
		break;
	default:
		//exception
		break;
	}
	if (!keyBlock) PC += 2;
}

void Chip8_TestProgram() {
	const byte prog[] = {
		0x63, 0x0A,
		0xF3, 0x29,
		0x60, VID_WIDTH - 4,
		0x61, VID_HEIGHT - 5,
		0xD0, 0x15,
		0xFF, 0x0A
	};

	memcpy(c8_Memory + PROGRAM_START, prog, sizeof(prog));
}

