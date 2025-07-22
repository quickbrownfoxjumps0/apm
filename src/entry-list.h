#ifndef ENTRY_LIST_H
#define ENTRY_LIST_H

#include "db.h"
#include <windows.h>

extern EntryList *entry_cache;

void on_add_entry(HWND hwndParent);
void on_edit_entry(HWND hwndParent, int selectedIndex);
void populate_entry_list(HWND hwndList);

#endif				// ENTRY_LIST_H
