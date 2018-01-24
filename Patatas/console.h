#ifndef PTS_CONSOLE
#define PTS_CONSOLE
#pragma once
#include <Windows.h>
#include <stdarg.h>
#include "stdafx.h"

extern bool Con_Init();
extern void Con_Start();
extern bool Con_Enabled();
extern void Con_Print(const char* fmt, ...);

#endif