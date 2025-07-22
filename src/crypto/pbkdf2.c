#include "pbkdf2.h"
#include "hmac.h"
#include <string.h>

static void int_to_big_endian(uint32_t val, uint8_t out[4])
{
	out[0] = (val >> 24) & 0xFF;
	out[1] = (val >> 16) & 0xFF;
	out[2] = (val >> 8) & 0xFF;
	out[3] = val & 0xFF;
}

void pbkdf2_hmac_sha256(const uint8_t * password, size_t password_len,
			const uint8_t * salt, size_t salt_len,
			uint32_t iterations, uint8_t * output, size_t dk_len)
{
	uint32_t block_count =
		(dk_len + HMAC_SHA256_DIGEST_SIZE -
		 1) / HMAC_SHA256_DIGEST_SIZE;
	uint8_t u[HMAC_SHA256_DIGEST_SIZE];
	uint8_t t[HMAC_SHA256_DIGEST_SIZE];
	uint8_t salt_block[128];	
	uint8_t block_index[4];

	for (uint32_t i = 1; i <= block_count; i++) {
		
		memcpy(salt_block, salt, salt_len);
		int_to_big_endian(i, block_index);
		memcpy(salt_block + salt_len, block_index, 4);

		hmac_sha256(password, password_len, salt_block, salt_len + 4,
			    u);
		memcpy(t, u, HMAC_SHA256_DIGEST_SIZE);

		for (uint32_t j = 1; j < iterations; j++) {
			hmac_sha256(password, password_len, u,
				    HMAC_SHA256_DIGEST_SIZE, u);
			for (int k = 0; k < HMAC_SHA256_DIGEST_SIZE; k++) {
				t[k] ^= u[k];
			}
		}

		size_t offset = (i - 1) * HMAC_SHA256_DIGEST_SIZE;
		size_t remaining = dk_len - offset;
		memcpy(output + offset, t,
		       remaining >
		       HMAC_SHA256_DIGEST_SIZE ? HMAC_SHA256_DIGEST_SIZE :
		       remaining);
	}
}

