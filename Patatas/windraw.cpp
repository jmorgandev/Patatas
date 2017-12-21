#include "windraw.h"
#include "chip8.h"

uint drawScale = 12;

uint bgColor = 0x362B00; //002B36
uint fgColor = 0xE3F6FD; //FDF6E3
HBRUSH bgBrush = NULL;
HBRUSH fgBrush = NULL;

double framesPerSecond = 60;
double renderFreq = 1000.0 / 60;

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


			if ((display[byt] >> (7 - bit)) & 0x01) {
				FillRect(deviceContext, &rect, fgBrush);
			}
		}
	}
	displayUpdate = false;
}

void Draw_Exit() {
	DeleteObject(bgBrush);
	DeleteObject(fgBrush);
}