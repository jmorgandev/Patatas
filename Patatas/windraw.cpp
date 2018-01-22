#include "windraw.h"
#include "chip8.h"
#include "resource.h"
#include "dialog_customsize.h"

uint drawScale = 12;
uint scaleIndex = 2;

static uint bgColor = 0x362B00; //002B36
static uint fgColor = 0xE3F6FD; //FDF6E3
static HBRUSH bgBrush = NULL;
static HBRUSH fgBrush = NULL;

static uint scaleArray[5] = {
	4, 8, 12, 16, 20
};

bool Draw_Init() {

	bgBrush = CreateSolidBrush(bgColor);
	if (bgBrush == NULL) return false;

	fgBrush = CreateSolidBrush(fgColor);
	if (fgBrush == NULL) return false;

	return true;
}

void Draw_SetColors(uint bg, uint fg) {
	if (bg != bgColor) {
		HBRUSH newBrush = CreateSolidBrush(bg);
		if (newBrush == NULL) return;
		DeleteObject(bgBrush);
		bgBrush = newBrush;
	}
	if (fg != fgColor) {
		HBRUSH newBrush = CreateSolidBrush(fg);
		if (newBrush == NULL) return;
		DeleteObject(fgBrush);
		fgBrush = newBrush;
	}
	bgColor = bg;
	fgColor = fg;

	//Post a paint message...
}

COLOR Draw_GetColorBG() {
	COLOR c = { 0 };
	c.r = (bgColor & 0x0000FF);
	c.g = (bgColor & 0x00FF00) >> 8;
	c.b = (bgColor & 0xFF0000) >> 16;
	return c;
}
COLOR Draw_GetColorFG() {
	COLOR c = { 0 };
	c.r = (fgColor & 0x0000FF);
	c.g = (fgColor & 0x00FF00) >> 8;
	c.b = (fgColor & 0xFF0000) >> 16;
	return c;
}

void Draw_SetColorBG(COLOR c, bool redraw) {
	uint newColor = RGB(c.r, c.g, c.b);
	HBRUSH newBrush = CreateSolidBrush(newColor);
	if (newBrush == NULL) return;
	if(bgBrush != NULL) DeleteObject(bgBrush);
	bgBrush = newBrush;
	bgColor = newColor;
	if (redraw) {
		RECT rect = { 0 };
		GetClientRect(winHandle, &rect);
		InvalidateRect(winHandle, &rect, FALSE);
	}
}
void Draw_SetColorFG(COLOR c, bool redraw) {
	uint newColor = RGB(c.r, c.g, c.b);
	HBRUSH newBrush = CreateSolidBrush(newColor);
	if (newBrush == NULL) return;
	if (fgBrush != NULL) DeleteObject(fgBrush);
	fgBrush = newBrush;
	fgColor = newColor;
	if (redraw) {
		RECT rect = { 0 };
		GetClientRect(winHandle, &rect);
		InvalidateRect(winHandle, &rect, FALSE);
	}
}
void Draw_SetColors(COLOR bg, COLOR fg, bool redraw) {
	Draw_SetColorBG(bg, false);
	Draw_SetColorFG(fg, false);
	if (redraw) {
		RECT rect = { 0 };
		GetClientRect(winHandle, &rect);
		InvalidateRect(winHandle, &rect, FALSE);
	}
}

void Draw_PaintFrame(HDC deviceContext) {
	RECT rect = {0, 0, VID_WIDTH * drawScale, VID_HEIGHT * drawScale };
	//Fill the background
	FillRect(deviceContext, &rect, bgBrush);

	//Draw pixels that are ON
	rect = { 0, 0, (LONG)drawScale, (LONG)drawScale };

	for (uint byt = 0; byt < (VID_WIDTH / 8) * VID_HEIGHT; byt++) {
		for (uint bit = 0; bit < 8; bit++) {
			rect.left = (((byt * 8) + bit) % VID_WIDTH)*drawScale;
			rect.top = (byt / 8) * drawScale;
			rect.right = rect.left + drawScale;
			rect.bottom = rect.top + drawScale;

			if ((c8.display[byt] >> (7 - bit)) & 0x01) {
				FillRect(deviceContext, &rect, fgBrush);
			}
		}
	}
	c8.displayUpdate = false;
}

void Draw_Exit() {
	DeleteObject(bgBrush);
	DeleteObject(fgBrush);
}

void Draw_SetScale(uint menuIndex) {
	if (menuIndex == CUSTOM_SCALE) {
		INT result = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_CUSTOMSIZE), winHandle, DialogProc_CustomSize);
		if (result == -1) NotifyError();
		Draw_RefreshWindow();
		CheckMenuRadioItem(winMenu, ID_WINSIZE_X4, ID_WINSIZE_CUSTOM, ID_WINSIZE_CUSTOM, MF_BYCOMMAND);
	}
	else if(drawScale != scaleArray[menuIndex]) {
		drawScale = scaleArray[menuIndex];
		Draw_RefreshWindow();
		CheckMenuRadioItem(winMenu, ID_WINSIZE_X4, ID_WINSIZE_CUSTOM, ID_WINSIZE_X4 + menuIndex, MF_BYCOMMAND);
		scaleIndex = menuIndex;
	}
}

void Draw_RefreshWindow() {
	uint border = GetSystemMetrics(SM_CXSIZEFRAME);
	RECT oldRect = { 0 };
	GetWindowRect(winHandle, &oldRect);
	RECT newRect = { 0, 0, (VID_WIDTH * drawScale) + border, (VID_HEIGHT * drawScale) + border };
	InvalidateRect(winHandle, &newRect, FALSE);
	AdjustWindowRect(&newRect, winStyle, TRUE);
	MoveWindow(winHandle, oldRect.left, oldRect.top, newRect.right - newRect.left, newRect.bottom - newRect.top, TRUE);
}