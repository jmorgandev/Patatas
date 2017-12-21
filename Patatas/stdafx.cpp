#include "stdafx.h"
#include <Windows.h>

void OpenURL(const char* url) {
	ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

void NotifyError() {
	DWORD code = GetLastError();
	WCHAR buffer[512];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL, GetLastError(), 0, (LPSTR)buffer, 512, NULL);
	MessageBox(NULL, (LPSTR)buffer, "Error", MB_OK | MB_ICONEXCLAMATION);
}