#define AD_MUL
#include<stdio.h>
#include<math.h>
#include<stdlib.h>

int main();

void EXIT(void){
	asm(".word 0x0000002b, 0x0000002b,0x0000002b":::);
	return;
}

void store_in_pos(int* valp, int mem){
	mem <<= 2;
	int val = *valp;
	asm("sw %0, 0(%1)" ::"r" (val), "r" (mem):);
}
void store_in_fpos(float* valp, int mem){
	mem <<= 2;
	float val = *valp;
	asm("fsw %0, 0(%1)" ::"f" (val), "r" (mem):);
}

void ADSW_0(int* mem, int seed){
	asm(".word 0x00b5200b":::); //ADSW a1, 0(a0)
}

void ADSW_1(int* mem, int seed){
	asm(".word 0x00b5208b":::);
}

int main(){
	asm volatile(
		"li	a5,0x4             	\n\t" //a5 = 0x4
		"li	a0, 1             	\n\t" 
		"li	a1, 2             	\n\t" 
		".word 0x00b5700b		\n\t" //ADMV a1, 0(a0)   
		:
		:
		: "a0", "a1", "a5"
	);
	EXIT();
	return EXIT_SUCCESS;
}
