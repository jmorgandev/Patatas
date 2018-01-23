#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <list>
#include "stdafx.h"
#include "input.h"
#include "chip8.h"
#include "resource.h"
#include "windraw.h"#
#include "sound.h"
#include <CommCtrl.h>

#include "modeless_dialog.h"
HWND modeless_dialogs[DLG_COUNT];

#include "dialog_about.h"
#include "dialog_file.h"
#include "dialog_opsettings.h"
#include "dialog_customspeed.h"
#include "dialog_keymap.h"
#include "dialog_color.h"

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
		"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Winmm.lib")

const char* winClass = "PTS_WINCLASS";
const char* winTitle = "Patatas";

void HandleCommand(HWND, word);

uint winStyle = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
HWND winHandle = NULL;
HMENU winMenu = NULL;
HDC deviceContext = NULL;

static bool running = true;
static bool paused = false;
static bool stepForward = false;

bool MessageForDialog(MSG* msg) {
	bool handled = false;
	for (HWND hwnd : modeless_dialogs) {
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

	winHandle = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		winClass,
		winTitle,
		winStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		NULL, NULL, hInstance, NULL
	);

	if (winHandle == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	deviceContext = GetDC(winHandle);
	if (deviceContext == NULL) {
		MessageBox(NULL, "Could not get device context!", "Error",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	HACCEL accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	winMenu = GetMenu(winHandle);
	CheckMenuRadioItem(winMenu, ID_WINSIZE_X4, ID_WINSIZE_CUSTOM, ID_WINSIZE_X4 + scaleIndex, MF_BYCOMMAND);

	ShowWindow(winHandle, nCmdShow);
	UpdateWindow(winHandle);

	Chip8_Init();
	Draw_Init();
	Input_Init();
	Sound_Init();

	double lastTime = (double)GetTickCount();
	double cycleTime = 0;
	double tickTime = 0;

	Chip8_TestProgram();
	
	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) running = false;
			else if (!MessageForDialog(&msg) && !TranslateAccelerator(winHandle, accel, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if (paused) {
			if (stepForward) {
				cycleTime += cycleFreq;
				Chip8_Cycle();
				if (cycleTime >= tickFreq) {
					Chip8_Tick();
					cycleTime -= tickFreq;
				}
				stepForward = false;
			}
		}
		else {
			double currentTime = (double)GetTickCount();
			cycleTime += (currentTime - lastTime);
			tickTime += (currentTime - lastTime);
			lastTime = currentTime;

			if (cycleTime >= cycleFreq || tickTime >= tickFreq) {
				while (cycleTime >= cycleFreq) {
					Chip8_Cycle();
					if (c8.displayUpdate) Draw_PaintFrame(deviceContext);
					cycleTime -= cycleFreq;
				}
				while (tickTime >= tickFreq) {
					Chip8_Tick();
					tickTime -= tickFreq;
				}
			}
			else {
				//Sleep until next cycle/tick?
			}
		}
	}

	Draw_Exit();
	ReleaseDC(winHandle, deviceContext);
	DestroyWindow(winHandle);
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
				modeless_dialogs[DLG_MEMORY] = dlg;
			}
			else NotifyError();
		} break;
	case ID_VIEW_REGISTERS: {
			HWND dlg = CreateDialog(GetModuleHandle(0), MAKEINTRESOURCE(IDD_REGISTER), hwnd, DialogProc_Register);
			if (dlg) {
				ShowWindow(dlg, SW_SHOW);
				modeless_dialogs[DLG_REGISTER] = dlg;
			}
		} break;
	case ID_CHIP8_SETTINGS: {
			INT result = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_OPSETTINGS), hwnd, DialogProc_OpcodeSettings);
			if (result == -1) NotifyError();
		} break;
	case ID_HELP_ABOUT: {
			INT result = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ABOUT), hwnd, DialogProc_About);
			if (result == -1) NotifyError();
		} break;
	case ID_SPEED_CUSTOM: {
			INT result = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_CUSTOMSPEED), hwnd, DialogProc_CustomSpeed);
			if (result == -1) NotifyError();
		} break;
	case ID_WINSIZE_X4:
		Draw_SetScale(0);
		break;
	case ID_WINSIZE_X8:
		Draw_SetScale(1);
		break;
	case ID_WINSIZE_X12:
		Draw_SetScale(2);
		break;
	case ID_WINSIZE_X16:
		Draw_SetScale(3);
		break;
	case ID_WINSIZE_X20:
		Draw_SetScale(4);
		break;
	case ID_WINSIZE_CUSTOM:
		Draw_SetScale(CUSTOM_SCALE);
		break;
	case ID_CONFIG_KEYMAP: {
			INT result = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_KEYMAP), hwnd, DialogProc_Keymap);
			if (result == -1) NotifyError();
		} break;
	case ID_CONFIG_COLORS: {
			INT result = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_COLOR), hwnd, DialogProc_Color);
			if (result == -1) NotifyError();
		} break;
	case ID_CHIP8_PAUSE:
		paused = !paused;
		break;
	case ID_CHIP8_STEP:
		if (paused) stepForward = true;
		break;
	case ID_CHIP8_RESET:
		break;
	}
}