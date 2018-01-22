#ifndef PTS_STDAFX
#define PTS_STDAFX
#pragma once
#include <Windows.h>
#include <stdint.h>

#define AS_STRING(a) #a
#define TO_STR(a) AS_STRING(a)

#define PTS_VERSION_MAJOR    0
#define PTS_VERSION_MINOR    4
#define PTS_VERSION_REVISION 0

#define PTS_VERSION TO_STR(PTS_VERSION_MAJOR)"."TO_STR(PTS_VERSION_MINOR)"."TO_STR(PTS_VERSION_REVISION)

typedef uint8_t byte, uint8, u8;
typedef uint16_t uint16, word, u16;
typedef uint32_t uint32, u32;
typedef unsigned int uint;

#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

void OpenURL(const char* url);
void NotifyError();

#define HREF_GITHUB "https://github.com/jmorgandev/Patatas"
#define HREF_WIKI   "https://en.wikipedia.org/wiki/CHIP-8"

bool ValidHexChar(const char c);
bool ValidHexInput(const char* str);

uint StrToHex(const char* str);

void PostCommand(HWND hwnd, word cmd, LPARAM lParam);

uint ClampMin(uint value, uint min);
uint ClampPos(int value);

#define IDC_REDRAW (WM_USER + 1)
#endif

