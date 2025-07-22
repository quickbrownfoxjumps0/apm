#include "gui.h"
#include "db.h"
#include "toast.h"
#include "entry-dialog.h"
#include "entry-list.h"
#include "utils.h"
#include "resource.h"
#include <windows.h>
#include <io.h>			// for _access()
#include <commdlg.h>		// for GetOpenFileName
#include <commctrl.h>
#include <string.h>
#include <stdio.h>


/* Forward declarations */
LRESULT CALLBACK PasswordEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
					  LPARAM lParam);

/* Global */
typedef enum {
	SCREEN_LOGIN,
	SCREEN_MANAGER
} ScreenState;

static ScreenState current_screen = -1;
static char db_path[MAX_PATH_LEN] = { 0 };

static char *clipboard_backup = NULL;	// To compare before clearing
static WNDPROC OriginalEditProc = NULL;

// Login screen menu and controls
static HMENU hMenuLogin = NULL;
static HWND hwndLabelDbFile = NULL;	// to show selected DB file on login screen
static HWND hwndPassword = NULL;
static HWND hwndLoginButton = NULL;

// Manager screen menu and controls
static HMENU hMenuManager = NULL;	// future use
static HWND hwndListView = NULL;
static HWND hwndAddButton = NULL;
static HWND hwndEditButton = NULL;
static HWND hwndDeleteButton = NULL;

/*
 * ================================
 * SCREENS
 * ================================
 */

/* Login screen */

// Create login screen menu
void create_login_menu(HWND hwnd)
{
	if (hMenuLogin)
		return;		// already created

	hMenuLogin = CreateMenu();
	HMENU hFileMenu = CreatePopupMenu();

	AppendMenuA(hFileMenu, MF_STRING, ID_FILE_OPEN, "Open...");
	AppendMenuA(hFileMenu, MF_STRING, ID_FILE_RESET, "Reset");
	AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuA(hFileMenu, MF_STRING, ID_FILE_EXIT, "Exit");

	AppendMenuA(hMenuLogin, MF_POPUP, (UINT_PTR) hFileMenu, "File");
	AppendMenuA(hMenuLogin, MF_STRING, ID_GENERATE, "Generate");

	SetMenu(hwnd, hMenuLogin);
}

void destroy_login_menu(HWND hwnd)
{
	if (hMenuLogin) {
		SetMenu(hwnd, NULL);
		DestroyMenu(hMenuLogin);
		hMenuLogin = NULL;
	}
}

void update_db_label_text()
{
	if (!hwndLabelDbFile)
		return;

	const char *filename =
		(db_path[0] !=
		 '\0') ? get_filename_from_path(db_path) : "(No DB selected)";
	SetWindowTextA(hwndLabelDbFile, filename);
}

void update_login_button_text()
{
	if (!hwndLoginButton)
		return;

	const char *btnText = (db_path[0] == '\0') ? "Register" : "Login";
	SetWindowTextA(hwndLoginButton, btnText);
}

// Function to create login screen controls:
void create_login_screen(HWND hwnd)
{
// Create DB filename label
	hwndLabelDbFile = CreateWindowExA(0, "STATIC", "",
					  WS_CHILD | WS_VISIBLE | SS_CENTER,
					  0, 0, 100, 20,
					  hwnd, NULL, GetModuleHandle(NULL),
					  NULL);

	// Create password edit box
	hwndPassword = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
				       WS_CHILD | WS_VISIBLE | WS_BORDER |
				       ES_PASSWORD | ES_AUTOHSCROLL, 0, 0, 100,
				       25, hwnd, (HMENU) ID_PASSWORD_EDIT,
				       GetModuleHandle(NULL), NULL);
	SetFocus(hwndPassword);
	OriginalEditProc =
		(WNDPROC) SetWindowLongPtr(hwndPassword, GWLP_WNDPROC,
					   (LONG_PTR)
					   PasswordEditSubclassProc);

	// Create login/register button
	hwndLoginButton = CreateWindowExA(0, "BUTTON", "Register",
					  WS_CHILD | WS_VISIBLE |
					  BS_PUSHBUTTON, 0, 0, 100, 30, hwnd,
					  (HMENU) ID_LOGIN_BUTTON,
					  GetModuleHandle(NULL), NULL);

	update_login_button_text();
	update_db_label_text();
}

// Function to destroy login screen controls:
void destroy_login_screen(void)
{
	if (hwndLabelDbFile) {
		DestroyWindow(hwndLabelDbFile);
		hwndLabelDbFile = NULL;
	}
	if (hwndPassword) {
		SetWindowText(hwndPassword, "");
		SendMessage(hwndPassword, EM_SETSEL, 0, -1);
		SendMessage(hwndPassword, EM_REPLACESEL, TRUE, (LPARAM) "");
		DestroyWindow(hwndPassword);
		hwndPassword = NULL;
	}
	if (hwndLoginButton) {
		DestroyWindow(hwndLoginButton);
		hwndLoginButton = NULL;
	}
}

// Layout function: position controls in login screen
void layout_login_screen(HWND hwnd, int width, int height)
{
	if (!hwndPassword || !hwndLoginButton || !hwndLabelDbFile)
		return;

	const int spacing = 10;

	// Get control sizes (we can fix height or query)
	// RECT rcLabel, rcPassword, rcButton;

	// For simplicity assume fixed heights or get from controls
	int label_height = 20;
	int edit_height = 25;
	int button_height = 30;

	// Calculate total height (3 controls + 2 spacings)
	int total_height =
		label_height + spacing + edit_height + spacing + button_height;

	// Calculate top offset for vertical centering
	int top = (height - total_height) / 2;

	// Center horizontally: width of controls - let label and password box stretch full width minus margins, button fixed width
	const int margin = 50;
	int ctrl_width = width - 2 * margin;
	int button_width = 100;

	// Position Label (db filename)
	MoveWindow(hwndLabelDbFile, margin, top, ctrl_width, label_height,
		   TRUE);

	// Position Password edit box below label
	MoveWindow(hwndPassword, margin, top + label_height + spacing,
		   ctrl_width, edit_height, TRUE);

	// Position Login/Register button below password box, centered horizontally
	int button_x = (width - button_width) / 2;
	MoveWindow(hwndLoginButton, button_x,
		   top + label_height + spacing + edit_height + spacing,
		   button_width, button_height, TRUE);

	update_db_label_text();
	update_login_button_text();
}



/* Manager screen */

void create_manager_menu(HWND hwnd)
{
	if (hMenuManager)
		return;

	hMenuManager = CreateMenu();
	HMENU hFileMenu = CreatePopupMenu();

	AppendMenuA(hFileMenu, MF_STRING, ID_FILE_LOGOUT, "Logout");
	AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuA(hFileMenu, MF_STRING, ID_FILE_EXIT, "Exit");

	AppendMenuA(hMenuManager, MF_POPUP, (UINT_PTR) hFileMenu, "File");
	AppendMenuA(hMenuManager, MF_STRING, ID_GENERATE, "Generate");

	SetMenu(hwnd, hMenuManager);
}

void destroy_manager_menu(HWND hwnd)
{
	if (hMenuManager) {
		SetMenu(hwnd, NULL);
		DestroyMenu(hMenuManager);
		hMenuManager = NULL;
	}
}

void create_manager_screen(HWND hwnd)
{
	hwndListView = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
				       WS_CHILD | WS_VISIBLE | LVS_REPORT |
				       LVS_OWNERDATA | LVS_SINGLESEL, 0, 0,
				       100, 100, hwnd, (HMENU) ID_ENTRY_LIST,
				       GetModuleHandle(NULL), NULL);

	ListView_SetExtendedListViewStyle(hwndListView,
					  LVS_EX_FULLROWSELECT |
					  LVS_EX_GRIDLINES);

	hwndAddButton =
		CreateWindowExA(0, "BUTTON", "Add", WS_CHILD | WS_VISIBLE, 0,
				0, 80, 25, hwnd, (HMENU) ID_BUTTON_ADD,
				GetModuleHandle(NULL), NULL);

	hwndEditButton =
		CreateWindowExA(0, "BUTTON", "Edit", WS_CHILD | WS_VISIBLE, 0,
				0, 80, 25, hwnd, (HMENU) ID_BUTTON_EDIT,
				GetModuleHandle(NULL), NULL);

	hwndDeleteButton =
		CreateWindowExA(0, "BUTTON", "Delete", WS_CHILD | WS_VISIBLE,
				0, 0, 80, 25, hwnd, (HMENU) ID_BUTTON_DELETE,
				GetModuleHandle(NULL), NULL);

	// Setup ListView columns
	LVCOLUMN col = { 0 };
	col.mask = LVCF_TEXT | LVCF_WIDTH;

	col.pszText = "ID";
	col.cx = 50;
	ListView_InsertColumn(hwndListView, 0, &col);

	col.pszText = "Service";
	col.cx = 200;
	ListView_InsertColumn(hwndListView, 1, &col);

	col.pszText = "Username";
	col.cx = 200;
	ListView_InsertColumn(hwndListView, 2, &col);

	col.pszText = "Notes";
	col.cx = 200;
	ListView_InsertColumn(hwndListView, 3, &col);

	populate_entry_list(hwndListView);
}

void destroy_manager_screen(void)
{
	if (hwndListView) {
		DestroyWindow(hwndListView);
		hwndListView = NULL;
	}
	if (hwndAddButton) {
		DestroyWindow(hwndAddButton);
		hwndAddButton = NULL;
	}
	if (hwndEditButton) {
		DestroyWindow(hwndEditButton);
		hwndEditButton = NULL;
	}
	if (hwndDeleteButton) {
		DestroyWindow(hwndDeleteButton);
		hwndDeleteButton = NULL;
	}
}

void layout_manager_screen(HWND hwnd, int width, int height)
{
	const int button_height = 30;
	const int button_width = 80;
	const int button_spacing = 10;
	const int padding = 10;

	int list_height = height - button_height - 3 * padding;

	MoveWindow(hwndListView, padding, padding, width - 2 * padding,
		   list_height, TRUE);

	int total_button_width = 3 * button_width + 2 * button_spacing;
	int left = (width - total_button_width) / 2;
	int top = list_height + 2 * padding;

	MoveWindow(hwndAddButton, left, top, button_width, button_height,
		   TRUE);
	MoveWindow(hwndEditButton, left + button_width + button_spacing, top,
		   button_width, button_height, TRUE);
	MoveWindow(hwndDeleteButton,
		   left + 2 * (button_width + button_spacing), top,
		   button_width, button_height, TRUE);
}



void set_screen(ScreenState new_screen, HWND hwnd)
{
	if (current_screen == new_screen)
		return;

	// Cleanup old screen and menu
	if (current_screen == SCREEN_LOGIN) {
		destroy_login_screen();
		destroy_login_menu(hwnd);
	} else if (current_screen == SCREEN_MANAGER) {
		destroy_manager_screen();
		destroy_manager_menu(hwnd);
	}

	current_screen = new_screen;

	// Create new screen and layout
	if (new_screen == SCREEN_LOGIN) {
		create_login_menu(hwnd);
		create_login_screen(hwnd);
		RECT rc;
		GetClientRect(hwnd, &rc);
		layout_login_screen(hwnd, rc.right, rc.bottom);
	} else if (new_screen == SCREEN_MANAGER) {
		create_manager_menu(hwnd);
		create_manager_screen(hwnd);
		RECT rc;
		GetClientRect(hwnd, &rc);
		layout_manager_screen(hwnd, rc.right, rc.bottom);
	}

	InvalidateRect(hwnd, NULL, TRUE);
}



/*
 * ================================
 * EVENTS
 * ================================
 */
static void open_file_dialog(HWND hwnd)
{
	OPENFILENAME ofn;
	char szFile[MAX_PATH_LEN] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter =
		"SQLite Databases (*.db;*.db3;*.sqlite;*.sqlite3)\0*.db;*.db3;*.sqlite;*.sqlite3\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE) {
		// Copy selection to db_path
		strncpy(db_path, ofn.lpstrFile, MAX_PATH_LEN - 1);
		db_path[MAX_PATH_LEN - 1] = '\0';
		update_db_label_text();
		update_login_button_text();
	}
}

static void reset_db_path(HWND hwnd)
{
	db_path[0] = '\0';
	update_db_label_text();
	update_login_button_text();
}

void handle_login_or_register(HWND hwnd)
{
	char password[256];
	GetWindowTextA(hwndPassword, password, sizeof(password));

	if (!is_ascii(password)) {
		MessageBox(hwnd,
			   "Password must contain only ASCII characters.",
			   "Invalid Password", MB_OK | MB_ICONWARNING);
		SecureZeroMemory(password, sizeof(password));
		return;
	}

	if (db_path[0] == '\0') {
		// Register mode — create vault.db if it doesn't exist
		const char *vault_name = "vault.db";
		const char *salt_name = "vault.db.salt";

		if (_access(vault_name, 0) != -1
		    || _access(salt_name, 0) != -1) {
			MessageBox(hwnd,
				   "vault.db or its .salt file already exists. Delete it first or choose a different filename.",
				   "File Exists", MB_OK | MB_ICONERROR);
			SecureZeroMemory(password, sizeof(password));
			return;
		}
		if (open_encrypted_db(vault_name, password, &db) == SQLITE_OK) {
			db_init_schema(db);
			// MessageBox(hwnd, "Database created successfully.", "Registered", MB_OK | MB_ICONINFORMATION);
			SecureZeroMemory(password, sizeof(password));
			set_screen(SCREEN_MANAGER, hwnd);

		} else {
			MessageBox(hwnd, "Failed to create database.", "Error",
				   MB_OK | MB_ICONERROR);
		}
	} else {
		// Login mode — open existing DB
		if (open_encrypted_db(db_path, password, &db) == SQLITE_OK) {
			// MessageBox(hwnd, "Login successful.", "Success", MB_OK | MB_ICONINFORMATION);
			SecureZeroMemory(password, sizeof(password));
			set_screen(SCREEN_MANAGER, hwnd);
		} else {
			MessageBox(hwnd,
				   "Login failed. Incorrect password? Missing or corrupted salt file?",
				   "Login Failed", MB_OK | MB_ICONERROR);
		}
	}

	SecureZeroMemory(password, sizeof(password));
}







/*
 * ================================
 * CALLBACKS/WINPROC
 * ================================
 */


LRESULT CALLBACK PasswordEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
					  LPARAM lParam)
{
	if (msg == WM_KEYDOWN) {
		BOOL ctrl = GetKeyState(VK_CONTROL) & 0x8000;
		switch (wParam) {
		case 'A':
			if (ctrl) {
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				return 0;
			}
			break;
		case 'V':
			if (ctrl) {
				SendMessage(hwnd, WM_PASTE, 0, 0);
				return 0;
			}
			break;
		}
	}

	return CallWindowProc(OriginalEditProc, hwnd, msg, wParam, lParam);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		set_screen(SCREEN_LOGIN, hwnd);
		break;
	case WM_SIZE:
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);

		if (current_screen == SCREEN_LOGIN) {
			layout_login_screen(hwnd, width, height);
		} else if (current_screen == SCREEN_MANAGER) {
			layout_manager_screen(hwnd, width, height);
		}
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
			free_entry_list(entry_cache);
			db_close(db);
			db = NULL;
			PostQuitMessage(0);
			break;
		case ID_GENERATE:
			MessageBox(hwnd, "Generate password clicked (TODO)",
				   "Generate", MB_OK);
			break;
		case ID_LOGIN_BUTTON:
			handle_login_or_register(hwnd);
			break;
		case ID_BUTTON_ADD:
			on_add_entry(hwnd);
			break;
		case ID_BUTTON_EDIT:
			HWND hwndList = GetDlgItem(hwnd, ID_ENTRY_LIST);
			int selectedIndex = -1;

			// Get selected item index
			for (int i = 0; i < ListView_GetItemCount(hwndList);
			     i++) {
				if (ListView_GetItemState
				    (hwndList, i,
				     LVIS_SELECTED) & LVIS_SELECTED) {
					selectedIndex = i;
					break;
				}
			}

			if (selectedIndex >= 0) {
				on_edit_entry(hwnd, selectedIndex);
			} else {
				MessageBox(hwnd,
					   "Please select an entry to edit.",
					   "No Selection", MB_ICONINFORMATION);
			}
			break;
		case ID_BUTTON_DELETE:{
				HWND hwndList =
					GetDlgItem(hwnd, ID_ENTRY_LIST);
				int selectedIndex = -1;

				for (int i = 0;
				     i < ListView_GetItemCount(hwndList);
				     i++) {
					if (ListView_GetItemState
					    (hwndList, i,
					     LVIS_SELECTED) & LVIS_SELECTED) {
						selectedIndex = i;
						break;
					}
				}

				if (selectedIndex < 0) {
					MessageBox(hwnd,
						   "Please select an entry to delete.",
						   "No Selection",
						   MB_ICONINFORMATION);
					break;
				}

				int confirm = MessageBox(hwnd,
							 "Are you sure you want to delete this entry?",
							 "Confirm Delete",
							 MB_YESNO |
							 MB_ICONWARNING);
				if (confirm != IDYES)
					break;

				Entry *e = entry_cache->entries[selectedIndex];
				if (db_delete_entry(db, e->id) != SQLITE_OK) {
					MessageBox(hwnd,
						   "Failed to delete entry.",
						   "Database Error",
						   MB_ICONERROR);
					break;
				}

				populate_entry_list(hwndList);	// Refresh the virtual list
				break;
			}
		}
		break;
	case WM_NOTIFY:{
			LPNMHDR nmhdr = (LPNMHDR) lParam;
			if (nmhdr->idFrom == ID_ENTRY_LIST
			    && nmhdr->code == LVN_GETDISPINFO) {
				NMLVDISPINFO *di = (NMLVDISPINFO *) lParam;

				if (!entry_cache
				    || di->item.iItem >= entry_cache->count)
					break;

				Entry *e =
					entry_cache->entries[di->item.iItem];

				if (di->item.mask & LVIF_TEXT) {
					static char buffer[256];	// temporary buffer

					switch (di->item.iSubItem) {
					case 0:
						snprintf(buffer,
							 sizeof(buffer), "%d",
							 e->id);
						break;
					case 1:
						strncpy(buffer, e->service,
							sizeof(buffer) - 1);
						buffer[sizeof(buffer) - 1] =
							'\0';
						break;
					case 2:
						strncpy(buffer, e->username,
							sizeof(buffer) - 1);
						buffer[sizeof(buffer) - 1] =
							'\0';
						break;
					case 3:
						strncpy(buffer, e->notes,
							sizeof(buffer) - 1);
						buffer[sizeof(buffer) - 1] =
							'\0';
						break;
					default:
						buffer[0] = '\0';
					}

					di->item.pszText = buffer;
				}
				break;
			} else if (nmhdr->idFrom == ID_ENTRY_LIST
				   && nmhdr->code == NM_DBLCLK) {
				NMLISTVIEW *lv = (NMLISTVIEW *) lParam;
				int index = lv->iItem;

				if (entry_cache && index >= 0
				    && index < entry_cache->count) {
					Entry *e = entry_cache->entries[index];

					char *password =
						db_get_password_by_id(db,
								      e->id);
					if (password) {
						free(clipboard_backup);
						clipboard_backup =
							_strdup(password);

						OpenClipboard(NULL);
						EmptyClipboard();
						HGLOBAL hGlob =
							GlobalAlloc
							(GMEM_MOVEABLE,
							 strlen(password) + 1);
						if (hGlob) {
							memcpy(GlobalLock
							       (hGlob),
							       password,
							       strlen(password)
							       + 1);
							GlobalUnlock(hGlob);
							SetClipboardData
								(CF_TEXT,
								 hGlob);
						}
						CloseClipboard();

						free_password(password);	// implement this

						// Show toast (non-blocking)
						show_toast(hwnd,
							   "Password copied to clipboard");

						// Start timer
						SetTimer(hwnd,
							 CLIPBOARD_CLEAR_TIMER_ID,
							 CLIPBOARD_CLEAR_TIMEOUT_MS,
							 NULL);

					}
				}
				break;
			}
		}
		break;

	case WM_DESTROY:
		free_entry_list(entry_cache);
		db_close(db);
		db = NULL;
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		if (wParam == CLIPBOARD_CLEAR_TIMER_ID) {
			if (OpenClipboard(NULL)) {
				HANDLE hData = GetClipboardData(CF_TEXT);
				if (hData) {
					char *clipText = GlobalLock(hData);
					if (clipText && clipboard_backup
					    && strcmp(clipText,
						      clipboard_backup) == 0) {
						EmptyClipboard();	// Clear it
						show_toast(hwnd,
							   "Clipboard auto-cleared");
					}
					GlobalUnlock(hData);
				}
				CloseClipboard();
			}

			KillTimer(hwnd, CLIPBOARD_CLEAR_TIMER_ID);
			if (clipboard_backup) {
				// Zero out the clipboard backup before freeing
				SecureZeroMemory(clipboard_backup,
						 strlen(clipboard_backup));
				free(clipboard_backup);
				clipboard_backup = NULL;
			}
		}
		break;

	default:
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

void run_gui(void)
{
	HINSTANCE hInstance = GetModuleHandleA(NULL);

	WNDCLASSA wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);

	RegisterClassA(&wc);

	INITCOMMONCONTROLSEX icex =
		{ sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
	InitCommonControlsEx(&icex);

	HWND hwnd = CreateWindowExA(0,
				    WINDOW_CLASS_NAME,
				    "APM - Advanced Password Manager 1.0",
				    WS_OVERLAPPEDWINDOW,
				    CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
				    NULL, NULL, hInstance, NULL);

	if (!hwnd) {
		MessageBoxA(NULL, "Failed to create window", "Error",
			    MB_OK | MB_ICONERROR);
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
