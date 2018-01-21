#include "dialog_customspeed.h"
#include "resource.h"
#include "chip8.h"

static uint customValue = 0;

INT_PTR CALLBACK DialogProc_CustomSpeed(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd, IDC_EDITSPEED, EM_LIMITTEXT, 5, NULL);
		char buffer[128];
		wsprintf(buffer, "%i", cyclesPerSecond);
		SetDlgItemText(hwnd, IDC_EDITSPEED, buffer);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (customValue < 100 || customValue > 10000) {
				MessageBox(hwnd, "Enter a value between 100 and 10,000", "Invalid Scale", MB_ICONWARNING | MB_OK);
			}
			else {
				Chip8_SetSpeed(customValue);
				EndDialog(hwnd, IDOK);
			}
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDOK);
			break;
		case IDC_EDITSPEED:
			if (HIWORD(wParam) == EN_CHANGE) {
				char buffer[128];
				GetDlgItemText(hwnd, IDC_EDITSPEED, buffer, 128);
				customValue = atoi(buffer);
			}
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, IDOK);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}