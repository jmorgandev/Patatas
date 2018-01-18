#include "dialog_register.h"
#include "resource.h"
#include "chip8.h"
#include <stdio.h>

static HWND handle = NULL;
static HWND list = NULL;

static void PopulateList() {
	char buffer[7];
	for (uint row = 0; row < 0x10; row++) {
		sprintf_s(buffer, "%01X: %03X", row,c8.stack[row]);
		SendMessage(list, LB_ADDSTRING, -1, (LPARAM)buffer);
	}
}

static void RefreshRegisterView() {
	char buffer[4];
	for (uint i = 0; i < 0x10; i++) {
		sprintf_s(buffer, "%02X", c8.V[i]);
		SetDlgItemText(handle, IDC_REG_V0 + i, buffer);
	}
	sprintf_s(buffer, "%02X", c8.DT);
	SetDlgItemText(handle, IDC_REG_DT, buffer);

	sprintf_s(buffer, "%02X", c8.ST);
	SetDlgItemText(handle, IDC_REG_ST, buffer);

	sprintf_s(buffer, "%02X", c8.I);
	SetDlgItemText(handle, IDC_REG_I, buffer);

	sprintf_s(buffer, "%02X", c8.PC);
	SetDlgItemText(handle, IDC_REG_PC, buffer);

	SendMessage(list, WM_SETREDRAW, false, 0);
	SendMessage(list, LB_RESETCONTENT, NULL, NULL);
	SendMessage(list, LB_INITSTORAGE, 0x15, 6);
	PopulateList();
	SendMessage(list, WM_SETREDRAW, true, 0);
}

INT_PTR CALLBACK DialogProc_Register(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		handle = hwnd;
		list = GetDlgItem(handle, IDC_STACK_LIST);
		RefreshRegisterView();
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_REFRESH) {
			RefreshRegisterView();
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}