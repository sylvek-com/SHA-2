#include <stdio.h>
#include <string.h>
const unsigned int H0[8] = { // 256 bits
0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
};
unsigned int H[8];
const unsigned int K[64] = { // 2048 bits
0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 
0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

#ifndef SW
unsigned int W[64];
#else
unsigned int W[16];
#endif
#define SHR(x,n) ((x) >> n)
#define ROR(x,n) (((x) >> n) | ((x) << (32 - n)))
#define sigma0(x) (ROR(x,7) ^ ROR(x,18) ^ SHR(x,3))
#define sigma1(x) (ROR(x,17) ^ ROR(x,19) ^ SHR(x,10))
#define SIGMA0(x) (ROR(x,2) ^ ROR(x,13) ^ ROR(x,22))
#define SIGMA1(x) (ROR(x,6) ^ ROR(x,11) ^ ROR(x,25))
#define Cho(x,y,z) ((x)&(y)|~(x)&(z))
#define Maj(x,y,z) ((x)&(y)|(x)&(z)|(y)&(z))
void outer()
{
	register int i;
	unsigned int Ht[8],T1,T2;

	for (i = 0; i < 16; ++i)
		W[i] = __builtin_bswap32(W[i]);
#ifndef SW
	for (i = 16; i < 64; ++i)
		W[i] = sigma1(W[i-2]) + W[i-7] + sigma0(W[i-15]) + W[i-16];
#endif
	for (i = 0; i < 8; ++i)
		Ht[i] = H[i];
	for (i = 0; i < 64; ++i) {
#ifndef SW
#define W(x) W[x]
#else
#define W(x) W[(x)%16]
		if (i >= 16)
			W(i) = sigma1(W(i-2)) + W(i-7) + sigma0(W(i-15)) + W(i-16);
#endif
#ifndef SR
#define Ht(x) Ht[x]
#else
#define Ht(x) Ht[(x+64-i)%8]
#endif
#define a Ht(0)
#define b Ht(1)
#define c Ht(2)
#define d Ht(3)
#define e Ht(4)
#define f Ht(5)
#define g Ht(6)
#define h Ht(7)
		T1 = h + SIGMA1(e) + Cho(e,f,g) + K[i] + W(i);
		T2 = SIGMA0(a) + Maj(a,b,c);
#ifndef SR
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;
#else
		h = d + T1;
		d = T1 + T2;
		#if SR>1
		{ int j; for (j = 0; j < 8; ++j) printf("%08x ",Ht(j-1)); putchar('\n'); }
		#endif
#endif
	}
#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h
#undef W
#undef Ht
	for (i = 0; i < 8; ++i)
		H[i] = H[i] + Ht[i];
}
void prb(void *h,int n)
{
	register unsigned char *p = h;
	register int i;

	for(i = 0; i < n; ++i)
		printf("%02x",p[i]);
	putchar('\n');
}
void prw(unsigned int w[],int n)
{
	register int i;

	for (i = 0; i < n; ++i)
		printf("%08x",w[i]);
	putchar('\n');
}
int main()
{
	memset(W,0,16*4);
	W[0]  = 0x80636261;
	W[15] = 0x18000000;
	prb(W,16*4);
	memcpy(H,H0,8*4);
	outer();
	prw(H,8);

	memset(W,0,16*4);
	W[0] = 0x80;
	prb(W,16*4);
	memcpy(H,H0,8*4);
	outer();
	prw(H,8);

	return 0;
}
/*
W is a 16-position shift register:
-) initialized to zeros
-) shifts to the left, inserts on the right
-) 64 clocks total before next reset
-) 4 taps (2,7,15,16)
-) one feedback at insertion point
-) feedback is a mux:
--) 1-16 source block
--) 17-64 feedback sum (quad-adder required)
Ht is an 8-position shift register
-) initialized to H0 in reverse order
-) shifts to the right, inserts on the left
-) two insertion points: beginning & middle
--) middle taps tailing 5 positions + W + K (hexa-adder required)
--) beginning taps heading 3 positions + what middle did (triple-adder required) 
-) 64 clocks total before reinitialization
H is an 8-position shift register
-) initialized to H0 in reverse order
-) simply adds self to Ht
H appended and prepended to Ht
-) 64 clock dormant then 8 active
-) whatever shifted out of both H and Ht is added reinserted back to H and Ht at the other end
-) final readback is in the opposite direction of shifting!
Pipelining the everything using 32-bit word transfers
1) 8-clocks to load midstate
--- 1st
2) 16-clocks to process the second 512-bit block
3) 48-clocks to complete 64-iterations
--- 1st
4) 8-clocks to finalize the H in wrong order
5) 8-clocks to load initstate 
--- 2nd
6) 8-clocks to move H to the second round in correct order
7) 8-clocks to shift in padding for the second round
8) 48-clocks to complete 64-iterations second round
--- 2nd
9) 8-clocks to finalize the H in wrong order
10) test if last word shifted zero
11) previous shifted word should be less then target word (for verification)
Unequal clocks: H & Ht operation that involve single additions
can be used with much faster clock or parallelized to use 8 adders
and single clock to execute
*/
