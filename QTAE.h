#include <iostream>
#include "tweakableBC.h"
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <malloc.h>
#include <intrin.h>
#include <iomanip>

typedef unsigned int u32;
typedef unsigned char u8;

void QTAE_associate(u8 (*A)[16], size_t A_bit_len, u8* K, u8* L, u8* N,  u8* Cst0, u8* Cst1, u8* S_a, u8* T_a);
void QTAE_encrypt(u8 (*M)[16], size_t M_bit_len, u8* K, u8* N, u8* S_a, u8* T_a, u8* TAG_out, int tau, u8 C[16]);
bool QTAE_decrypt(u8 (*C)[16], size_t C_bit_len, u8* K, u8* N, u8* S_a, u8* T_a, u8* TAG_out, int tau, u8 M[16], u8* TAG_in);
void print_u8(u8* in, int size);
bool decrypt(u8 (*A)[16], size_t A_bit_len, u8* K, u8* L, u8* N, u8* Cst0, u8* Cst1, u8* S_a, u8* T_a, u8 (*C)[16], size_t C_bit_len, u8* TAG_out, int tau, u8 M[16],u8* TAG_in);