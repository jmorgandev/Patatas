#ifndef PTS_FILE_DIALOG
#define PTS_FILE_DIALOG
#include <Windows.h>
#include "stdafx.h"

bool FilePromptOpen(HWND hwnd, char* buffer, uint size, const char* filter, const char* title = "Open");

#endif