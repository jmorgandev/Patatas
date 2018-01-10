#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <list>
#include "stdafx.h"
#include "input.h"
#include "chip8.h"
#include "resource.h"
#include "windraw.h"
#include <CommCtrl.h>

#include "dialog.h"
#include "dialog_about.h"
#include "dialog_file.h"
#include "dialog_memory.h"

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
		"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

const char* winClass = "PTS_WINCLASS";
const char* winTitle = "Patatas";

void HandleCommand(HWND, word);

HDC deviceContext = NULL;

uint winStyle = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);

HWND dialogs[DLG_COUNT];

bool running = true;

bool MessageForDialog(MSG* msg) {
	bool handled = false;
	for (HWND hwnd : dialogs) {
		if (IsWindow(hwnd) && IsDialogMessage(hwnd, msg)) handled = true;
	}
	return handled;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE:
		//ON CLOSE BUTTON
		running = false;
		break;
	case WM_DESTROY:
		//CLEANUP
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		Input_KeyEvent(msg, wParam);
		break;
	case WM_COMMAND:
		HandleCommand(hwnd, LOWORD(wParam));
		break;
	case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hwnd, &ps);
			Draw_PaintFrame(dc);
			EndPaint(hwnd, &ps);
		} break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HWND hwnd;
	MSG msg;

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszClassName = winClass;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	uint border = GetSystemMetrics(SM_CXSIZEFRAME);
	RECT wr = { 0, 0, (VID_WIDTH * drawScale) + border, (VID_HEIGHT * drawScale) + border };
	AdjustWindowRect(&wr, winStyle, TRUE);

	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		winClass,
		winTitle,
		winStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		NULL, NULL, hInstance, NULL
	);

	if (hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	deviceContext = GetDC(hwnd);
	if (deviceContext == NULL) {
		MessageBox(NULL, "Could not get device context!", "Error",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	RECT r;
	GetClientRect(hwnd, &r);
	uint w = r.right;
	uint h = r.bottom;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	Chip8_Init();
	Draw_Init();
	Input_Init();

	double lastTime = (double)GetTickCount();
	double accumulator = 0;

	Chip8_TestProgram();
	
	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) running = false;
			else if (!MessageForDialog(&msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		double currentTime = (double)GetTickCount();
		accumulator += (currentTime - lastTime);
		lastTime = currentTime;

		while (accumulator >= cycleFreq) {
			Chip8_Cycle();
			if (c8.displayUpdate) Draw_PaintFrame(deviceContext);
			accumulator -= cycleFreq;
		}
	}

	Draw_Exit();
	ReleaseDC(hwnd, deviceContext);
	DestroyWindow(hwnd);
	return 0;
}

void HandleCommand(HWND hwnd, word cmd) {
	switch (cmd) {
	case ID_FILE_QUIT:
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		break;
	case ID_FILE_OPENROM: {
			char fileBuffer[MAX_PATH + 1];
			const char* filters = "Chip8 ROMs (*.c8)\0*.c8\0All Files (*.*)\0*.*\0";
			if (FilePromptOpen(hwnd, fileBuffer, MAX_PATH, filters, "Open ROM File")) {
				Chip8_LoadProgram(fileBuffer);
			}
		} break;
	case ID_VIEW_MEMORY: {
			HWND dlg = CreateDialog(GetModuleHandle(0), MAKEINTRESOURCE(IDD_MEMORY), hwnd, DialogProc_Memory);
			if (dlg) {
				ShowWindow(dlg, SW_SHOW);
				dialogs[DLG_MEMORY] = dlg;
			}
			else NotifyError();
		} break;
	case ID_HELP_ABOUT:
		INT result = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ABOUT), hwnd, DialogProc_About);
		if (result == -1) NotifyError();
		break;
	}
}