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

//https://codereview.stackexchange.com/questions/5211/sine-function-in-c-c
float sine_taylor(float x)
{
	float x2 = x*x;
	float x4 = x2*x2;
	float t1 = x * ((float)1.0 - x2 / (2*3));
	float x5 = x * x4;
	float t2 = x5 * ((float)1.0 - x2 / (6*7)) / ((float)1.0* 2*3*4*5);
	float x9 = x5 * x4;
	float t3 = x9 * ((float)1.0 - x2 / (10*11)) / ((float)1.0* 2*3*4*5*6*7*8*9);
	float x13 = x9 * x4;
	float t4 = x13 * ((float)1.0 - x2 / (14*15)) / ((float)1.0*2*3*4*5*6*7*8*9*10*11*12*13);
	float result = t4;
	result += t3;
	result += t2;
	result += t1;
	return result;
}

int main(){
	float var0 = 0.9024806151106468;
	float var1 = 0.17342207429073256;
	float var2 = 0.21650050689617395;
	float var3 = 0.9012721135873213;
	float ftemp = 0.0;
	int multemp = 0;
	ftemp = (float) 1.0;
	ADSW_0((int*)&var0, *(unsigned int*)(&ftemp));
	ftemp = (float) 1.0;
	ADSW_1((int*)&var1, *(unsigned int*)(&ftemp));
	var1 = sine_taylor(var0);
	var2 = sin(var0);
	store_in_fpos((float*) &var0, 0);
	store_in_fpos((float*) &var1, 1);
	store_in_fpos((float*) &var2, 2);
	store_in_fpos((float*) &var3, 3);
	EXIT();
	return EXIT_SUCCESS;
}
