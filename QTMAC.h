#include <stdint.h>
#include "tweakableBC.h"
#include <time.h> 
#include <iomanip>
#include <stdio.h> 
#include <intrin.h>
#include <iostream>

#define u8 uint8_t
#define XOR(a, b) for(int _i=0;_i<16;_i++) a[_i] ^= b[_i];

void qTMAC_128_256(u8(*pt)[16], size_t pt_bits_num, u8 key[16], u8 N[8], u8 tag[], size_t tau, u8 cst0[16],u8 cst1[16]);
void print_u8(u8* in, int size);