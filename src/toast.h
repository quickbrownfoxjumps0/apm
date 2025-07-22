#ifndef TOAST_H
#define TOAST_H

#include <windows.h>

void show_toast(HWND hwnd, const char *message);
LRESULT CALLBACK ToastWndProc(HWND hwnd, UINT msg, WPARAM wParam,
			      LPARAM lParam);

#endif				// TOAST_H
