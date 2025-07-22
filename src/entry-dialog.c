#include "entry-dialog.h"
#include "entry-list.h"
#include "db.h"
#include "utils.h"
#include "resource.h"
#include <windows.h>
#include <string.h>

// Implement ShowEntryDialog to create the dialog window
void ShowEntryDialog(HWND hwndParent, EntryData * data)
{
	static const char className[] = "EntryDialogClass";

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = EntryDialogProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = className;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);

	RegisterClass(&wc);

	HWND hwndDialog = CreateWindowEx(WS_EX_DLGMODALFRAME,
					 className,
					 data->is_edit ? "Edit Entry" :
					 "Add Entry",
					 WS_OVERLAPPED | WS_CAPTION |
					 WS_SYSMENU,
					 CW_USEDEFAULT, CW_USEDEFAULT, 390,
					 280,
					 hwndParent, NULL,
					 GetModuleHandle(NULL), data);

	ShowWindow(hwndDialog, SW_SHOW);
	UpdateWindow(hwndDialog);
}

LRESULT CALLBACK EntryDialogProc(HWND hwnd, UINT msg, WPARAM wParam,
				 LPARAM lParam)
{
	static EntryData *pData;
	static HWND hEditService, hEditUsername, hEditPassword, hEditNotes;

	switch (msg) {
	case WM_CREATE:{
			pData = (EntryData *) ((LPCREATESTRUCT)
					       lParam)->lpCreateParams;

			int x = 20, y = 20, w = 220, h = 20, gap = 30;

			CreateWindow("STATIC", "Service:",
				     WS_CHILD | WS_VISIBLE, x, y, 80, h, hwnd,
				     NULL, NULL, NULL);
			hEditService =
				CreateWindow("EDIT", "",
					     WS_CHILD | WS_VISIBLE | WS_BORDER,
					     x + 90, y, w, h, hwnd,
					     (HMENU) ID_EDIT_SERVICE, NULL,
					     NULL);

			y += gap;
			CreateWindow("STATIC", "Username:",
				     WS_CHILD | WS_VISIBLE, x, y, 80, h, hwnd,
				     NULL, NULL, NULL);
			hEditUsername =
				CreateWindow("EDIT", "",
					     WS_CHILD | WS_VISIBLE | WS_BORDER,
					     x + 90, y, w, h, hwnd,
					     (HMENU) ID_EDIT_USERNAME, NULL,
					     NULL);

			y += gap;
			CreateWindow("STATIC", "Password:",
				     WS_CHILD | WS_VISIBLE, x, y, 80, h, hwnd,
				     NULL, NULL, NULL);
			hEditPassword =
				CreateWindow("EDIT", "",
					     WS_CHILD | WS_VISIBLE | WS_BORDER
					     | ES_PASSWORD, x + 90, y, w, h,
					     hwnd, (HMENU) ID_EDIT_PASSWORD,
					     NULL, NULL);

			y += gap;
			CreateWindow("STATIC", "Notes:", WS_CHILD | WS_VISIBLE,
				     x, y, 80, h, hwnd, NULL, NULL, NULL);
			hEditNotes =
				CreateWindow("EDIT", "",
					     WS_CHILD | WS_VISIBLE | WS_BORDER
					     | WS_VSCROLL,
					     x + 90, y, w, 80,
					     hwnd, (HMENU) ID_EDIT_NOTES, NULL,
					     NULL);

			y += 100;
			CreateWindow("BUTTON", "OK",
				     WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
				     x, y, 100, 25, hwnd, (HMENU) ID_BUTTON_OK,
				     NULL, NULL);
			CreateWindow("BUTTON", "Cancel", WS_CHILD | WS_VISIBLE,
				     x + 120, y, 100, 25, hwnd,
				     (HMENU) ID_BUTTON_CANCEL, NULL, NULL);

			// Now populate fields
			SetWindowText(hEditService, pData->service);
			SetWindowText(hEditUsername, pData->username);
			SetWindowText(hEditNotes, pData->notes);


			return 0;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_BUTTON_OK:{
				GetWindowText(hEditService, pData->service,
					      sizeof(pData->service));
				GetWindowText(hEditUsername, pData->username,
					      sizeof(pData->username));
				GetWindowText(hEditPassword, pData->password,
					      sizeof(pData->password));
				SetWindowText(hEditPassword, "");	// Clears UI content
				SendMessage(hEditPassword, EM_SETSEL, 0, -1);
				SendMessage(hEditPassword, EM_REPLACESEL, TRUE, (LPARAM) "");	// Further overwrite attempt

				GetWindowText(hEditNotes, pData->notes,
					      sizeof(pData->notes));

				if (strlen(pData->service) == 0
				    || strlen(pData->password) == 0) {
					MessageBox(hwnd,
						   "Service and Password are required.",
						   "Validation Error",
						   MB_ICONWARNING);
					break;
				}

				if (!is_ascii(pData->service)
				    || !is_ascii(pData->username)
				    || !is_ascii(pData->password)
				    || !is_ascii(pData->notes)) {
					MessageBox(hwnd,
						   "All fields must contain ASCII characters only.",
						   "Validation Error",
						   MB_ICONWARNING);
					break;
				}

				if (strlen(pData->service) >=
				    sizeof(pData->service)
				    || strlen(pData->username) >=
				    sizeof(pData->username)
				    || strlen(pData->password) >=
				    sizeof(pData->password)
				    || strlen(pData->notes) >=
				    sizeof(pData->notes)) {
					MessageBox(hwnd,
						   "One or more fields exceed allowed length.",
						   "Validation Error",
						   MB_ICONWARNING);
					break;
				}

				int result = pData->is_edit
					? db_update_entry(db, pData->entry_id,
							  pData->service,
							  pData->username,
							  pData->password,
							  pData->notes)
					: db_add_entry(db, pData->service,
						       pData->username,
						       pData->password,
						       pData->notes);

				if (result != SQLITE_OK) {
					MessageBox(hwnd,
						   pData->is_edit ?
						   "Failed to update entry." :
						   "Failed to add entry.",
						   "Database Error",
						   MB_ICONERROR);
					break;
				}

				SecureZeroMemory(pData->password,
						 sizeof(pData->password));
				populate_entry_list(GetDlgItem
						    (pData->hwndParent,
						     ID_ENTRY_LIST));
				DestroyWindow(hwnd);
				break;
			}

		case ID_BUTTON_CANCEL:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		SetWindowText(hEditPassword, "");
		if (pData) {
			SecureZeroMemory(pData->password,
					 sizeof(pData->password));
			free(pData);
		}
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);

}
