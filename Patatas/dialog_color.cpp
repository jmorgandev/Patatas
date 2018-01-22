#include "dialog_color.h"
#include "windraw.h"
#include "resource.h"
#include <CommCtrl.h>

static COLOR newColorFG = { 0 };
static COLOR newColorBG = { 0 };

static HWND previewFG = NULL;
static HWND previewBG = NULL;
static HBRUSH brushFG = NULL;
static HBRUSH brushBG = NULL;

static INT_PTR PreviewPaint(HWND hwnd, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps = { 0 };
	HDC dc = BeginPaint(hwnd, &ps);

	RECT rect;
	GetClientRect(hwnd, &rect);

	HBRUSH brush = NULL;
	if (GetDlgCtrlID(hwnd) == IDC_FGCOLOR)
		brush = brushFG;
	else brush = brushBG;

	FillRect(dc, &rect, brush);
	FrameRect(dc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

	EndPaint(hwnd, &ps);
	return 0;
}

static INT_PTR CALLBACK PreviewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubClass, DWORD_PTR dwRefData) {
	if (msg == WM_PAINT) {
		PreviewPaint(hwnd, wParam, lParam);
		return 0;
	}
	else return DefSubclassProc(hwnd, msg, wParam, lParam);
}

INT_PTR CALLBACK DialogProc_Color(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG: {
			newColorFG = Draw_GetColorFG();
			newColorBG = Draw_GetColorBG();

			previewFG = GetDlgItem(hwnd, IDC_FGCOLOR);
			previewBG = GetDlgItem(hwnd, IDC_BGCOLOR);

			SetWindowSubclass(previewFG, PreviewProc, 0, 0);
			SetWindowSubclass(previewBG, PreviewProc, 0, 0);

			char buffer[4];
			wsprintf(buffer, "%i", newColorFG.r);
			SetDlgItemText(hwnd, IDC_RED, buffer);
			SendDlgItemMessage(hwnd, IDC_RED, EM_LIMITTEXT, 3, NULL);
			wsprintf(buffer, "%i", newColorFG.g);
			SetDlgItemText(hwnd, IDC_GREEN, buffer);
			SendDlgItemMessage(hwnd, IDC_GREEN, EM_LIMITTEXT, 3, NULL);
			wsprintf(buffer, "%i", newColorFG.b);
			SetDlgItemText(hwnd, IDC_BLUE, buffer);
			SendDlgItemMessage(hwnd, IDC_BLUE, EM_LIMITTEXT, 3, NULL);

			wsprintf(buffer, "%i", newColorBG.r);
			SetDlgItemText(hwnd, IDC_RED2, buffer);
			SendDlgItemMessage(hwnd, IDC_RED2, EM_LIMITTEXT, 3, NULL);
			wsprintf(buffer, "%i", newColorBG.g);
			SetDlgItemText(hwnd, IDC_GREEN2, buffer);
			SendDlgItemMessage(hwnd, IDC_GREEN2, EM_LIMITTEXT, 3, NULL);
			wsprintf(buffer, "%i", newColorBG.b);
			SetDlgItemText(hwnd, IDC_BLUE2, buffer);
			SendDlgItemMessage(hwnd, IDC_BLUE2, EM_LIMITTEXT, 3, NULL);

			brushFG = CreateSolidBrush(RGB(newColorFG.r, newColorFG.g, newColorFG.b));
			if (brushFG == NULL) {
				MessageBox(hwnd, "Could not create FG Brush", "Error", MB_OK | MB_ICONERROR);
				EndDialog(hwnd, FALSE);
			}
			brushBG = CreateSolidBrush(RGB(newColorBG.r, newColorBG.g, newColorBG.b));
			if (brushBG == NULL) {
				MessageBox(hwnd, "Could not create BG Brush", "Error", MB_OK | MB_ICONERROR);
				EndDialog(hwnd, FALSE);
			}
		} break;
	case WM_COMMAND: {
			uint controlID = LOWORD(wParam);
			if (HIWORD(wParam) == EN_CHANGE) {
				char buffer[4];
				GetDlgItemText(hwnd, controlID, buffer, 4);
				uint value = atoi(buffer);
				if (buffer[0] == NULL) {
					MessageBeep(MB_ICONEXCLAMATION);
					SetDlgItemText(hwnd, controlID, "0");
				}
				else if (value > 255) {
					MessageBeep(MB_ICONEXCLAMATION);
					SetDlgItemText(hwnd, controlID, "255");
				}
				if (controlID == IDC_RED || controlID == IDC_GREEN || controlID == IDC_BLUE) {
					GetDlgItemText(hwnd, IDC_RED, buffer, 4);
					if (buffer[0] == NULL) break;
					BYTE r = atoi(buffer);
					GetDlgItemText(hwnd, IDC_GREEN, buffer, 4);
					if (buffer[0] == NULL) break;
					BYTE g = atoi(buffer);
					GetDlgItemText(hwnd, IDC_BLUE, buffer, 4);
					if (buffer[0] == NULL) break;
					BYTE b = atoi(buffer);

					DeleteObject(brushFG);
					brushFG = CreateSolidBrush(RGB(r, g, b));
					if (brushFG == NULL) {
						MessageBox(hwnd, "Could not create FG Brush", "Error", MB_OK | MB_ICONERROR);
						EndDialog(hwnd, FALSE);
					}
					newColorFG = { r, g, b };
					SendMessage(GetDlgItem(hwnd, IDC_FGCOLOR), WM_PAINT, 0, 0);
					RECT rect;
					GetClientRect(previewFG, &rect);
					InvalidateRect(previewFG, &rect, TRUE);
				}
				else {
					GetDlgItemText(hwnd, IDC_RED2, buffer, 4);
					if (buffer[0] == NULL) break;
					BYTE r = atoi(buffer);
					GetDlgItemText(hwnd, IDC_GREEN2, buffer, 4);
					if (buffer[0] == NULL) break;
					BYTE g = atoi(buffer);
					GetDlgItemText(hwnd, IDC_BLUE2, buffer, 4);
					if (buffer[0] == NULL) break;
					BYTE b = atoi(buffer);

					DeleteObject(brushBG);
					brushBG = CreateSolidBrush(RGB(r, g, b));
					if (brushFG == NULL) {
						MessageBox(hwnd, "Could not create FG Brush", "Error", MB_OK | MB_ICONERROR);
						EndDialog(hwnd, FALSE);
					}
					newColorBG = { r, g, b };
					RECT rect;
					GetClientRect(previewBG, &rect);
					InvalidateRect(previewBG, &rect, TRUE);
				}
			}
			else if (controlID == IDOK) {
				Draw_SetColors(newColorBG, newColorFG);
				EndDialog(hwnd, IDOK);
			}
			else if (controlID == IDCANCEL) {
				EndDialog(hwnd, IDOK);
			}
		}
		break;
	case WM_DESTROY:
		if (brushBG != NULL) DeleteObject(brushBG);
		if (brushFG != NULL) DeleteObject(brushFG);
		break;
	case WM_CLOSE:
		EndDialog(hwnd, IDOK);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}