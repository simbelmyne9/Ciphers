#include "QTMAC.h"

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

void contatenation_128(u8 flag, u8 tweak[16])
{
	flag=flag<<6;
	tweak[0] = (tweak[0] & (0b00111111)) ^ flag;

}

void updTweak(u8 t[16])
{
	for(int i=0;i<15;i++)
	{
		t[i] = (t[i] << 1) | (t[i+1] >> 7 ) ; 
	}
	t[15] = t[15] << 1;
	if((t[0]^0x80) !=0 )
	{
		t[15] = t[15] & 0b10000111;
	}
}

void qTMAC_128_256(u8 (*pt)[16], size_t pt_bits_num, u8 key[16], u8 N[12], u8 tag[], size_t tau, u8 cst0[16],u8 cst1[16])
{
	int last_block_len = pt_bits_num % 128; // last block length in bits
	bool need_padding = last_block_len > 0; // need padding or not
	uint64_t block_num = pt_bits_num / 128 + need_padding; // number of blocks
	
	//Line-1
	u8 padN[16]={0};
	for(int i = 0; i<12; i++) padN[i] = N[i];
	padN[12]=0b10000000;
	//Line-2
	u8 T_0[16] = { 0 };
	u8 tweakey[32] = { 0 };
	for (int i = 0; i < 16; i++)	tweakey[i] = key[i];
	cst0[0]=(cst0[0] & (0b00111111)) ^ (0b01000000);
	for (int i = 16; i < 16 + 16; i++) tweakey[i] = cst0[i - 16];
	aesTweakEncrypt(256,padN,tweakey,T_0);
	/*** 
	std::cout << "T_0: ";
	print_u8(T_0,16);
	***/
	//Line-3
	u8 S_0[16] = { 0 };
	T_0[0] = T_0[0] & (0b00111111);
	for (int i = 16; i < 16 + 16; i++) tweakey[i] = T_0[i - 16];
	aesTweakEncrypt(256,cst1,tweakey,S_0);
	/*** 
	std::cout << "S_0: ";
	print_u8(S_0,16);
	***/
	//Line-4
	u8 T[16],S[16];
	for(int i=0;i<16;i++) {
		T[i] = T_0[i];
		S[i] = S_0[i];
	}
	for(int i=0;i<block_num-1;i++)
	{
		updTweak(T);
		tweakey[16] = (T[0] & (0b00111111)) ^ (0b10000000);
		for (int j = 17; j < 17 + 15; j++) tweakey[j] = T[j - 16];
		XOR(pt[i],S);
		aesTweakEncrypt(256, pt[i], tweakey, S);
		/***
		std::cout << "T_"<<i+1<<": ";	
		print_u8(T,16);
		std::cout << "S_"<<i+1<<": ";
		print_u8(S,16);		
		//***/
	}
	//Line-5
	updTweak(T);
	/***
	std::cout << "T_"<<block_num<<": ";	
	print_u8(T,16);
	***/
	//Line-6
	if (need_padding)
	{
		pad(pt[block_num - 1], last_block_len);
		XOR(pt[block_num - 1],S);
		tweakey[16] = (T[0] & (0b00111111)) ^ (0b10000000);
	}
	else
	{
		updTweak(T);
		XOR(pt[block_num - 1],S);
		tweakey[16] = (T[0] & (0b00111111)) ^ (0b11000000);
	}
	aesTweakEncrypt(256, S, tweakey, S);
	//Line-7
	for (int i = 0; i < tau / 8; i++)
	{
		tag[i] = S[i];
	}
}


