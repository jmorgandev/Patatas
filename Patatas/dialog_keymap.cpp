#include "dialog_keymap.h"
#include "resource.h"
#include "input.h"
#include "chip8.h"
#include <CommCtrl.h>

static uint newKeys[CHIP8_KEY_COUNT];

static void GetKeyString(uint key, char* buffer, uint size) {
	uint scanCode = MapVirtualKey(key, MAPVK_VK_TO_VSC);
	switch (key) {
	case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
	case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
	case VK_INSERT: case VK_DELETE: case VK_DIVIDE: case VK_NUMLOCK:
		scanCode |= 0x100; //extended bit
		break;
	}
	if (!GetKeyNameText(scanCode << 16, buffer, size)) {
		wsprintf(buffer, "Error");
	}
}

static bool ValidKey(uint vk) {
	uint c = MapVirtualKey(vk, MAPVK_VK_TO_CHAR);
	if (c >= 32 && c <= 127) return true;
	if (vk >= VK_LEFT && vk <= VK_DOWN) return true;
	return false;
}

static int FindKey(uint vk) {
	for (uint i = 0; i < CHIP8_KEY_COUNT; i++) {
		if (newKeys[i] == vk) return i;
	}
	return -1;
}

static int FindDuplicateKey(uint srcIndex) {
	uint key = newKeys[srcIndex];
	for (uint i = 0; i < CHIP8_KEY_COUNT; i++) {
		if (i == srcIndex) continue;
		if (newKeys[i] == key) return i;
	}
	return -1;
}

static bool CheckDuplicateKeys(HWND hwnd) {
	for (uint i = 0; i < CHIP8_KEY_COUNT; i++) {
		uint loc = FindDuplicateKey(i);
		if (loc != -1) {
			char keyName[10];
			GetKeyString(newKeys[i], keyName, 10);
			char buffer[128];
			wsprintf(buffer, "CK_%X and CK_%X cannot both be \"%s\"", i, loc, keyName);
			MessageBox(hwnd, buffer, "Duplicate Key", MB_OK | MB_ICONWARNING);
			return true;
		}
	}
	return false;
}

static LRESULT CALLBACK ButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubClass, DWORD_PTR dwRefData) {
	switch (msg) {
	case WM_GETDLGCODE:
		return DLGC_BUTTON | DLGC_WANTCHARS | DLGC_WANTARROWS;
	case WM_KEYDOWN:
		if (ValidKey(wParam)) {
			uint keyID = GetDlgCtrlID(hwnd) - IDC_KEY0;
			newKeys[keyID] = wParam;
			char keyName[10];
			GetKeyString(wParam, keyName, 10);
			SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)keyName);
		}
		break;
	default:
		return DefSubclassProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

INT_PTR CALLBACK DialogProc_Keymap(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		for (uint i = 0; i < CHIP8_KEY_COUNT; i++) {
			newKeys[i] = keyBinds[i];
			char buffer[10];
			GetKeyString(keyBinds[i], buffer, 10);
			SetDlgItemText(hwnd, IDC_KEY0 + i, buffer);
			SetWindowSubclass(GetDlgItem(hwnd, IDC_KEY0 + i), ButtonProc, 0, 0);
		}
		break;
	case WM_COMMAND: {
			uint ControlID = LOWORD(wParam);
			if (ControlID == IDOK && !CheckDuplicateKeys(hwnd)) {
				for (uint i = 0; i < CHIP8_KEY_COUNT; i++)
					keyBinds[i] = newKeys[i];
				EndDialog(hwnd, IDOK);
			}
			else if (ControlID == IDCANCEL) {
				EndDialog(hwnd, IDOK);
			}
		} break;
	default:
		return FALSE;
	}
	return TRUE;
}