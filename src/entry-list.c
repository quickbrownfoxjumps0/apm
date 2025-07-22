#include "entry-list.h"
#include "entry-dialog.h"
#include "db.h"
#include "toast.h"
#include <windows.h>
#include <commctrl.h>

EntryList *entry_cache = NULL;

void on_add_entry(HWND hwndParent)
{
	EntryData *data = calloc(1, sizeof(EntryData));
	data->hwndParent = hwndParent;
	data->is_edit = 0;
	ShowEntryDialog(hwndParent, data);
}

void on_edit_entry(HWND hwndParent, int selectedIndex)
{
	if (!entry_cache || selectedIndex < 0
	    || (size_t) selectedIndex >= entry_cache->count)
		return;

	Entry *e = entry_cache->entries[selectedIndex];
	EntryData *data = calloc(1, sizeof(EntryData));
	strncpy(data->service, e->service, sizeof(data->service) - 1);
	strncpy(data->username, e->username, sizeof(data->username) - 1);
	strncpy(data->notes, e->notes, sizeof(data->notes) - 1);
	data->entry_id = e->id;
	data->hwndParent = hwndParent;
	data->is_edit = 1;

	ShowEntryDialog(hwndParent, data);
}

void populate_entry_list(HWND hwndList)
{
	if (entry_cache) {
		free_entry_list(entry_cache);
		entry_cache = NULL;
	}

	if (!db)
		return;

	entry_cache = db_get_all_entries(db);
	if (entry_cache) {
		ListView_SetItemCount(hwndList, entry_cache->count);
	} else {
		ListView_SetItemCount(hwndList, 0);
	}
}
