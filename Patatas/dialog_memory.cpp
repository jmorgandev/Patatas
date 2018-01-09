#include "dialog_memory.h"
#include "stdafx.h"
#include <stdio.h>
#include "chip8.h"
#include "resource.h"

HFONT fnt = NULL;

static void HandleCommand(HWND hwnd, word cmd) {
	switch (cmd) {
	case IDC_MEMSEEK_ADDR: {
			char buffer[4];
			GetDlgItemText(hwnd, IDC_MEM_ADDR, buffer, 4);
			if (!ValidHexInput(buffer)) {
				MessageBox(hwnd, "Invalid Hex Input", "Warning", MB_OK);
				return;
			}
			uint target = StrToHex(buffer) / 0x10;
			PostMessage(GetDlgItem(hwnd, IDC_MEMORY_LIST), WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, target), 0);
		} break;
	case IDC_MEMSEEK_PC: {
			
		} break;
	}
}

INT_PTR CALLBACK DialogProc_Memory(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG: {
			HDC temp = GetDC(hwnd);
			uint height = -MulDiv(10, GetDeviceCaps(temp, LOGPIXELSY), 72);
			ReleaseDC(hwnd, temp);
			fnt = CreateFont(height, 0, 0, 0, FW_REGULAR, 0, 0, 0, 0, 0, 0, 0, 0, "Consolas");
			SendDlgItemMessage(hwnd, IDC_MEMORY_LIST, WM_SETFONT, (WPARAM)fnt, TRUE);
			SendDlgItemMessage(hwnd, IDC_MEMORY_AXIS, WM_SETFONT, (WPARAM)fnt, TRUE);

			SendDlgItemMessage(hwnd, IDC_MEM_ADDR, EM_LIMITTEXT, 3, NULL);

			char buffer[70];
			for (uint row = 0; row < (MEMORY_SIZE / 0x10); ++row) {
				sprintf_s(buffer, 70, "%03X:", row * 0x10);
				for (uint column = 0; column < 0x10; ++column) {
					uint len = strlen(buffer);
					uint data = c8.memory[(row * 0x10) + column];
					uint spaces = (column != 0);
					sprintf_s(buffer + len, 70 - len, " %02X", data);
				}
				SendDlgItemMessage(hwnd, IDC_MEMORY_LIST, LB_ADDSTRING, 0, (LPARAM)buffer);
			}
		} break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		DeleteObject(fnt);
		break;
	case WM_COMMAND:
		HandleCommand(hwnd, LOWORD(wParam));
		break;
	default:
		return FALSE;
	}
	return TRUE;
}