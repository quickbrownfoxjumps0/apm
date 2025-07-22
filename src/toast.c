#include "toast.h"
#include <windows.h>


void show_toast(HWND hwnd, const char *message)
{
	static HWND toast = NULL;

	if (toast) {
		DestroyWindow(toast);
		toast = NULL;
	}

	toast = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, "STATIC", NULL,	// no text here, we draw it ourselves
				WS_POPUP | WS_BORDER | SS_CENTER,
				0, 0, 300, 40, hwnd, NULL, NULL, NULL);

	if (!toast)
		return;

	// Store message pointer in window user data for use in wndproc
	SetWindowLongPtr(toast, GWLP_USERDATA, (LONG_PTR) message);

	RECT rc;
	GetWindowRect(hwnd, &rc);	// Now rc is in screen coordinates

	int toastWidth = 300;
	int toastHeight = 40;

	int x = rc.left + ((rc.right - rc.left) - toastWidth) / 2;
	int y = rc.bottom - toastHeight - 80;	// Add small margin from bottom

	SetWindowPos(toast, HWND_TOPMOST, x, y, toastWidth, toastHeight,
		     SWP_SHOWWINDOW);
	SetTimer(toast, 1, 2000, NULL);	// Auto-hide after 3 seconds

	SetWindowLongPtr(toast, GWLP_WNDPROC, (LONG_PTR) ToastWndProc);
}

LRESULT CALLBACK ToastWndProc(HWND h, UINT msg, WPARAM w, LPARAM l)
{
	if (msg == WM_TIMER) {
		KillTimer(h, 1);
		DestroyWindow(h);
		return 0;
	} else if (msg == WM_PAINT) {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(h, &ps);

		// Retrieve message pointer from user data
		const char *message =
			(const char *) GetWindowLongPtr(h, GWLP_USERDATA);

		DrawTextA(hdc, message, -1, &ps.rcPaint,
			  DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		EndPaint(h, &ps);
		return 0;
	}
	return DefWindowProc(h, msg, w, l);
}
