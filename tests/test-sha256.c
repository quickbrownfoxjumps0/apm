#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../src/sha256.h"

#define ASSERT_EQ_HEX(actual, expected) do {                             \
    char actual_hex[SHA256_BLOCK_SIZE * 2 + 1];                          \
    bytes_to_hex(actual, SHA256_BLOCK_SIZE, actual_hex);                \
    if (strcmp(actual_hex, expected) != 0) {                             \
        printf("Failed: %s:%d\n", __FILE__, __LINE__);                \
        printf("Expected: %s\n", expected);                              \
        printf("Actual  : %s\n", actual_hex);                            \
        failures++;                                                     \
    } else {                                                             \
        printf("Passed\n");                                           \
        passes++;                                                        \
    }                                                                    \
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

void test_sha256_case(const char *input, const char *expected_hex)
{
	SHA256Context ctx;
	uint8_t hash[SHA256_BLOCK_SIZE];
	sha256_init(&ctx);
	sha256_update(&ctx, (const uint8_t *) input, strlen(input));
	sha256_final(&ctx, hash);
	ASSERT_EQ_HEX(hash, expected_hex);
}

int main(void)
{
	printf("Running SHA-256 unit tests...\n");

	test_sha256_case("",
			 "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	test_sha256_case("abc",
			 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
	test_sha256_case("hello world",
			 "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9");
	test_sha256_case("quickbrownfox",
			 "464cb013effbc2f6b98e8a136f3e5ac4f8a266f38dd5ea6292314d9b16b28e73");

	printf("\nTests passed: %d\n", passes);
	printf("Tests failed: %d\n", failures);
	return failures == 0 ? 0 : 1;
}
