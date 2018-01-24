#include "console.h"
#include "windraw.h"
#include <stdio.h>

static HWND handle = NULL;
static HWND textHandle = NULL;
static HFONT fnt = NULL;
static uint lines = 0;

static LRESULT CALLBACK ConProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Con_Enabled() {
	return handle != NULL;
}

bool Con_Init() {
	WNDCLASSEX conclass = { 0 };
	conclass.cbSize = sizeof(WNDCLASSEX);
	conclass.lpfnWndProc = ConProc;
	conclass.hInstance = GetModuleHandle(NULL);
	conclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	conclass.hbrBackground = NULL;
	conclass.lpszClassName = "PTS_CONSOLECLASS";
	conclass.lpszMenuName = NULL;
	conclass.hIcon = NULL;
	conclass.hIconSm = NULL;

	if (!RegisterClassEx(&conclass)) {
		NotifyError();
		return false;
	}
	return true;
}

void Con_Start() {
	if (handle != NULL) return;
	handle = CreateWindowEx(
		WS_EX_CLIENTEDGE | WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME,
		"PTS_CONSOLECLASS",
		"Opcode Logs",
		WS_VISIBLE | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		450, 250, winHandle, NULL, GetModuleHandle(NULL), NULL
	);
	if (handle == NULL) {
		NotifyError();
	}
}

#define FORMAT_BUFFER_SIZE 4096
#define CONSOLE_MAX_LINES 100
void Con_Print(const char* fmt, ...) {
	if (handle == NULL) return;
	if (lines == CONSOLE_MAX_LINES) {
		SendMessage(textHandle, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		lines = 0;
	}
	else {
		uint index = GetWindowTextLength(textHandle);
		SendMessage(textHandle, EM_SETSEL, (WPARAM)index, (LPARAM)index);
	}
	va_list args;
	char buffer[FORMAT_BUFFER_SIZE];

	va_start(args, fmt);
	uint length = vsprintf_s(buffer, fmt, args);
	va_end(args);

	buffer[length++] = '\r';
	buffer[length++] = '\n';
	buffer[length] = '\0';

	SendMessage(textHandle, EM_REPLACESEL, 0, (LPARAM)buffer);
	lines++;
}

static LRESULT CALLBACK ConProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
			uint borderx = GetSystemMetrics(SM_CXSIZEFRAME);
			uint bordery = GetSystemMetrics(SM_CYSIZEFRAME);
			uint scrollpx = GetSystemMetrics(SM_CXVSCROLL);
			uint scrollpy = GetSystemMetrics(SM_CYVSCROLL);
			uint sysmenupy = GetSystemMetrics(SM_CYCAPTION);
			textHandle = CreateWindowEx(
				WS_EX_CLIENTEDGE, "EDIT", "",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
				0, 0, 450 - scrollpx - borderx, 250 - sysmenupy - scrollpy,
				hwnd, NULL, GetModuleHandle(NULL), NULL
			);
			if (textHandle == NULL) {
				NotifyError();
				DestroyWindow(hwnd);
				return FALSE;
			}
			HDC temp = GetDC(hwnd);
			uint height = -MulDiv(10, GetDeviceCaps(temp, LOGPIXELSY), 72);
			ReleaseDC(hwnd, temp);
			fnt = CreateFont(height, 0, 0, 0, FW_REGULAR, 0, 0, 0, 0, 0, 0, 0, 0, "Consolas");
			SendMessage(textHandle, WM_SETFONT, (WPARAM)fnt, TRUE);
		} break;
	case WM_DESTROY:
		DeleteObject(fnt);
		handle = NULL;
		textHandle = NULL;
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}