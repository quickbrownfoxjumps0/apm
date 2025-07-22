#include "entropy.h"
#include "diceware-dict.h"
#include <windows.h>
#include <bcrypt.h>		
#include <wincrypt.h>		
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define CHARSET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*(){}[]"
#define CHARSET_LEN (sizeof(CHARSET) - 1)


static int get_secure_random_bytes(unsigned char *buffer, size_t length)
{
	static int use_bcrypt = -1;

	if (use_bcrypt == -1) {
		HMODULE hBcrypt = LoadLibraryA("bcrypt.dll");
		if (hBcrypt && GetProcAddress(hBcrypt, "BCryptGenRandom")) {
			use_bcrypt = 1;
		} else {
			use_bcrypt = 0;
		}
		if (hBcrypt) {
			FreeLibrary(hBcrypt);
		}
	}

	if (use_bcrypt) {
		NTSTATUS status =
			BCryptGenRandom(NULL, buffer, (ULONG) length,
					BCRYPT_USE_SYSTEM_PREFERRED_RNG);
		return (status == 0);
	} else {
		HCRYPTPROV hProv = 0;
		if (!CryptAcquireContextA
		    (&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
			return 0;

		BOOL result = CryptGenRandom(hProv, (DWORD) length, buffer);
		CryptReleaseContext(hProv, 0);
		return result;
	}
}

static int calculate_entropy_per_char(void)
{
	return (int) log2(CHARSET_LEN);
}

static int calculate_entropy_per_word(void)
{
	return (int) log2(DICEWARE_DICT_SIZE);
}

void generate_random_password(int min_bits, char *out, size_t out_len)
{
	int entropy_per_char = calculate_entropy_per_char();
	int len = (min_bits + entropy_per_char - 1) / entropy_per_char;
	if ((size_t) len >= out_len)
		len = (int) out_len - 1;

	unsigned char *random_bytes = (unsigned char *) malloc(len);
	if (!random_bytes) {
		strncpy(out, "MEM_ERR", out_len);
		return;
	}

	if (!get_secure_random_bytes(random_bytes, len)) {
		strncpy(out, "RNG_ERR", out_len);
		free(random_bytes);
		return;
	}

	for (int i = 0; i < len; ++i) {
		out[i] = CHARSET[random_bytes[i] % CHARSET_LEN];
	}
	out[len] = '\0';
	free(random_bytes);
}

void generate_diceware_password(int min_bits, char *out, size_t out_len)
{
	int entropy_per_word = calculate_entropy_per_word();
	int word_count = (min_bits + entropy_per_word - 1) / entropy_per_word;

	
	unsigned char *random_bytes = (unsigned char *) malloc(word_count * 2);
	if (!random_bytes) {
		strncpy(out, "MEM_ERR", out_len);
		return;
	}

	if (!get_secure_random_bytes(random_bytes, word_count * 2)) {
		strncpy(out, "RNG_ERR", out_len);
		free(random_bytes);
		return;
	}

	out[0] = '\0';
	size_t written = 0;

	for (int i = 0; i < word_count; ++i) {
		int idx =
			((random_bytes[2 * i] << 8) | random_bytes[2 * i + 1])
			% DICEWARE_DICT_SIZE;
		const char *word = diceware_dict(idx);

		size_t remaining = out_len - written;
		int needed =
			snprintf(out + written, remaining, "%s%s", word,
				 (i < word_count - 1) ? "-" : "");

		if (needed < 0 || (size_t) needed >= remaining) {
			
			strncpy(out + written, "...", remaining - 1);
			out[out_len - 1] = '\0';
			break;
		}
		written += needed;
	}

	free(random_bytes);
}

