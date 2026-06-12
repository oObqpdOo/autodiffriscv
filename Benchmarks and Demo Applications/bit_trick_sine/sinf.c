#include<math.h>
#include<stdio.h>
#include<stdlib.h>

#define PI 3.1415927410125732421875f

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

float test_sinf(float x){
	float ftemp_1 = 1.0;
	ADSW_0((int*)&x, *(unsigned int*)(&ftemp_1));
	return sinf(x);
}
int main(){
	float y[8];
	y[0] = test_sinf(1);
	y[1] = test_sinf(2);
	y[2] = test_sinf(3);
	y[3] = test_sinf(4);
	y[4] = test_sinf(5);
	y[5] = test_sinf(6);
	y[6] = test_sinf(7);
	y[7] = test_sinf(8);
	for (int i = 0; i < 8; i++){
		store_in_fpos((float*) &y[i], i);
	}	
	return EXIT_SUCCESS;
}
