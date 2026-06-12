#include<stdio.h>
#include<math.h>
#include<stdlib.h>

int main();

void CALL_MAIN_FIRST(void){ //tiny hack, so that main gets called first when not using stdlib, proc does not have a start address yet
    	main();
	return;
}

void EXIT(void){
    asm(".word 0x0000002b, 0x0000002b,0x0000002b":::);
	return;
}

void store_in_pos(int* valp, int mem){
    mem <<= 2;
    int val = *valp;
    asm("sw %0, 0(%1)" ::"r" (val), "r" (mem):);
}

void ADSW_0(int* mem, int seed){
    asm(".word 0x00b5200b":::);
}

void ADSH_0(short* mem, int seed){
    asm(".word 0x00b5100b":::);
}

void ADSB_0(char* mem, int seed){
    asm(".word 0x00b5000b":::);
}

void ADSW_1(int* mem, int seed){
    asm(".word 0x00b5208b":::);
}

void ADSH_1(short* mem, int seed){
    asm(".word 0x00b5108b":::);
}

void ADSB_1(char* mem, int seed){
    asm(".word 0x00b5008b":::);
}

int main(){
	int x = 3; 
	int y = 5;
	ADSW_0(&x, 0.0);
	ADSW_1(&y, 2.0);

	int z = x * y; 

	store_in_pos(&z, 0);
	EXIT();
	return EXIT_SUCCESS;
}
