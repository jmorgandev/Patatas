#include "stdafx.h"
#include <Windows.h>

void OpenURL(const char* url) {
	ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

void NotifyError() {
	DWORD code = GetLastError();
	WCHAR buffer[512];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL, GetLastError(), 0, (LPSTR)buffer, 512, NULL);
	MessageBox(NULL, (LPSTR)buffer, "Error", MB_OK | MB_ICONEXCLAMATION);
}

bool ValidHexChar(const char c) {
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
}

bool ValidHexInput(const char* str) {
	while (*str != '\0') {
		if (!ValidHexChar(*(str++)))
			return false;
	}
	return true;
}

uint StrToHex(const char* str) {
	uint result = 0;
	while (*str != '\0') {
		result = (result << 4) + ((*str > '9') ? (*str - 'A') + 0xA : *str - '0');
		str++;
	}
	return result;
}

void PostCommand(HWND hwnd, word cmd, LPARAM lParam) {
	PostMessage(hwnd, WM_COMMAND, cmd, lParam);
}

uint ClampMin(uint value, uint min) {
	if (value < min) return min;
	else return value;
}

uint ClampPos(int value) {
	if (value < 0) return 0;
	else return value;
}

void SwitchEndian(uint size, byte* value) {
	byte result[32];
	for (uint i = 0; i < size; i++)
		result[i] = ((byte*)value)[size - i - 1];
	for (uint i = 0; i < size; i++)
		((byte*)value)[i] = result[i];
}