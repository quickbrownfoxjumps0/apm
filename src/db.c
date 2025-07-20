#include "pbkdf2.h"
#define SQLITE_HAS_CODEC 1
#include <sqlcipher/sqlite3.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define DERIVED_KEY_LEN 32 /* 256 bit key */

int open_encrypted_db(const char *filename,
		      const char *password,
		      const uint8_t * salt, size_t salt_len,
		      int iterations, sqlite3 ** out_db)
{
	uint8_t key[DERIVED_KEY_LEN];
	pbkdf2_hmac_sha256((const uint8_t *) password, strlen(password),
			   salt, salt_len, iterations, key, DERIVED_KEY_LEN);

	sqlite3 *db = NULL;
	int rc = sqlite3_open(filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Could not open database: %s\n",
			sqlite3_errmsg(db));
		return rc;
	}

	/* Key for db */
	rc = sqlite3_key(db, key, DERIVED_KEY_LEN);
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
