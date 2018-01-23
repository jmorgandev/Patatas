#include "chip8.h"
#include "c8font.h"
#include <memory>
#include <fstream>
#include <ctime>
#include "sound.h"

#define PROGRAM_START 0x200
#define PROGRAM_SIZE (MEMORY_SIZE - PROGRAM_START)

c8_State c8 = { 0 };
c8_Settings opcodeSettings = { 0 };

const char* loadedProgram = nullptr;

uint cyclesPerSecond = 500;
const uint ticksPerSecond = 60;
double cycleFreq = 1000.0 / cyclesPerSecond;
const double tickFreq = 1000.0 / ticksPerSecond;

bool Chip8_Init() {
	Chip8_Reset();

	

	srand(time(NULL));

	return true;
}

void Chip8_Reset() {
	memset(c8.memory, NULL, MEMORY_SIZE);
	memcpy(c8.memory, CHIP8_FONT, CHIP8_FONT_SIZE);
	memset(c8.display, NULL, VID_SIZE);
	memset(c8.stack, NULL, STACK_SIZE);
	memset(c8.V, NULL, 16);

	c8.PC = PROGRAM_START;
	c8.ST = 0;
	c8.DT = 0;
	c8.SP = 0;
	c8.I = 0;
	c8.displayUpdate = true;
	c8.tickBlock = false;
	c8.keyBlock = false;
}

bool Chip8_LoadProgram(const char* path) {
	Chip8_Reset();

	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) return false;

	file.seekg(std::ios::end);
	uint length = file.tellg();
	if (length > PROGRAM_SIZE) return false;

	file.seekg(std::ios::beg);
	file.read((char*)c8.memory + PROGRAM_START, length);
	file.close();
}

void Chip8_SetSpeed(uint cps) {
	cyclesPerSecond = cps;
	cycleFreq = 1000.0 / cyclesPerSecond;
}

void Chip8_Cycle() {
	if (c8.keyBlock || c8.tickBlock) return;
	word opcode = (c8.memory[c8.PC] << 8) | (c8.memory[c8.PC + 1]);
	
	switch (opcode >> 12) {
	case 0x0:
		if (opcode == 0x00E0) {
			//Clear the screen
			memset(c8.display, NULL, VID_SIZE);
		}
		else if(opcode == 0x00EE) {
			//Return from a subroutine
			c8.PC = c8.stack[c8.SP--];
			if (c8.SP < 0) {
				//exception
			}
		}
		else {
			//exception
		}
		break;
	case 0x1:
		//Jump to address NNN
		c8.PC = opcode & 0xFFF;
		break;
	case 0x2:
		//Execute subroutine starting at address NNN
		c8.stack[c8.SP++] = c8.PC;
		c8.PC = opcode & 0xFFF;
		break;
	case 0x3:
		//Skip the following instruction if the value of VX equals NN
		if (c8.V[(opcode & 0xF00) >> 8] == opcode & 0xFF) c8.PC += 2;
		break;
	case 0x4:
		//Skip the following instruction if the value of VX is not equal to NN
		if (c8.V[(opcode & 0xF00) >> 8] != opcode & 0xFF) c8.PC += 2;
		break;
	case 0x5:
		if (opcode & 0xF == 0x0) {
			//Skip the following instruction if the value of VX is equal
			//to the value of VY
			if (c8.V[(opcode & 0xF00) >> 8] == c8.V[(opcode & 0xF0) >> 4]) c8.PC += 2;
		}
		else {
			//exception
		}
		break;
	case 0x6:
		//Store the number NN in VX
		c8.V[(opcode & 0xF00) >> 8] = opcode & 0xFF;
		break;
	case 0x7:
		//Add the value NN to VX
		c8.V[(opcode & 0xF00) >> 8] += opcode & 0xFF;
		break;
	case 0x8:
		switch (opcode & 0xF) {
		case 0x0:
			//Store the value of VY in VX
			c8.V[(opcode & 0xF00) >> 8] = c8.V[(opcode & 0xF0) >> 4];
			break;
		case 0x1:
			//Set VX to VX OR VY
			c8.V[(opcode & 0xF00) >> 8] |= c8.V[(opcode & 0xF0) >> 4];
			break;
		case 0x2:
			//Set VX to VX AND VY
			c8.V[(opcode & 0xF00) >> 8] &= c8.V[(opcode & 0xF0) >> 4];
			break;
		case 0x3:
			//Set VX to VX XOR VY
			c8.V[(opcode & 0xF00) >> 8] ^= c8.V[(opcode & 0xF0) >> 4];
			break;
		case 0x4:
			//Add the value of VY to VX. Set VF to 1 if a carry occurs,
			//otherwise set VF to 0
			c8.V[0xF] = (c8.V[(opcode & 0xF0) >> 4] > (0xFF - c8.V[(opcode & 0xF00) >> 8])) ? 1 : 0;
			c8.V[(opcode & 0xF00) >> 8] += c8.V[(opcode & 0xF0) >> 4];
			break;
		case 0x5:
			//Set VX to the value of VX minus VY. Set VF to 0 if a borrow
			//occurs, otherwise set VF to 1
			c8.V[0xF] = (c8.V[(opcode & 0xF0) >> 4] > c8.V[(opcode & 0xF00) >> 8]) ? 0 : 1;
			c8.V[(opcode & 0xF00) >> 8] -= c8.V[(opcode & 0xF0) >> 4];
			break;
		case 0x6:
			//Store the value of VY shifted right one bit in VX.
			//Set VF to the least significant bit prior to the shift
			if (opcodeSettings.shiftInPlace) {
				c8.V[0xF] = (c8.V[(opcode & 0xF00) >> 8] & 1);
				c8.V[(opcode & 0xF00) >> 8] = c8.V[(opcode & 0xF00) >> 8] >> 1;
			}
			else {
				c8.V[0xF] = (c8.V[(opcode & 0xF0) >> 4] & 1);
				c8.V[(opcode & 0xF00) >> 8] = c8.V[(opcode & 0xF0) >> 4] >> 1;
			}
			break;
		case 0x7:
			//Set VX to the value of VY minus VX. Set VF to 0 if a borrow
			//occurs, otherwise set VF to 1
			c8.V[0xF] = (c8.V[(opcode & 0xF00) >> 8] > c8.V[(opcode & 0xF0) >> 4]) ? 0 : 1;
			c8.V[(opcode & 0xF00) >> 8] = c8.V[(opcode & 0xF0) >> 4] - c8.V[(opcode & 0xF00) >> 8];
			break;
		case 0xE:
			//Store the value of VY shifted left one bit in VX.
			//Set VF to the most significant bit prior to the shift
			if (opcodeSettings.shiftInPlace) {
				c8.V[0xF] = (c8.V[(opcode & 0xF00) >> 8] & 0x80);
				c8.V[(opcode & 0xF00) >> 8] = c8.V[(opcode & 0xF00) >> 8] << 1;
			}
			else {
				c8.V[0xF] = (c8.V[(opcode & 0xF0) >> 4] & 0x80);
				c8.V[(opcode & 0xF00) >> 8] = c8.V[(opcode & 0xF0) >> 4] << 1;
			}
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
			if (c8.V[(opcode & 0xF00) >> 8] != c8.V[(opcode & 0xF0) >> 4]) c8.PC += 2;
		}
		else {
			//exception
		}
		break;
	case 0xA:
		//Store memory address NNN in I
		c8.I = opcode & 0xFFF;
		break;
	case 0xB:
		//Jump to address NNN + V0
		c8.PC = (opcode & 0xFFF) + c8.V[0];
		break;
	case 0xC:
		//Set VX to a random number with a mask of NN
		c8.V[(opcode & 0xF00) >> 8] = (rand() % 0xFF) & (opcode & 0xFF);
		break;
	case 0xD: {
		//Draw sprite at position VX, VY with N bytes of sprite data starting
		//at address stored in I. Set VF to 1 if any existing pixels are overwritten.
		c8.V[0xF] = 0;
		uint xPos = c8.V[(opcode & 0xF00) >> 8];
		uint yPos = c8.V[(opcode & 0xF0) >> 4];
		uint bytes = opcode & 0xF;

		if (bytes == 0) {
			if (opcodeSettings.emptyDrawSync) {
				c8.tickBlock = true;
			}
			else {
				break; //Throw exception
			}
		}

		for (uint row = 0; row < bytes; row++) {
			uint pixelStrip = c8.memory[c8.I + row];
			for (uint bit = 0; bit < 8; bit++) {
				if((pixelStrip & (0x80 >> bit))) {
					uint x = xPos + bit;
					uint y = yPos + row;
					uint index = (y * VID_BYTES_PER_ROW) + (x / 8);
					
					if ((c8.display[index] >> (x % 8)) & 0x1) c8.V[0xF] = 1;
					c8.display[index] ^= (0x80 >> (x % 8));
				}
			}
		}

		c8.displayUpdate = true;
	}   break;
	case 0xE:
		if (opcode & 0xFF == 0x9E) {
			//Skip the following instruction if the key corresponding to
			//the hex value stored in VX is pressed
			if (c8.keys[c8.V[(opcode & 0xF00) >> 8]]) c8.PC += 2;
		}
		else if (opcode & 0xFF == 0xA1) {
			//Skip the following instruction if the key corresponding to
			//the hex value stored in VX is NOT pressed
			if (!c8.keys[c8.V[(opcode & 0xF00) >> 8]]) c8.PC += 2;
		}
		else {
			//exception
		}
		break;
	case 0xF:
		switch (opcode & 0xFF) {
		case 0x07:
			//Store the current value of the delay timer in VX
			c8.V[(opcode & 0xF00) >> 8] = c8.DT;
			break;
		case 0x0A:
			//Wait for a keypress and store the result in VX
			c8.keyBlock = true;
			break;
		case 0x15:
			//Set the delay timer to the value of VX
			c8.DT = c8.V[(opcode & 0xF00) >> 8];
			break;
		case 0x18:
			//Set the sound timer to the value of VX
			c8.ST = c8.V[(opcode & 0xF00) >> 8];
			if (c8.ST != 0) Sound_Play();
			break;
		case 0x1E:
			//Add the value stored in VX to I
			c8.I += c8.V[(opcode & 0xF00) >> 8];
			break;
		case 0x29:
			//Set I to the memory address of the sprite data corresponding
			//to the hex digit stored in register VX
			c8.I = c8.V[(opcode & 0xF00) >> 8] * CHIP8_FONT_HEIGHT;
			break;
		case 0x33: {
			//Store the binary-coded decimal equivalent of the value stored
			//in register VX at addresses I, I+1, and I+2
			uint reg = (opcode & 0xF00) >> 8;
			c8.memory[c8.I] = (c8.V[reg] / 100);
			c8.memory[c8.I + 1] = (c8.V[reg] / 10) % 10;
			c8.memory[c8.I + 2] = (c8.V[reg] % 10);
		}   break;
		case 0x55:
			//Store the values of registers V0 to VX inclusive in memory starting
			//at address I. I is set to I + X + 1 after operation.
			for (uint i = 0; i <= (opcode & 0xF00) >> 8; ++i) {
				c8.memory[c8.I + i] = c8.V[i];
			}
			if (!opcodeSettings.fixedMemoryAccess) {
				c8.I = c8.I + ((opcode & 0xF00) >> 8) + 1;
			}
			break;
		case 0x65:
			//Fill registers V0 to VX inclusive with the values stored in memory
			//starting at address I. I is set to I + X + 1 after operation.
			for (uint i = 0; i <= (opcode & 0xF00) >> 8; ++i) {
				c8.V[i] = c8.memory[c8.I + i];
			}
			if (!opcodeSettings.fixedMemoryAccess) {
				c8.I = c8.I + ((opcode & 0xF00) >> 8) + 1;
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
	c8.PC += 2;
}

void Chip8_Tick() {
	c8.DT = ClampPos(c8.DT - 1);
	if (c8.ST - 1 == 0) Sound_Stop();
	c8.ST = ClampPos(c8.ST - 1);
	c8.tickBlock = false;
}

void Chip8_TestProgram() {
	const byte prog[] = {
		0x63, 0x0A,
		0xF3, 0x29,
		0x60, VID_WIDTH - 4,
		0x61, VID_HEIGHT - 5,
		0xD0, 0x15,
		0xA2, 0x00,
		0xFF, 0x0A
	};

	memcpy(c8.memory + PROGRAM_START, prog, sizeof(prog));
}

