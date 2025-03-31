#include "QTAE.h"

void gen_tweak(u8 const_num, u8 in[16], u8 out[16])
{
    for (int i = 0; i < 16; i++){ 
        out[i] = in[i];
    }
	out[0] &= 0b00011111;
	out[0] ^= (const_num << 5);
}

void gen_tweakey(u8* k, u8* t, u8* tk)
{
	for (int i = 0; i < 16; i++)
	{
		tk[i] = k[i];
		tk[i + 16] = t[i];
	}
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

void assign_128(u8* dst, u8* src)
{
	for (int i = 0; i < 16; i++)
	{
		dst[i] = src[i];
	}
}

void XOR(u8* in1, u8* in2, u8* out)
{
	for (int i = 0; i < 16; i++)
	{
		out[i] = in1[i] ^ in2[i];
	}
}

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

void phi(u8 in[16], u8 out[16])
{
	u32 S[4],phiS[4];
	for (int  i = 0; i < 4; i++)
	{
		S[i]=in[4*i]+(in[4*i+1]<<8)+(in[4*i+2]<<16)+(in[4*i+3]<<24);
	}
	phiS[0] = S[1];
	phiS[1] = S[2];
	phiS[2] = S[3];
	phiS[3] = S[0] ^ ((S[3]>>31)|(S[3]<<1));
	for (int i = 0; i < 4; i++)
	{	
		out[4*i+0] = (phiS[i] >> 24) & 0xff;
		out[4*i+1] = (phiS[i] >> 16) & 0xff;
		out[4*i+2] = (phiS[i] >> 8) & 0xff;
		out[4*i+3] = (phiS[i] >> 0) & 0xff;
	}
}

/*
void iphi(u8 in[16], u8 out[16])
{
	u32 S[4],iphiS[4];
	for (int  i = 0; i < 4; i++)
	{
		S[i]=in[4*i]+(in[4*i+1]<<8)+(in[4*i+2]<<16)+(in[4*i+3]<<24);
	}
	iphiS[0] = S[1] ^ S[2];
	iphiS[1] = S[2] ^ S[3];
	iphiS[2] = S[3] ^ S[4];
	iphiS[3] = S[0] ^ ((S[3]>>31)|(S[3]<<1)) ^ S[1];
	for (int i = 0; i < 4; i++)
	{	
		out[4*i+0] = (iphiS[i] >> 24) & 0xff;
		out[4*i+1] = (iphiS[i] >> 16) & 0xff;
		out[4*i+2] = (iphiS[i] >> 8) & 0xff;
		out[4*i+3] = (iphiS[i] >> 0) & 0xff;
	}
}
*/

void theta(u8* in_M, u8* in_S, u8* out_U, u8* out_C)
{
	u8 phiS[16]={0};
	phi(in_S,phiS);
	XOR(in_M,in_S,out_U);
	XOR(in_M,phiS,out_C);
}

void thetaStar(u8* in_C, u8* in_S, u8* out_U, u8* out_M)
{
	u8 phiS[16]={0};
	u8 iphiS[16] = {0};
	phi(in_S,phiS);
	XOR(in_C,phiS,out_M);
	XOR(out_M,in_S,out_U);
	
}

void QTAE_associate(u8 (*A)[16], size_t A_bit_len, u8* K, u8* L, u8* N, u8* Cst0, u8* Cst1, u8* S_a, u8* T_a)
{
    int last_block_len = A_bit_len % 128; // last block length in bits
	bool need_padding = last_block_len > 0; // need padding or not
	uint64_t block_num = A_bit_len / 128 + need_padding; // number of blocks

    //u8 new_t_a[16] = {};
	u8 temp_tweak[16] = {};
	u8 temp_tweakey[32] = {};
    u8 T_0[16] = { 0 };
    u8 S_0[16] = { 0 };
	// T_0 = E_[k, (1, cst0](N)
	gen_tweak(1, Cst0, temp_tweak);
	gen_tweakey(L, temp_tweak, temp_tweakey);//
	aesTweakEncrypt(256, N, temp_tweakey, T_0);//

	// S_0 = E_[k,(0, T_0)](Cst1)
	gen_tweak(0, T_0, temp_tweak);
	gen_tweakey(K, temp_tweak, temp_tweakey);//
	aesTweakEncrypt(256, Cst1, temp_tweakey, S_0);
	//**** 
	//Test Vector Output: T_0,S_0
	std::cout << "T_0: ";
	print_u8(T_0,16);
	std::cout << "S_0: ";
	print_u8(S_0,16);
    //****/
	u8 T[16],S[16];
	for(int i=0;i<16;i++) {
		T[i] = T_0[i];
		S[i] = S_0[i];
	}

	if(block_num>0)// with or without associated data
 	{
	for (int i = 0; i < block_num - 1; i++)//ִ
	{
		// T_i = h(T_[i-1])
		updTweak(T);
		gen_tweak(2, T, temp_tweak);
		XOR(A[i],S,S);
		gen_tweakey(K, temp_tweak, temp_tweakey);
		aesTweakEncrypt(256, S, temp_tweakey, S); 
	}
	if(need_padding)
	{
		pad(A[block_num - 1], last_block_len);
		XOR(A[block_num - 1], S, S);
		gen_tweak(2, T, temp_tweak);		
	}
	else{
		XOR(A[block_num - 1], S, S);
		gen_tweak(3, T, temp_tweak);
	}
	gen_tweakey(K, temp_tweak, temp_tweakey);
	aesTweakEncrypt(256, S, temp_tweakey, S);
	}

	for (int i = 0; i < 16; i++)
	{
		S_a[i] = S[i];
		T_a[i] = T[i];
	}
}

void QTAE_encrypt(u8 (*M)[16], size_t M_bit_len, u8* K, u8* N, u8* S_a, u8* T_a, u8* TAG_out, int tau, u8 C[16])
{
	int last_block_len = M_bit_len % 128; // last block length in bits
	bool need_padding = last_block_len > 0; // need padding or not
	uint64_t block_num = M_bit_len / 128 + need_padding; // number of blocks

	u8 temp_tweak[16] = {};
	u8 temp_tweakey[32] = {};
	u8 U[16]={0};
	u8 T[16], S[16];
	for(int i=0;i<16;i++)
	{
		T[i] = T_a[i];
		S[i] = S_a[i];
	}

	for (int i = 0; i < block_num - 1; i++)
	{
		// T_[a+i] = h(T_[a+i-1])
		updTweak(T);
		gen_tweak(4, T, temp_tweak);
		theta(M[i],S, U, C);
		gen_tweakey(K, temp_tweak, temp_tweakey);
		aesTweakEncrypt(256, U, temp_tweakey, S);

		//****  
		//Test Vector Output: T_1, S_1, C_1
		if(i==0){
			std::cout << "T_"<<i+1<<": ";
			print_u8(T,16);
			std::cout << "S_"<<i+1<<": ";
			print_u8(S,16);
			std::cout << "C_"<<i+1<<": ";
			print_u8(C,16);	
		}
		//****/
		
	}
	updTweak(T);//T_[a+m] = h(T_[a+m-1])
	//**** 
	//Test Vector Output: T_2
	std::cout << "T_2: ";
	print_u8(T,16);
	//****/

	if(need_padding)
	{
		pad(M[block_num - 1], last_block_len);
		theta(M[block_num - 1], S, U, C);
		gen_tweak(4, T, temp_tweak);
	}
	else{
		theta(M[block_num - 1], S, U, C);
		gen_tweak(5, T, temp_tweak);
	}
	std::cout << "C_"<<block_num<<": ";
	print_u8(C,16);	
	gen_tweakey(K, temp_tweak, temp_tweakey);
	aesTweakEncrypt(256, U, temp_tweakey, S);//S_[a+m]=E_[K,[4,T+m]](U_[a+m])
	
	

	gen_tweak(6, T, temp_tweak);
	gen_tweakey(K, temp_tweak, temp_tweakey);
	u8 St[16];
	aesTweakEncrypt(256, S, temp_tweakey, St);	
	for (int i = 0; i < tau / 8; i++)
	{
		TAG_out[i] = St[i];
	}
	//Test Vector Output: TAG_out
	std::cout << "Tag: ";
	print_u8(TAG_out, tau / 8);
	//****/
}


bool QTAE_decrypt(u8 (*C)[16], size_t C_bit_len, u8* K, u8* N, u8* S_a, u8* T_a, u8* TAG_out, int tau, u8 M[16],u8* TAG_in)
{
    int last_block_len = C_bit_len % 128; // last block length in bits
	bool need_padding = last_block_len > 0; // need padding or not
	uint64_t block_num = C_bit_len / 128 + need_padding; // number of blocks

	u8 temp_tweak[16] = {};
	u8 temp_tweakey[32] = {};
	u8 U[16]={0};
	u8 T[16], S[16];
	u8 pt[block_num][16] = {};
	for(int i=0;i<16;i++)
	{
		T[i] = T_a[i];
		S[i] = S_a[i];
	}

	for (int i = 0; i < block_num - 1; i++)
	{
		// T_[a+i] = h(T_[a+i-1])
		updTweak(T);
		gen_tweak(4, T, temp_tweak);
		thetaStar(C[i],S, U, M);
		for(int j=0; j<16; j++)
			{
				pt[i][j]=M[j];
			}
		gen_tweakey(K, temp_tweak, temp_tweakey);
		aesTweakEncrypt(256, U, temp_tweakey, S);
		
		std::cout << "T_"<<i+1<<": ";
		print_u8(T,16);
		std::cout << "S_"<<i+1<<": ";
		print_u8(S,16);
		std::cout << "M_"<<i+1<<": ";
		print_u8(M,16);		
	}
	updTweak(T);
	std::cout << "T_2: ";
	print_u8(T,16);

	if(need_padding)
	{
		pad(C[block_num - 1], last_block_len);
		thetaStar(C[block_num - 1], S, U, M);
		gen_tweak(4, T, temp_tweak);
	}
	else{
		thetaStar(C[block_num - 1], S, U, M);
		gen_tweak(5, T, temp_tweak);
	}
	std::cout << "M_"<<block_num<<": ";
	print_u8(M,16);	
	for(int j=0; j<16; j++)
	{
		pt[block_num - 1][j]=M[j];
	}
	gen_tweakey(K, temp_tweak, temp_tweakey);
	aesTweakEncrypt(256, U, temp_tweakey, S);//S_[a+m]=E_[K,[4,T+m]](U_[a+m])
	gen_tweak(6, T, temp_tweak);
	gen_tweakey(K, temp_tweak, temp_tweakey);
	u8 St[16];
	aesTweakEncrypt(256, S, temp_tweakey, St);	
	for (int i = 0; i < tau / 8; i++)
	{
		TAG_out[i] = St[i];
	}
	std::cout << "Tagout: ";
	print_u8(TAG_out, tau / 8);
	std::cout << "Tagin: ";
	print_u8(TAG_in, tau / 8);
    
    for(int i = 0; i < tau/8 ; i++)
    {
        if(TAG_in[i] != TAG_out[i]) 
		{
			std::cout << "Tags are inconsistent" << std::endl;
			return false;
		}

    }
	for(int i=0;i<block_num;i++)
	{
		std::cout << "M_"<<i+1<< ": ";
		print_u8(pt[i], 16);
	}
	return true;
}

bool decrypt(u8 (*A)[16], size_t A_bit_len, u8* K, u8* L, u8* N, u8* Cst0, u8* Cst1, u8* S_a, u8* T_a,
u8 (*C)[16], size_t C_bit_len, u8* TAG_out, int tau,u8 M[16], u8* TAG_in)
{
	bool result;
	QTAE_associate(A, A_bit_len, K,  L,  N,  Cst0,  Cst1,  S_a,  T_a);
	result = QTAE_decrypt(C, C_bit_len, K, N,  S_a, T_a, TAG_out, tau, M, TAG_in);
	return result;
}