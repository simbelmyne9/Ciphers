
#include <stdint.h>
#include "tweakableBC.h"

#define u8 uint8_t
#define h(x) (H[(x & 0b11110000) >> 4] << 4) ^ (H[x & 0b00001111])
#define XOR(a, b) for(int _i=0;_i<16;_i++) a[_i] ^= b[_i];

void sTMAC_128_256(u8(*pt)[16], size_t pt_bits_num, u8 key[16], u8 N[8], u8 tag[], size_t tau, bool print_model=false);
void print_u8(u8* in, int size);