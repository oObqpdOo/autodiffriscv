#define AD_MUL
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

void store_in_fpos(float* valp, int mem){
    mem <<= 2;
    float val = *valp;
    asm("fsw %0, 0(%1)" ::"f" (val), "r" (mem):);
}

void ADSW_0(int* mem, int seed){
    asm(".word 0x00b5200b":::);
}

void ADSW_1(int* mem, int seed){
    asm(".word 0x00b5208b":::);
}


int main(){
	volatile int a = 3; 
	volatile int b = 5;
	volatile int c = 15;
	ADSW_0(&a, 1);
	
	volatile float d = 3; 
	volatile float e = 5;
	volatile float f = 15;
	volatile float seed = 1.0;
	ADSW_0((int*)&d, *(int*)(&seed));

	volatile int u = a * b / c;	// = 1
	volatile int v = a / b * c;	// = 0 
	volatile int w = v / u;	// = 0

	store_in_pos(&u, 0);
	store_in_pos(&v, 1);
	store_in_pos(&w, 2);	
	
	volatile float x = d * e / f;	// = 1.0 
	volatile float y = d / e * f;	// = 9.0	 
	volatile float z = y / x; 	// = 9.0

	store_in_fpos(&x, 3);
	store_in_fpos(&y, 4);
	store_in_fpos(&z, 5);	
	
	EXIT();
	return EXIT_SUCCESS;
}
