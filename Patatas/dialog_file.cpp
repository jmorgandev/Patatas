#include "dialog_file.h"

bool FilePromptOpen(HWND hwnd, char* buffer, uint size, const char* filters, const char* title) {
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrTitle = title;
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filters;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buffer;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = size;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	return GetOpenFileName(&ofn);
}