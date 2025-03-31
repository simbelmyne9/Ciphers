#include "STMAC.h"
#include<iostream>
#include <iomanip>

u8 H[16] = { 1, 6, 11, 12, 5, 10, 15, 0, 9, 14, 3, 4, 13, 2, 7, 8 };

void print_u8(u8* in, int size)
{
	for (int i = 0; i < size; i++)
	{
		// std::cout << "Element " << i << ": 0x" << std::hex << std::setw(8) << std::setfill('0') << in[i] << std::endl;
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(in[i]) << " ";
	}
	std::cout << std::endl;
}

void pad(u8 pt[16], int last_block_len)
{
	pt[last_block_len / 8] ^= (1 << (last_block_len % 8 - 1));
}

void lfsr3(u8 pt[16])
{
	u8 temp1 = pt[14] ^ pt[3];
	u8 temp2 = pt[15] ^ pt[4];
	for (int i = 0; i < 8; i++)
	{
		pt[i + 2] = pt[i];
		pt[i + 3] = pt[i + 1];
	}
	pt[0] = temp1;
	pt[1] = temp2;
}

void tranf_H(u8 pt[16])
{
	for (int i = 0; i < 16; i++)
	{
		pt[i] = h(pt[i]);
	}
}

void assignAtoB(u8 a[16], u8 b[16])
{
	for (int i = 0; i < 8; i++)
	{
		b[i] = a[i];
	}
}

void theta(u8 x[16])
{
	int flag = x[0] & 0b10000000;
	if (flag > 0)
	{
		// lsb(X) == 1
		tranf_H(x);
		lfsr3(x);
	}
	else
	{
		tranf_H(x);
	}
}

void contatenation_128_256(u8 tweakey[32], u8 flag, uint64_t ctr)
{
	for (int j = 0; j < 8; j++)
	{
		if (ctr < ((uint64_t)1 << 8 * j))
		{
			for (int k = 0; k < j; k++)
			{
				tweakey[31 - k] = ((ctr + 1) & (uint64_t)(0x000000ff) << (8 * k)) >> (8 * k);
			}
		}
		else
		{
			continue;
		}
	}
	tweakey[24] = (tweakey[24] & (0b00001111)) ^ flag;

}

void sTMAC_128_256(u8(*pt)[16], size_t pt_bits_num, u8 key[16], u8 N[8], u8 tag[], size_t tau, bool print_model)
{
	int last_block_len = pt_bits_num % 128;
	bool need_padding = last_block_len > 0;
	uint64_t block_num = pt_bits_num / 128 + need_padding;


	u8 S[16] = { 0 };
	u8 tweakey[32] = { 0 };
	for (int i = 0; i < 16; i++)	tweakey[i] = key[i];
	for (int i = 16; i < 16 + 8; i++) tweakey[i] = N[i - 16];

	for (uint64_t i = 0; i < block_num - 1; i++)
	{
		u8 Y[16] = { 0 };

		// create tweakey( N(64) || 2(4) || i(60))
		contatenation_128_256(tweakey, 0b00100000, i);

		aesTweakEncrypt(256, pt[i], tweakey, Y);

		if (print_model)
			print_u8(Y, 16);

		XOR(S, Y);
		theta(S);

	}


	if (need_padding)
	{
		pad(pt[block_num - 1], last_block_len);
		XOR(S, pt[block_num - 1]);
		contatenation_128_256(tweakey, 0b00010000, block_num - 1);
	}
	else
	{
		XOR(S, pt[block_num - 1]);
		contatenation_128_256(tweakey, 0b00000000, block_num - 1);
	}

	if (print_model)
		print_u8(S, 16);

	aesTweakEncrypt(256, S, tweakey, S);

	for (int i = 0; i < tau / 8; i++)
	{
		tag[i] = S[i];
	}

	if (print_model)
		print_u8(S, 8);
}

