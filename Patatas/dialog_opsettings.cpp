#include "dialog_opsettings.h"
#include "chip8.h"
#include "resource.h"

static c8_Settings newSettings = { 0 };

INT_PTR CALLBACK DialogProc_OpcodeSettings(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		newSettings = opcodeSettings;
		SendDlgItemMessage(hwnd, IDC_CHECK1, BM_SETCHECK, newSettings.shiftInPlace, 0);
		SendDlgItemMessage(hwnd, IDC_CHECK2, BM_SETCHECK, newSettings.storeLoadAdvance, 0);
		SendDlgItemMessage(hwnd, IDC_CHECK3, BM_SETCHECK, newSettings.emptyDrawSync, 0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			newSettings.shiftInPlace = SendDlgItemMessage(hwnd, IDC_CHECK1, BM_GETCHECK, 0, 0);
			newSettings.storeLoadAdvance = SendDlgItemMessage(hwnd, IDC_CHECK2, BM_GETCHECK, 0, 0);
			newSettings.emptyDrawSync = SendDlgItemMessage(hwnd, IDC_CHECK3, BM_GETCHECK, 0, 0);
			opcodeSettings = newSettings;
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDOK);
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