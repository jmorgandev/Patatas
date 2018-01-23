#include "sound.h"
#include "stdafx.h"
#include <Windows.h>
#include <fstream>

static char* soundBuffer = NULL;

bool Sound_Init() {
	std::ifstream file("res/tone_110hz.wav", std::ios::binary);
	if (!file.is_open()) return false;
	file.seekg(0, std::ios::end);
	uint size = file.tellg();
	soundBuffer = new char[size];
	file.seekg(0, std::ios::beg);
	file.read(soundBuffer, size);
	file.close();
	return true;
}

void Sound_Play() {
	PlaySound(soundBuffer, NULL, SND_MEMORY | SND_ASYNC | SND_LOOP);
}
void Sound_Stop() {
	PlaySound(NULL, NULL, 0);
}

void Sound_Exit() {
	if (soundBuffer != NULL) delete[] soundBuffer;
}