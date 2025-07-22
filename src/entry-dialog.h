#ifndef ENTRY_DIALOG_H
#define ENTRY_DIALOG_H

#include <windows.h>

typedef struct {
	int is_edit;
	int entry_id;
	char service[256];
	char username[256];
	char password[256];
	char notes[512];
	HWND hwndParent;
} EntryData;

void ShowEntryDialog(HWND hwndParent, EntryData * data);
LRESULT CALLBACK EntryDialogProc(HWND hwnd, UINT msg, WPARAM wParam,
				 LPARAM lParam);

#endif				// ENTRY_DIALOG_H
