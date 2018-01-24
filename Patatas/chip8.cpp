#include "chip8.h"
#include "c8font.h"
#include <memory>
#include <fstream>
#include <ctime>
#include "sound.h"
#include "console.h"

#define PROGRAM_START 0x200
#define PROGRAM_SIZE (MEMORY_SIZE - PROGRAM_START)

c8_State c8 = { 0 };
c8_Settings opcodeSettings = { 0 };

static char currentROM[MAX_PATH];

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

	file.seekg(0, std::ios::end);
	uint length = file.tellg();
	if (length > PROGRAM_SIZE) return false;

	file.seekg(0, std::ios::beg);
	file.read((char*)c8.memory + PROGRAM_START, length);
	file.close();
	strcpy_s(currentROM, path);
}

void Chip8_Reload() {
	Chip8_Reset();
	Chip8_LoadProgram(currentROM);
}

void Chip8_SetSpeed(uint cps) {
	cyclesPerSecond = cps;
	cycleFreq = 1000.0 / cyclesPerSecond;
}

void Chip8_Cycle() {
	if (c8.keyBlock || c8.tickBlock) return;
	word opcode = (c8.memory[c8.PC] << 8) | (c8.memory[c8.PC + 1]);
	uint addr = opcode & 0xFFF;
	uint vx = (opcode & 0xF00) >> 8;
	uint vy = (opcode & 0xF0) >> 4;
	uint num8 = (opcode & 0xFF);
	uint num4 = (opcode & 0xF);

	switch (opcode >> 12) {
	case 0x0:
		if (opcode == 0x00E0) {
			Con_Print("%X: Clear display", opcode);
			memset(c8.display, NULL, VID_SIZE);
		}
		else if(opcode == 0x00EE) {
			Con_Print("%X: Return from subroutine", opcode);
			if (c8.SP - 1 < 0) {
				Con_Print("[EXCEPTION] SP < 0");
				PauseEmulation(); return;
			}
			c8.PC = c8.stack[c8.SP--];
		}
		else {
			Con_Print("%X: Undefined opcode");
			PauseEmulation(); return;
		}
		break;
	case 0x1:
		//Jump to address NNN
		Con_Print("%X: Jump to address %X", opcode, addr);
		c8.PC = addr;
		break;
	case 0x2:
		//Execute subroutine starting at address NNN
		Con_Print("%X: Call subroutine at address %X", opcode, addr);
		c8.stack[c8.SP++] = c8.PC;
		c8.PC = addr;
		break;
	case 0x3:
		//Skip the following instruction if the value of VX equals NN
		Con_Print("%X: Skip next opcode if V%X (%X) == %X", opcode, vx, c8.V[vx], num8);
		if (c8.V[vx] == num8) c8.PC += 2;
		break;
	case 0x4:
		//Skip the following instruction if the value of VX is not equal to NN
		Con_Print("%X: Skip next opcode if V%X (%X) != %X", opcode, vx, c8.V[vx], num8);
		if (c8.V[vx] != num8) c8.PC += 2;
		break;
	case 0x5:
		if (opcode & 0xF == 0x0) {
			//Skip the following instruction if the value of VX is equal to the value of VY
			Con_Print("%X: Skip next opcode if V%X (%X) == V%X (%i)",
				opcode, vx, c8.V[vx], vy, c8.V[vy]);
			if (c8.V[vx] == c8.V[vy]) c8.PC += 2;
		}
		else {
			Con_Print("%X: Undefined opcode");
			PauseEmulation(); return;
		}
		break;
	case 0x6:
		//Store the number NN in VX
		Con_Print("%X: V%X = %X", opcode, vx, num8);
		c8.V[vx] = num8;
		break;
	case 0x7:
		//Add the value NN to VX
		Con_Print("%X: V%X += %X", opcode, vx, num8);
		c8.V[vx] += num8;
		break;
	case 0x8:
		switch (opcode & 0xF) {
		case 0x0:
			//Store the value of VY in VX
			Con_Print("%X: V%X = V%X", opcode, vx, vy);
			c8.V[vx] = c8.V[vy];
			break;
		case 0x1:
			//Set VX to VX OR VY
			Con_Print("%X: V%X = V%X OR V%X", opcode, vx, vx, vy);
			c8.V[vx] |= c8.V[vy];
			break;
		case 0x2:
			//Set VX to VX AND VY
			Con_Print("%X: V%X = V%X AND V%X", opcode, vx, vx, vy);
			c8.V[vx] &= c8.V[vy];
			break;
		case 0x3:
			//Set VX to VX XOR VY
			Con_Print("%X: V%X = V%X XOR V%X", opcode, vx, vx, vy);
			c8.V[vx] ^= c8.V[vy];
			break;
		case 0x4:
			//Add the value of VY to VX. Set VF to 1 if a carry occurs,
			//otherwise set VF to 0
			Con_Print("%X: V%X += V%Y, VF = carry", opcode, vy, vx);
			c8.V[0xF] = (c8.V[vy] > (0xFF - c8.V[vx])) ? 1 : 0;
			c8.V[vx] += c8.V[vy];
			break;
		case 0x5:
			//Set VX to the value of VX minus VY. Set VF to 0 if a borrow
			//occurs, otherwise set VF to 1
			Con_Print("%X: V%X -= V%X, VF = !borrow", opcode, vx, vy);
			c8.V[0xF] = (c8.V[vy] > c8.V[vx]) ? 0 : 1;
			c8.V[vx] -= c8.V[vy];
			break;
		case 0x6:
			//Store the value of VY shifted right one bit in VX.
			//Set VF to the least significant bit prior to the shift
			if (opcodeSettings.shiftInPlace) {
				Con_Print("%X: V%X >>= 1, VF = V%X lsb [Shift In Place]", opcode, vx, vx);
				c8.V[0xF] = (c8.V[vx] & 1);
				c8.V[vx] = c8.V[vx] >> 1;
			}
			else {
				Con_Print("%X: V%X = V%X >> 1, VF = V%X lsb", opcode, vx, vy, vy);
				c8.V[0xF] = (c8.V[vy] & 1);
				c8.V[vx] = c8.V[vy] >> 1;
			}
			break;
		case 0x7:
			//Set VX to the value of VY minus VX. Set VF to 0 if a borrow
			//occurs, otherwise set VF to 1
			Con_Print("%X: V%X = V%X - V%X, VF = !borrow", opcode, vx, vy, vx);
			c8.V[0xF] = (c8.V[vx] > c8.V[vy]) ? 0 : 1;
			c8.V[vx] = c8.V[vy] - c8.V[vx];
			break;
		case 0xE:
			//Store the value of VY shifted left one bit in VX.
			//Set VF to the most significant bit prior to the shift
			if (opcodeSettings.shiftInPlace) {
				Con_Print("%X: V%X <<= 1, VF = V%X msb [Shift In Place]", opcode, vx, vx);
				c8.V[0xF] = (c8.V[vx] & 0x80);
				c8.V[vx] = c8.V[vx] << 1;
			}
			else {
				Con_Print("%X: V%X = V%X << 1, VF = V%X msb", opcode, vx, vy, vy);
				c8.V[0xF] = (c8.V[vy] & 0x80);
				c8.V[vx] = c8.V[vy] << 1;
			}
			break;
		default:
			Con_Print("%X: Undefined opcode");
			PauseEmulation(); return;
		}
		break;
	case 0x9:
		if (opcode & 0xF == 0x0) {
			//Skip the following instruction if the value of register VX is
			//not equal to the value of register VY
			Con_Print("%X: Skip next opcode if V%X (%X) != V%X (%X)",
				opcode, vx, c8.V[vx], vy, c8.V[vy]);
			if (c8.V[vx] != c8.V[vy]) c8.PC += 2;
		}
		else {
			Con_Print("%X: Undefined opcode");
			PauseEmulation(); return;
		}
		break;
	case 0xA:
		//Store memory address NNN in I
		Con_Print("%X: Set I to address %X", opcode, addr);
		c8.I = addr;
		break;
	case 0xB:
		//Jump to address NNN + V0
		Con_Print("%X: Jump to address %X + V0 (%X)", opcode, addr, c8.V[0x0]);
		c8.PC = addr + c8.V[0x0];
		break;
	case 0xC:
		//Set VX to a random number with a mask of NN
		Con_Print("%X: V%X = RAND & %X", opcode, vx, num8);
		c8.V[vx] = (rand() % 0xFF) & num8;
		break;
	case 0xD: {
		//Draw sprite at position VX, VY with N bytes of sprite data starting
		//at address stored in I. Set VF to 1 if any existing pixels are overwritten.
		c8.V[0xF] = 0;
		uint xPos = c8.V[vx];
		uint yPos = c8.V[vy];
		uint bytes = num4;

		if (bytes == 0 && opcodeSettings.emptyDrawSync) {
			c8.tickBlock = true;
			Con_Print("%X: Wait for next timer tick [VSYNC]");
			break;
		}
		Con_Print("%X: Draw %X byte sprite at %X X and %X Y, VF = overdraw",
			opcode, num4, vx, vy);

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
			Con_Print("%X: Skip next opcode if Key value V%X (%X) is pressed",
				opcode, vx, c8.V[vx]);
			if (c8.keys[c8.V[vx]]) c8.PC += 2;
		}
		else if (opcode & 0xFF == 0xA1) {
			//Skip the following instruction if the key corresponding to
			//the hex value stored in VX is NOT pressed
			Con_Print("%X: Skip next opcode if Key value V%X (%X) is not pressed",
				opcode, vx, c8.V[vx]);
			if (!c8.keys[c8.V[vx]]) c8.PC += 2;
		}
		else {
			Con_Print("%X: Undefined opcode");
			PauseEmulation(); return;
		}
		break;
	case 0xF:
		switch (opcode & 0xFF) {
		case 0x07:
			//Store the current value of the delay timer in VX
			Con_Print("%X: V%X = DT (%X)", opcode, vx, c8.DT);
			c8.V[vx] = c8.DT;
			break;
		case 0x0A:
			//Wait for a keypress and store the result in VX
			Con_Print("%X: Wait for a keypress and store value in V%X", opcode, vx);
			c8.keyBlock = true;
			break;
		case 0x15:
			//Set the delay timer to the value of VX
			Con_Print("%X: DT = V%X (%X)", opcode, vx, c8.V[vx]);
			c8.DT = c8.V[vx];
			break;
		case 0x18:
			//Set the sound timer to the value of VX
			Con_Print("%X: ST = V%X (%X)", opcode, vx, c8.V[vx]);
			c8.ST = c8.V[vx];
			if (c8.ST != 0) Sound_Play();
			break;
		case 0x1E:
			//Add the value stored in VX to I
			Con_Print("%X: I += V%X (%X)", opcode, vx, c8.V[vx]);
			c8.I += c8.V[vx];
			break;
		case 0x29:
			//Set I to the memory address of the sprite data corresponding
			//to the hex digit stored in register VX
			Con_Print("%X: I = FONT_SPRITE(V%X)", opcode, vx);
			c8.I = c8.V[vx] * CHIP8_FONT_HEIGHT;
			break;
		case 0x33: {
			//Store the binary-coded decimal equivalent of the value stored
			//in register VX at addresses I, I+1, and I+2
			Con_Print("%X: Store BCD of V%X at address I (%X)", opcode, vx, c8.I);
			c8.memory[c8.I] = (c8.V[vx] / 100);
			c8.memory[c8.I + 1] = (c8.V[vx] / 10) % 10;
			c8.memory[c8.I + 2] = (c8.V[vx] % 10);
		}   break;
		case 0x55:
			//Store the values of registers V0 to VX inclusive in memory starting
			//at address I. I is set to I + X + 1 after operation.
			for (uint i = 0; i <= vx; ++i) {
				c8.memory[c8.I + i] = c8.V[i];
			}
			if (!opcodeSettings.fixedMemoryAccess) {
				Con_Print("%X: *I = V0 - V%X, I += %X + 1", opcode, vx, c8.I, vx);
				c8.I = c8.I + vx + 1;
			}
			else
				Con_Print("%X: *I = V0 - V%X [I unmodified]", opcode, vx);
			break;
		case 0x65:
			//Fill registers V0 to VX inclusive with the values stored in memory
			//starting at address I. I is set to I + X + 1 after operation.
			for (uint i = 0; i <= vx; ++i) {
				c8.V[i] = c8.memory[c8.I + i];
			}
			if (!opcodeSettings.fixedMemoryAccess) {
				Con_Print("%X: V0 - V%X = *I, I += %X + 1", opcode, vx, vx);
				c8.I = c8.I + vx + 1;
			}
			else 
				Con_Print("%X: V0 - V%X = *I, [I unmodified]", opcode, vx, vx);
			break;
		default:
			Con_Print("%X: Undefined opcode");
			PauseEmulation(); return;
		}
		break;
	default:
		Con_Print("%X: Undefined opcode");
		PauseEmulation(); return;
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

