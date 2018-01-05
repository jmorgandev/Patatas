#include "dialog_about.h"
#include "stdafx.h"
#include <CommCtrl.h>
#include "resource.h"

INT_PTR CALLBACK DialogProc_About(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LPNMHDR pnmh = (LPNMHDR)lParam;
	switch (msg) {
	case WM_INITDIALOG:
		SetDlgItemText(hwnd, IDC_VERSION, "Version " PTS_VERSION);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hwnd, IDOK);
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, IDOK);
		break;
	case WM_NOTIFY:
		if (pnmh->code == NM_CLICK || pnmh->code == NM_RETURN) {
			switch (pnmh->idFrom) {
			case IDC_GITHUB: OpenURL(HREF_GITHUB); break;
			case IDC_WIKI: OpenURL(HREF_WIKI); break;
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}