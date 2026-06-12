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

void ADSW_JIT(int* mem, int seed, unsigned char channel){
	unsigned int instruction = channel << 7; //shift channel to bit 7 to 11
	instruction |= 0x00b5200b; //ADSW a1, channel(a0)
	unsigned int return_instruction = 0x00008067; //generate return instruction
	unsigned int* address_instruction; //generate address to jump to 
	unsigned int* address_return = address_instruction + 1; //place ret next
	unsigned int* old_return; //place to backup previous ra
	asm("sw ra, 0(%0)"::"r" (old_return):); //backup ra
	asm("sw %0, 0(%1)"::"r" (instruction), "r" (address_instruction):);
	asm("sw %0, 0(%1)"::"r" (return_instruction), "r" (address_return):);
	asm("jalr ra, %0, 0"::"r" (address_instruction):);	
	asm("lw ra, 0(%0)"::"r" (old_return):);
}

int main(){
	int val_i = 5; 
	int seed_i = 1;
	float val_f = 5.0f;
	float seed_f = 1.0f;
	ADSW_0(&val_i, seed_i);								/*@\label{codel:ADSW_0:1}@*/
	ADSW_0((int*)&val_f, *(unsigned int*)(&seed_f));	/*@\label{codel:ADSW_0:2}@*/

	store_in_pos(&val_i, 0);
	store_in_fpos(&val_f, 1);		

	ADSW_0(&val_i, 1);
	ADSW_0((int*)&val_f, 0x3f800000);

	store_in_pos(&val_i, 2);
	store_in_fpos(&val_f, 3);	

	ADSW_JIT(&val_i, 1, 1);	
	store_in_pos(&val_i, 4);

	EXIT();
	return EXIT_SUCCESS;
}
