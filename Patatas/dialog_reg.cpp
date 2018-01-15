#include "dialog_reg.h"
#include "chip8.h"

static void RefreshRegisterView() {
	
}

INT_PTR CALLBACK DialogProc_Reg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		break;
	default:
		return FALSE;
	}

	return TRUE;
}