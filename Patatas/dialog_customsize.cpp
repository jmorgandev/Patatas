#include "dialog_customsize.h"
#include "resource.h"
#include "windraw.h"
#include "chip8.h"

static uint customValue = 0;

INT_PTR CALLBACK DialogProc_CustomSize(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		SetDlgItemText(hwnd, IDC_PIXELSIZE, "");
		SendDlgItemMessage(hwnd, IDC_EDITSCALE, EM_LIMITTEXT, 2, NULL);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (customValue < 4 || customValue > 32) {
				MessageBox(hwnd, "Enter a value between 4 and 32", "Invalid Scale", MB_ICONWARNING | MB_OK);
			}
			else {
				drawScale = customValue;
				EndDialog(hwnd, IDOK);
			}
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDOK);
			break;
		case IDC_EDITSCALE:
			if (HIWORD(wParam) == EN_CHANGE) {
				char buffer[128];
				GetDlgItemText(hwnd, IDC_EDITSCALE, buffer, 128);
				customValue = atoi(buffer);
				wsprintf(buffer, "(%i x %i)", VID_WIDTH * customValue, VID_HEIGHT * customValue);
				SetDlgItemText(hwnd, IDC_PIXELSIZE, buffer);
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