#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>

void get_input(const char *prompt, char *buffer, size_t size, int hide) {
	printf("%s", prompt);

	if (hide) {
		DWORD mode;
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		GetConsoleMode(hStdin, &mode);
		SetConsoleMode(hStdin, mode & ~(ENABLE_ECHO_INPUT));
		fgets(buffer, size, stdin);
		SetConsoleMode(hStdin, mode);  // restore echo
		printf("\n");
	} else {
		fgets(buffer, size, stdin);
	}

	// Strip newline
	size_t len = strlen(buffer);
	if (len > 0 && buffer[len - 1] == '\n')
		buffer[len - 1] = '\0';
}

void print_menu() {
	printf("\n=== Password Vault ===\n");
	printf("1. Add Entry\n");
	printf("2. List Entries\n");
	printf("3. Delete Entry\n");
	printf("4. Exit\n");
	printf("Choice: ");
}

int main(void) {
	const char *filename = "vault.db";
	char master_password[128];
	get_input("Enter master password: ", master_password, sizeof(master_password), 1);

	sqlite3 *db;
	int rc = open_encrypted_db(filename, master_password, &db);
	SecureZeroMemory(master_password, sizeof(master_password));

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to open encrypted DB\n");
		return 1;
	}

	if (db_init_schema(db) != SQLITE_OK) {
		fprintf(stderr, "Failed to initialize schema.\n");
		db_close(db);
		return 1;
	}

	while (1) {
		print_menu();
		int choice;
		scanf("%d", &choice);
		getchar(); // consume newline

		if (choice == 1) {
			char service[128], username[128], password[128], notes[256];

			get_input("service: ", service, sizeof(service), 0);
			get_input("Username: ", username, sizeof(username), 0);
			get_input("Password: ", password, sizeof(password), 1);
			get_input("Notes: ", notes, sizeof(notes), 0);

			db_add_entry(db, service, username, password, notes);
			SecureZeroMemory(password, sizeof(password));

		} else if (choice == 2) {
			EntryList *entries = db_get_all_entries(db);
			if (!entries || entries->count == 0) {
				printf("No entries found.\n");
			} else {
				for (size_t i = 0; i < entries->count; ++i) {
					Entry *e = entries->entries[i];
					printf("[%d] %s\n", e->id, e->service);
					printf("  Username: %s\n", e->username);
					printf("  Password: %s\n", e->password);
					printf("  Notes: %s\n", e->notes ? e->notes : "(none)");
				}
			}
			free_entry_list(entries);

		} else if (choice == 3) {
			int del_id;
			printf("Enter entry ID to delete: ");
			scanf("%d", &del_id);
			getchar();
			db_delete_entry(db, del_id);
			printf("Deleted entry ID %d\n", del_id);

		} else if (choice == 4) {
			break;

		} else {
			printf("Invalid choice.\n");
		}
	}

	db_close(db);
	return 0;
}

