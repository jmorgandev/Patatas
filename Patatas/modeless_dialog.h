#ifndef PTS_DIALOG
#define PTS_DIALOG
#include <Windows.h>
#include "dialog_memory.h"
#include "dialog_register.h"

enum ModelessDialogs {
	DLG_MEMORY,
	DLG_REGISTER,
	DLG_COUNT
};
extern HWND modeless_dialogs[DLG_COUNT];

#endif