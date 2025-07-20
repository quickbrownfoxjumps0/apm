#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../src/hmac.h"

#define ASSERT_HEX_EQ(actual, expected_hex) do {                          \
    char hex[HMAC_SHA256_DIGEST_SIZE * 2 + 1];                            \
    bytes_to_hex(actual, HMAC_SHA256_DIGEST_SIZE, hex);                  \
    if (strcmp(hex, expected_hex) != 0) {                                 \
        printf("FAILED: %s:%d\n", __FILE__, __LINE__);                 \
        printf("Expected: %s\n", expected_hex);                           \
        printf("Actual  : %s\n", hex);                                    \
        failures++;                                                      \
    } else {                                                              \
        printf("Passed\n");                                            \
        passes++;                                                         \
    }                                                                     \
} while (0)

void bytes_to_hex(const uint8_t *bytes, size_t len, char *hexstr)
{
	static const char hex[] = "0123456789abcdef";
	for (size_t i = 0; i < len; i++) {
		hexstr[i * 2] = hex[(bytes[i] >> 4) & 0xF];
		hexstr[i * 2 + 1] = hex[bytes[i] & 0xF];
	}
	hexstr[len * 2] = '\0';
}

int failures = 0;
int passes = 0;

void test_hmac_case(const char *key, const char *message,
		    const char *expected_hex)
{
	uint8_t mac[HMAC_SHA256_DIGEST_SIZE];
	hmac_sha256((const uint8_t *) key, strlen(key),
		    (const uint8_t *) message, strlen(message), mac);
	ASSERT_HEX_EQ(mac, expected_hex);
}

int main(void)
{
	printf("Running HMAC-SHA256 tests...\n");

	test_hmac_case("key",
		       "The quick brown fox jumps over the lazy dog",
		       "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");

	printf("\nTests passed: %d\n", passes);
	printf("Tests failed: %d\n", failures);
	return failures == 0 ? 0 : 1;
}
