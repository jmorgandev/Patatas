#ifndef PTS_WINDRAW
#define PTS_WINDRAW
#pragma once
#include <Windows.h>
#include "stdafx.h"

const uint CUSTOM_SCALE = 5;

extern double framesPerSecond;
extern double renderFreq;
extern uint drawScale;
extern uint scaleIndex;
extern uint winStyle;
extern HWND winHandle;
extern HMENU winMenu;

bool Draw_Init();
void Draw_SetColors(uint bg, uint fg);
void Draw_Exit();
void Draw_PaintFrame(HDC deviceContext);
void Draw_SetScale(uint menuIndex);
void Draw_RefreshWindow();


#endif