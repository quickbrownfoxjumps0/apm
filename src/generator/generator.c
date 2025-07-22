#include "generator.h"
#include "entropy.h"
#include "diceware-dict.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static WNDPROC originalOutputProc = NULL;
static HWND hwndWindow = NULL;
static HWND hwndCombo = NULL;
static HWND hwndEntropyEdit = NULL;
static HWND hwndGenerateBtn = NULL;
static HWND hwndOutput = NULL;

enum Strategy {
	STRATEGY_RANDOM = 0,
	STRATEGY_DICEWARE
};

static void generate_password(HWND hwnd)
{
	char buffer[1024] = { 0 };
	int strategy = (int) SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
	BOOL valid = FALSE;
	int bits = GetDlgItemInt(hwnd, 1002, &valid, FALSE);
	if (!valid || bits < 50 || bits > 128) {
		MessageBox(hwnd, "Enter entropy between 50 and 128.", "Error",
			   MB_OK | MB_ICONERROR);
		return;
	}

	if (strategy == STRATEGY_RANDOM) {
		generate_random_password(bits, buffer, sizeof(buffer));
	} else {
		generate_diceware_password(bits, buffer, sizeof(buffer));
	}

	SetWindowTextA(hwndOutput, buffer);
}

static LRESULT CALLBACK OutputEditProc(HWND hwnd, UINT msg, WPARAM wParam,
				       LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		if (GetKeyState(VK_CONTROL) & 0x8000) {
			if (wParam == 'A') {
				
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				return 0;
			} else if (wParam == 'C') {
				
				SendMessage(hwnd, WM_COPY, 0, 0);
				return 0;
			}
		}
		break;
	}
	return CallWindowProc(originalOutputProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK GeneratorWndProc(HWND hwnd, UINT msg, WPARAM wParam,
					 LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
		CREATESTRUCTA *cs = (CREATESTRUCTA *)lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->hwndParent);
		CreateWindowA("STATIC", "Strategy:", WS_CHILD | WS_VISIBLE, 10,
			      10, 60, 20, hwnd, NULL, NULL, NULL);

		hwndCombo = CreateWindowA("COMBOBOX", NULL,
					  WS_CHILD | WS_VISIBLE |
					  CBS_DROPDOWNLIST, 80, 10, 200, 200,
					  hwnd, (HMENU) 1001, NULL, NULL);
		SendMessage(hwndCombo, CB_ADDSTRING, 0,
			    (LPARAM) "Random Characters");
		SendMessage(hwndCombo, CB_ADDSTRING, 0,
			    (LPARAM) "Diceware Passphrase");
		SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);

		CreateWindowA("STATIC", "Min Entropy:", WS_CHILD | WS_VISIBLE,
			      10, 50, 120, 20, hwnd, NULL, NULL, NULL);

		hwndEntropyEdit =
			CreateWindowA("EDIT", "80",
				      WS_CHILD | WS_VISIBLE | WS_BORDER |
				      ES_NUMBER, 100, 50, 60, 20, hwnd,
				      (HMENU) 1002, NULL, NULL);

		hwndGenerateBtn =
			CreateWindowA("BUTTON", "Generate",
				      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				      10, 90, 80, 30, hwnd, (HMENU) 1003, NULL,
				      NULL);

		hwndOutput = CreateWindowA("EDIT", NULL,
					   WS_CHILD | WS_VISIBLE | WS_BORDER |
					   ES_AUTOHSCROLL | ES_READONLY, 10,
					   130, 360, 25, hwnd, NULL, NULL,
					   NULL);
		originalOutputProc =
			(WNDPROC) SetWindowLongPtr(hwndOutput, GWLP_WNDPROC,
						   (LONG_PTR) OutputEditProc);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == 1003) {	
			generate_password(hwnd);
		}
		break;

	case WM_DESTROY:
		HWND parent_hwnd = (HWND)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		hwndWindow = NULL;
		if (parent_hwnd) {
			ShowWindow(parent_hwnd, SW_SHOW);
			SetForegroundWindow(parent_hwnd);
			SetFocus(parent_hwnd);
		}
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void show_generator_window(HWND parent_hwnd)
{
	if (hwndWindow) {
		ShowWindow(hwndWindow, SW_SHOW);
		SetForegroundWindow(hwndWindow);
		return;
	}

	HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASSA wc = { 0 };
	wc.lpfnWndProc = GeneratorWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "PasswordGenerator";
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);

	RegisterClassA(&wc);

	hwndWindow = CreateWindowExA(0, "PasswordGenerator",
				     "Password Generator",
				     WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
				     WS_MINIMIZEBOX, CW_USEDEFAULT,
				     CW_USEDEFAULT, 400, 200, parent_hwnd,
				     NULL, hInstance, NULL);

	if (hwndWindow) {
		ShowWindow(hwndWindow, SW_SHOW);
		UpdateWindow(hwndWindow);
	}
}

