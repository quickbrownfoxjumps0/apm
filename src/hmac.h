#ifndef HMAC_H
#define HMAC_H

#include <stdint.h>
#include <stddef.h>

#define HMAC_SHA256_DIGEST_SIZE 32
#define HMAC_SHA256_BLOCK_SIZE 64

void hmac_sha256(const uint8_t *key, size_t key_len,
		 const uint8_t *message, size_t message_len,
		 uint8_t *mac_out);

#endif
