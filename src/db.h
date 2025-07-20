#ifndef DB_H
#define DB_H

#define SQLITE_HAS_CODEC 1
#include <sqlcipher/sqlite3.h>
#include <stddef.h>
#include <stdint.h>

int open_encrypted_db(const char *filename,
		      const char *password,
		      const uint8_t * salt, size_t salt_len,
		      int iterations, sqlite3 **out_db);

#endif
