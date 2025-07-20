#include "db.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	const char *filename = "vault.db";
	const char *password = "correct horse battery staple";
	uint8_t salt[16] = {
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f
	};
	int iterations = 200000;

	sqlite3 *db;
	int rc = open_encrypted_db(filename, password, salt, sizeof(salt),
				   iterations, &db);

	if (rc == SQLITE_OK) {
		printf("Successfully opened and unlocked database: %s\n",
		       filename);
		sqlite3_close(db);
	} else {
		printf("Failed to open database: %d\n", rc);
	}

	return rc == SQLITE_OK ? 0 : 1;
}
