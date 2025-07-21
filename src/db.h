#ifndef DB_H
#define DB_H

#define SQLITE_HAS_CODEC 1
#include <sqlcipher/sqlite3.h>
#include <stddef.h>
#include <stdint.h>

#define DERIVED_KEY_LEN 32 /* 256 bit key */
#define PBKDF2_ITERATIONS 300000 /* number of iterations */

#define SALT_EXT ".salt"
#define SALT_SIZE 16

typedef struct {
	int id;
	char *service;
	char *username;
	char *password;
	char *notes;
} Entry;

typedef struct {
	Entry **entries;
	size_t count;
} EntryList;

void free_entry(Entry *entry);
void free_entry_list(EntryList *list);

int open_encrypted_db(const char *filename,
                      const char *password,
                      sqlite3 **out_db);

int db_init_schema(sqlite3 *db);

int db_add_entry(sqlite3 *db,
                 const char *service,
                 const char *username,
                 const char *password,
                 const char *notes);

int db_update_entry(sqlite3 *db,
                    int id,
                    const char *service,
                    const char *username,
                    char *password,
                    const char *notes);

int db_delete_entry(sqlite3 *db, int id);

EntryList* db_get_all_entries(sqlite3 *db);

void db_close(sqlite3 *db);

#endif
