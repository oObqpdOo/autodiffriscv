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
float sine_taylor_old(float x)
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
//https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5336225
float sine_taylor(float x)
{
	float x2 = x*x;
	float t1 = GAMMA - DELTA * x2;
	float t2 = BETA - x2 * t1;
	float t3 = ALPHA - x2 * t2;
	float t4 = 1 - x2 * t3;
	float result = x * t4;
	return result;
}
int main(){
	float var0 = PI/2.0f;
	float var1 = PI/4.0f;
	float var2 = PI/6.0f;
	float var3 = PI;

	ADSW_0((int*)&var0, 0x3f800000);
	ADSW_0((int*)&var1, 0x3f800000);
	ADSW_0((int*)&var2, 0x3f800000);
	ADSW_0((int*)&var3, 0x3f800000);

	float var4 = sin(var0);
	float var5 = sin(var1);
	float var6 = sin(var2);
	float var7 = sin(var3);

	float var8 = sinf(var0);
	float var9 = sinf(var1);
	float var10 = sinf(var2);
	float var11 = sinf(var3);

	float var12 = sine_taylor(var0);
	float var13 = sine_taylor(var1);
	float var14 = sine_taylor(var2);
	float var15 = sine_taylor(var3);
	
	store_in_fpos((float*) &var0, 0);
	store_in_fpos((float*) &var1, 1);
	store_in_fpos((float*) &var2, 2);
	store_in_fpos((float*) &var3, 3);

	store_in_fpos((float*) &var4, 4);
	store_in_fpos((float*) &var5, 5);
	store_in_fpos((float*) &var6, 6);
	store_in_fpos((float*) &var7, 7);

	store_in_fpos((float*) &var8, 8);
	store_in_fpos((float*) &var9, 9);
	store_in_fpos((float*) &var10, 10);
	store_in_fpos((float*) &var11, 11);

	store_in_fpos((float*) &var12, 12);
	store_in_fpos((float*) &var13, 13);
	store_in_fpos((float*) &var14, 14);
	store_in_fpos((float*) &var15, 15);
	EXIT();
	return EXIT_SUCCESS;
}
