#ifndef ENTROPY_H
#define ENTROPY_H

#include <stddef.h>



void generate_random_password(int min_bits, char *out, size_t out_len);



void generate_diceware_password(int min_bits, char *out, size_t out_len);

#endif				

