#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../src/pbkdf2.h"

#define DKLEN 32

void bytes_to_hex(const uint8_t *bytes, size_t len, char *hexstr) {
    static const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        hexstr[i * 2] = hex[(bytes[i] >> 4) & 0xF];
        hexstr[i * 2 + 1] = hex[bytes[i] & 0xF];
    }
    hexstr[len * 2] = '\0';
}

void test_pbkdf2_case(const char *pass, const char *salt_str, uint32_t iterations, size_t dklen, const char *expected_hex) {
    uint8_t dk[DKLEN];
    char dk_hex[DKLEN * 2 + 1];

    pbkdf2_hmac_sha256((const uint8_t *)pass, strlen(pass),
                       (const uint8_t *)salt_str, strlen(salt_str),
                       iterations, dk, dklen);

    bytes_to_hex(dk, dklen, dk_hex);

    printf("Password: \"%s\"\nSalt: \"%s\"\nIterations: %u\n", pass, salt_str, iterations);
    printf("Derived Key: %s\n", dk_hex);
    printf("Expected    : %s\n", expected_hex);

    if (strncmp(dk_hex, expected_hex, dklen * 2) == 0) {
        printf("Passed\n\n");
    } else {
        printf("Failed\n\n");
    }
}

int main(void) {
    test_pbkdf2_case(
        "password",
        "salt",
        1,
        32,
        "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b"
    );

    test_pbkdf2_case(
        "password",
        "salt",
        2,
        32,
        "ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43"
    );

    test_pbkdf2_case(
        "password",
        "salt",
        4096,
        32,
        "c5e478d59288c841aa530db6845c4c8d962893a001ce4e11a4963873aa98134a"
    );

    return 0;
}
