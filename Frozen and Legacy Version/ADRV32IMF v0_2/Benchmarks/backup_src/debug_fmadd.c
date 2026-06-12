#define AD_MUL
#include<stdio.h>
#include<math.h>
#include<stdlib.h>

#define ALPHA  0.16666667163372039794921875f
#define BETA   0.008333333767950534820556640625f
#define GAMMA  0.000198412701138295233249664306640625f
#define DELTA  0.000002755731884462875314056873321533203125f
#define PI	   3.1415927410125732421875f

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
    asm(".word 0x00b5200b":::);
}

float debug_inst_fmadd(float val0, float val1, float val2){
    float *res; 
	asm("fmadd.s %0, %1, %2, %3" :"=fr" (val0):"fr" (val0), "fr" (val1), "fr" (val2):);
	asm("fsw %0, 0(%1)" ::"fr" (val0), "r" (res):);
	return *res;
}

float debug_inst_fsub(float val0, float val1){
    float *res; 
	asm("fsub.s %0, %1, %2" :"=fr" (val0):"fr" (val0), "fr" (val1):);
	asm("fsw %0, 0(%1)" ::"fr" (val0), "r" (res):);
	return *res;
}

int main(){
	float var0 = 2.0f;
	float var1 = 3.0f;
	float var2 = 4.0f;
	
	float var3 = debug_inst_fmadd(var0, var1, var2);
	store_in_fpos((float*) &var3, 0);

	float var4 = debug_inst_fsub(var0, var1);
	store_in_fpos((float*) &var3, 1);
	
	EXIT();
	return EXIT_SUCCESS;
}
