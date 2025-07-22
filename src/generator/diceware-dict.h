#pragma once

#include <stddef.h>

#define DICEWARE_DICT_SIZE 7776

extern const char diceware_words_data[];
extern const size_t diceware_words_offsets[];

static inline const char *diceware_dict(size_t idx)
{
	if (idx >= DICEWARE_DICT_SIZE)
		return NULL;
	return diceware_words_data + diceware_words_offsets[idx];
}

