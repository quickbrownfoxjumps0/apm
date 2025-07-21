#include "pbkdf2.h"
#include "db.h"
#include <windows.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

static char *get_salt_filename(const char *db_filename) {
	size_t len = strlen(db_filename) + strlen(SALT_EXT) + 1;
	char *salt_filename = malloc(len);
	if (!salt_filename) return NULL;
	snprintf(salt_filename, len, "%s%s", db_filename, SALT_EXT);
	return salt_filename;
}

static int load_or_create_salt(const char *salt_filename, uint8_t *salt_out, size_t salt_len) {
	FILE *fp = fopen(salt_filename, "rb");

	if (fp) {
		size_t read = fread(salt_out, 1, salt_len, fp);
		fclose(fp);
		return (read == salt_len) ? 0 : -1;
	}

	// Generate salt
	HCRYPTPROV hProv = 0;
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		return -1;

	if (!CryptGenRandom(hProv, (DWORD)salt_len, salt_out)) {
		CryptReleaseContext(hProv, 0);
		return -1;
	}
	CryptReleaseContext(hProv, 0);

	// Save to file
	fp = fopen(salt_filename, "wb");
	if (!fp) return -1;

	if (fwrite(salt_out, 1, salt_len, fp) != salt_len) {
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int open_encrypted_db(const char *filename,
		const char *password,
		sqlite3 **out_db)
{
	
	uint8_t salt[SALT_SIZE];
	char *salt_file = get_salt_filename(filename);
	if (!salt_file || load_or_create_salt(salt_file, salt, SALT_SIZE) != 0) {
		fprintf(stderr, "Failed to load or generate salt for %s\n", filename);
		free(salt_file);
		return SQLITE_ERROR;
	}
	free(salt_file);

	uint8_t key[DERIVED_KEY_LEN];
	pbkdf2_hmac_sha256((const uint8_t *)password, strlen(password),
			salt, SALT_SIZE, PBKDF2_ITERATIONS, key, DERIVED_KEY_LEN);
	
	// Print derived key in hex
	/*
	printf("Derived key: ");
	for (int i = 0; i < DERIVED_KEY_LEN; ++i)
		printf("%02x", key[i]);
	printf("\n");
	*/

	sqlite3 *db = NULL;
	int rc = sqlite3_open(filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Could not open database: %s\n",
				sqlite3_errmsg(db));
		return rc;
	}

	/* Key for db */
	rc = sqlite3_key(db, key, DERIVED_KEY_LEN); 

	SecureZeroMemory(key, DERIVED_KEY_LEN);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "sqlite3_key failed: %d\n", rc);
		sqlite3_close(db);
		return rc;
	}

	/* Test query */
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(db, "SELECT count(*) FROM sqlite_master;", -1,
			&stmt, NULL);
	if (rc != SQLITE_OK) {
		fprintf(stderr,
				"Failed to unlock database (bad password or corruption): %s\n",
				sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	sqlite3_finalize(stmt);

	*out_db = db;
	return SQLITE_OK;
} 

int db_init_schema(sqlite3 *db)
{
	const char *sql =
		"CREATE TABLE IF NOT EXISTS passwords ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"service TEXT NOT NULL,"
		"username TEXT NOT NULL,"
		"password TEXT NOT NULL,"
		"notes TEXT"
		");";

	char *errmsg = NULL;
	int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to create schema: %s\n", errmsg);
		sqlite3_free(errmsg);
		return rc;
	}
	return SQLITE_OK;
}

int db_add_entry(sqlite3 *db,
		const char *service,
		const char *username,
		const char *password,
		const char *notes)
{
	const char *sql = "INSERT INTO passwords (service, username, password, notes) VALUES (?, ?, ?, ?);";
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare insert statement: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	sqlite3_bind_text(stmt, 1, service, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, username, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, password, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 4, notes, -1, SQLITE_TRANSIENT);

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute insert statement: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	if (password != NULL) {
		SecureZeroMemory((void *)password, strlen(password));
	}

	return SQLITE_OK;
}

int db_update_entry(sqlite3 *db,
		int id,
		const char *service,
		const char *username,
		char *password,
		const char *notes)
{
	int rc;
	sqlite3_stmt *stmt;
	const char *sql = "UPDATE passwords SET service=?, username=?, password=?, notes=? WHERE id=?;";

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		return rc;
	}

	sqlite3_bind_text(stmt, 1, service, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, password, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 4, notes, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 5, id);

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	if (password) {
		SecureZeroMemory(password, strlen(password));
	}

	return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

int db_delete_entry(sqlite3 *db, int id)
{
	sqlite3_stmt *stmt;
	const char *sql = "DELETE FROM passwords WHERE id = ?;";
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) return rc;

	sqlite3_bind_int(stmt, 1, id);
	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

EntryList* db_get_all_entries(sqlite3 *db)
{
	const char *sql = "SELECT id, service, username, password, notes FROM passwords;";
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) return NULL;

	EntryList *list = calloc(1, sizeof(EntryList));
	if (!list) {
		sqlite3_finalize(stmt);
		return NULL;
	}

	size_t capacity = 10;
	list->entries = malloc(sizeof(Entry*) * capacity);
	if (!list->entries) {
		free(list);
		sqlite3_finalize(stmt);
		return NULL;
	}

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		if (list->count == capacity) {
			capacity *= 2;
			Entry **new_entries = realloc(list->entries, capacity * sizeof(Entry*));
			if (!new_entries) {
				// cleanup and exit on failure
				free_entry_list(list);
				sqlite3_finalize(stmt);
				return NULL;
			}
			list->entries = new_entries;
		}

		Entry *entry = calloc(1, sizeof(Entry));
		if (!entry) {
			free_entry_list(list);
			sqlite3_finalize(stmt);
			return NULL;
		}

		entry->id = sqlite3_column_int(stmt, 0);
		entry->service = _strdup((const char*)sqlite3_column_text(stmt, 1));
		entry->username = _strdup((const char*)sqlite3_column_text(stmt, 2));

		const unsigned char *pwd_text = sqlite3_column_text(stmt, 3);
		if (pwd_text) {
			size_t len = strlen((const char*)pwd_text);
			entry->password = malloc(len + 1);
			if (entry->password) {
				memcpy(entry->password, pwd_text, len);
				entry->password[len] = '\0';
			}
		}

		entry->notes = _strdup((const char*)sqlite3_column_text(stmt, 4));

		list->entries[list->count++] = entry;
	}

	sqlite3_finalize(stmt);

	if (rc != SQLITE_DONE) {
		free_entry_list(list);
		return NULL;
	}

	return list;
}

void free_entry(Entry *entry) 
{
	if (!entry) return;
	if (entry->password) SecureZeroMemory(entry->password, strlen(entry->password));
	free(entry->service);
	free(entry->username);
	if (entry->password) free(entry->password);
	free(entry->notes);
	free(entry);
}

void free_entry_list(EntryList *list) 
{
	if (!list) return;
	for (size_t i = 0; i < list->count; i++) {
		free_entry(list->entries[i]);
	}
	free(list->entries);
	free(list);
}

void db_close(sqlite3 *db)
{
	if (db) {
		sqlite3_close(db);
	}
}
