#include "console.h"
#include <stdio.h>

static HWND handle = NULL;
static HWND textHandle = NULL;

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
		WS_EX_CLIENTEDGE | WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		"PTS_CONSOLECLASS",
		"Opcode Logs",
		WS_VISIBLE | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		400, 200, NULL, NULL, GetModuleHandle(NULL), NULL
	);
	if (handle == NULL) {
		NotifyError();
	}
}

#define FORMAT_BUFFER_SIZE 4096
void Con_Print(const char* fmt, ...) {
	if (handle == NULL) return;
	va_list args;
	char buffer[FORMAT_BUFFER_SIZE];

	va_start(args, fmt);
	uint length = vsprintf_s(buffer, fmt, args);
	va_end(args);

	buffer[length++] = '\r';
	buffer[length++] = '\n';
	buffer[length] = '\0';

	uint index = GetWindowTextLength(textHandle);
	SendMessage(textHandle, EM_SETSEL, (WPARAM)index, (LPARAM)index);
	SendMessage(textHandle, EM_REPLACESEL, 0, (LPARAM)buffer);
}

static LRESULT CALLBACK ConProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
			textHandle = CreateWindowEx(
				WS_EX_CLIENTEDGE, "EDIT", "",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
				0, 0, 400, 200, hwnd, NULL, GetModuleHandle(NULL), NULL
			);
			if (textHandle == NULL) {
				NotifyError();
				DestroyWindow(hwnd);
				return FALSE;
			}
		} break;
	case WM_DESTROY:
		handle = NULL;
		textHandle = NULL;
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}