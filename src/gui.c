#include "gui.h"
#include <windows.h>
#include <commdlg.h> // for GetOpenFileName
#include <string.h>

#define WINDOW_CLASS_NAME "APMWindowClass"
#define MAX_PATH_LEN 512

/* Controls */
#define ID_FILE_OPEN  101
#define ID_FILE_RESET 102
#define ID_FILE_EXIT 103
#define ID_HELP_ABOUT 111

#define ID_PASSWORD_EDIT 201
#define ID_LOGIN_BUTTON 202

/* Global */
static char db_path[MAX_PATH_LEN] = {0};
static HWND hwndPassword = NULL;
static HWND hwndButton = NULL;

// Helper: get filename from full path
static const char* get_filename_from_path(const char *path)
{
    const char *slash = strrchr(path, '\\');
    if (slash) return slash + 1;
    slash = strrchr(path, '/');
    if (slash) return slash + 1;
    return path;
}

static void menubar(HWND hwnd)
{
	/* File */
	HMENU hFileMenu = CreateMenu();
	AppendMenuA(hFileMenu, MF_STRING, ID_FILE_OPEN, "Open DB file");
	AppendMenuA(hFileMenu, MF_STRING, ID_FILE_RESET, "Reset");
	AppendMenuA(hFileMenu, MF_STRING, ID_FILE_EXIT, "Exit");

	/* Help */
	HMENU hHelpMenu = CreateMenu();
	AppendMenuA(hHelpMenu, MF_STRING, ID_HELP_ABOUT, "About");

	/* Menubar */
	HMENU hMenubar = CreateMenu();
	AppendMenuA(hMenubar, MF_POPUP, (UINT_PTR)hFileMenu, "File");
	AppendMenuA(hMenubar, MF_POPUP, (UINT_PTR)hHelpMenu, "Help");

	SetMenu(hwnd, hMenubar);
}

// Draw centered filename text with bold menu font, returns bottom Y of text for control positioning
static int draw_centered_filename(HWND hwnd, HDC hdc, int top)
{
    RECT rect;
    GetClientRect(hwnd, &rect);

    NONCLIENTMETRICS ncm = {0};
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    HFONT hFont = NULL;

    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0)) {
        LOGFONT lf = ncm.lfMenuFont;
        lf.lfWeight = FW_BOLD;
        lf.lfHeight = -24; // 24 pt font size
        hFont = CreateFontIndirect(&lf);
    }

    if (!hFont) {
        hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    const char *filename = (db_path[0]) ? get_filename_from_path(db_path) : "(No database selected)";

    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);

    RECT drawRect = rect;
    drawRect.top = top;
    drawRect.bottom = top + 40; // fixed height

    DrawTextA(hdc, filename, -1, &drawRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    int bottomY = drawRect.bottom;

    SelectObject(hdc, hOldFont);
    if (hFont != GetStockObject(DEFAULT_GUI_FONT)) {
        DeleteObject(hFont);
    }

    return bottomY;
}

static void first_screen_paint(HWND hwnd, int startY)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    draw_centered_filename(hwnd, hdc, startY);

    EndPaint(hwnd, &ps);
}

static void first_screen_create_controls(HWND hwnd)
{
    if (!hwndPassword) {
        hwndPassword = CreateWindowExA(
            0, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD,
            0, 0, 200, 25,
            hwnd, (HMENU)ID_PASSWORD_EDIT, NULL, NULL);
    }

    if (!hwndButton) {
        hwndButton = CreateWindowExA(
            0, "BUTTON", "Register",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 100, 30,
            hwnd, (HMENU)ID_LOGIN_BUTTON, NULL, NULL);
    }

    // Set font for controls to match menu font but normal weight
    NONCLIENTMETRICS ncm = {0};
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    HFONT hFont = NULL;
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0)) {
        LOGFONT lf = ncm.lfMenuFont;
        lf.lfWeight = FW_NORMAL;
        lf.lfHeight = -16; // 16 pt font size for controls
        hFont = CreateFontIndirect(&lf);
    }

    if (!hFont) {
        hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }

    SendMessage(hwndPassword, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hwndButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    if (hFont != GetStockObject(DEFAULT_GUI_FONT)) {
        DeleteObject(hFont);
    }
}

static void first_screen_layout_controls(HWND hwnd)
{
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    int padding = 10;
    int textHeight = 40;
    int passwordHeight = 25;
    int buttonHeight = 30;

    // total height of all elements and paddings
    int totalHeight = textHeight + padding + passwordHeight + padding + buttonHeight;

    // Calculate vertical start so group is vertically centered
    int startY = (rcClient.bottom - totalHeight) / 2;

    // Position password box
    int passwordWidth = 200;
    int passwordX = (rcClient.right - passwordWidth) / 2;
    int passwordY = startY + textHeight + padding;
    SetWindowPos(hwndPassword, NULL, passwordX, passwordY, passwordWidth, passwordHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

    // Position button below password box
    int buttonWidth = 100;
    int buttonX = (rcClient.right - buttonWidth) / 2;
    int buttonY = passwordY + passwordHeight + padding;
    SetWindowPos(hwndButton, NULL, buttonX, buttonY, buttonWidth, buttonHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

    // Update button text based on db_path
    if (db_path[0]) {
        SetWindowTextA(hwndButton, "Login");
    } else {
        SetWindowTextA(hwndButton, "Register");
    }

    // Trigger repaint with new startY stored somewhere or passed
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);

    // Store or communicate startY for paint:
    // For simplicity, save it in window user data or a global variable.
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)startY);
}

static void open_file_dialog(HWND hwnd)
{
    OPENFILENAME ofn;
    char szFile[MAX_PATH_LEN] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "SQLite Databases (*.db;*.db3;*.sqlite;*.sqlite3)\0*.db;*.db3;*.sqlite;*.sqlite3\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        // Copy selection to db_path
        strncpy(db_path, ofn.lpstrFile, MAX_PATH_LEN - 1);
        db_path[MAX_PATH_LEN - 1] = '\0';

	first_screen_layout_controls(hwnd);
	InvalidateRect(hwnd, NULL, TRUE);
	UpdateWindow(hwnd);
    }
}

static void reset_db_path(HWND hwnd)
{
	db_path[0] = '\0';
	first_screen_layout_controls(hwnd);
	InvalidateRect(hwnd, NULL, TRUE);
	UpdateWindow(hwnd);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	switch (uMsg) {
		case WM_CREATE:
			menubar(hwnd);
			first_screen_create_controls(hwnd);
			first_screen_layout_controls(hwnd);
			break;

		case WM_PAINT:
			int startY = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (startY == 0) {
				// If not set yet, default to original 50
				startY = 50;
			}
			first_screen_paint(hwnd, startY);
			break;

		case WM_SIZE:
			first_screen_layout_controls(hwnd);
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_FILE_OPEN:
					open_file_dialog(hwnd);
					break;
				case ID_FILE_RESET:
					reset_db_path(hwnd);
					break;
				case ID_FILE_EXIT:
					PostQuitMessage(0);
					break;
				case ID_HELP_ABOUT:
					MessageBoxA(hwnd, "APM - Advanced Password Manager\n\nThis is a minimal password manager.\n(c) 2025", "About", MB_OK | MB_ICONINFORMATION);
					break;
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

void run_gui(void) 
{
	HINSTANCE hInstance = GetModuleHandleA(NULL);

	WNDCLASSA wc = {0};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	RegisterClassA(&wc);

	HWND hwnd = CreateWindowExA(
			0,
			WINDOW_CLASS_NAME,
			"APM - Advanced Password Manager 1.0",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
			NULL, NULL, hInstance, NULL
			);

	if (!hwnd) {
		MessageBoxA(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessageA(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}
