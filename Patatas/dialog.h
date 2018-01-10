#ifndef PTS_DIALOG
#define PTS_DIALOG
#include <Windows.h>

enum ModelessDialogs {
	DLG_MEMORY,
	DLG_REG,
	DLG_COUNT
};
extern HWND dialogs[DLG_COUNT];

#endif