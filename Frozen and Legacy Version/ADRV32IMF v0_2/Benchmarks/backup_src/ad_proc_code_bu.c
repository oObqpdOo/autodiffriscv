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


int main(){
	float var0 = 0.9024806151106468;
	float var1 = 0.17342207429073256;
	float var2 = 0.21650050689617395;
	float var3 = 0.9012721135873213;
	var1 = var2 + var3;
	var0 = sin(var1);
	store_in_fpos((float*) &var0, 0);
	store_in_fpos((float*) &var1, 1);
	store_in_fpos((float*) &var2, 2);
	store_in_fpos((float*) &var3, 3);
	EXIT();
	return EXIT_SUCCESS;
}
