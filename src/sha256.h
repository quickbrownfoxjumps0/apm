#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include <stddef.h>

#define SHA256_BLOCK_SIZE 32

typedef struct {
	uint32_t state[8];
	uint64_t bitlen;
	uint8_t data[64];
	uint32_t datalen;
} SHA256Context;


void sha256_init(SHA256Context * ctx);
void sha256_update(SHA256Context * ctx, const uint8_t * data, size_t len);
void sha256_final(SHA256Context * ctx, uint8_t * hash);


#endif
