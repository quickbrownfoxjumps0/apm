#include "hmac.h"
#include "sha256.h"
#include <string.h>
#include <stdint.h>

void hmac_sha256(const uint8_t * key, size_t key_len,
		 const uint8_t * message, size_t message_len,
		 uint8_t * mac_out)
{
	uint8_t key_block[HMAC_SHA256_BLOCK_SIZE];
	uint8_t o_key_pad[HMAC_SHA256_BLOCK_SIZE];
	uint8_t i_key_pad[HMAC_SHA256_BLOCK_SIZE];
	uint8_t inner_hash[SHA256_BLOCK_SIZE];

	
	if (key_len > HMAC_SHA256_BLOCK_SIZE) {
		SHA256Context ctx;
		sha256_init(&ctx);
		sha256_update(&ctx, key, key_len);
		sha256_final(&ctx, key_block);
		memset(key_block + SHA256_BLOCK_SIZE, 0,
		       HMAC_SHA256_BLOCK_SIZE - SHA256_BLOCK_SIZE);
	} else {
		memcpy(key_block, key, key_len);
		memset(key_block + key_len, 0,
		       HMAC_SHA256_BLOCK_SIZE - key_len);
	}

	
	for (int i = 0; i < HMAC_SHA256_BLOCK_SIZE; ++i) {
		o_key_pad[i] = key_block[i] ^ 0x5c;
		i_key_pad[i] = key_block[i] ^ 0x36;
	}

	
	SHA256Context inner_ctx;
	sha256_init(&inner_ctx);
	sha256_update(&inner_ctx, i_key_pad, HMAC_SHA256_BLOCK_SIZE);
	sha256_update(&inner_ctx, message, message_len);
	sha256_final(&inner_ctx, inner_hash);

	
	SHA256Context outer_ctx;
	sha256_init(&outer_ctx);
	sha256_update(&outer_ctx, o_key_pad, HMAC_SHA256_BLOCK_SIZE);
	sha256_update(&outer_ctx, inner_hash, SHA256_BLOCK_SIZE);
	sha256_final(&outer_ctx, mac_out);
}

