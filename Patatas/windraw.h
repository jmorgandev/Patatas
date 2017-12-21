#ifndef PTS_WINDRAW
#define PTS_WINDRAW
#pragma once
#include <Windows.h>
#include "stdafx.h"

extern double framesPerSecond;
extern double renderFreq;
extern uint drawScale;

bool Draw_Init();
void Draw_SetColors(uint bg, uint fg);
void Draw_Exit();
void Draw_PaintFrame(HDC deviceContext);


#endif