#include<math.h>
#include<stdio.h>
#include<stdlib.h>

#define ALPHA	0.16666667163372039794921875f
#define BETA 	0.008333333767950534820556640625f
#define GAMMA	0.000198412701138295233249664306640625f
#define DELTA	0.000002755731884462875314056873321533203125f
#define PI	3.1415927410125732421875f
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
float sine_kernel(float x){
	float x2 = x*x;
	float t1 = GAMMA - DELTA * x2;
	float t2 = BETA - x2 * t1;
	float t3 = ALPHA - x2 * t2;
	float t4 = 1 - x2 * t3;
	float y = x * t4;
	return y;
}


float sine(float x)
{
	float a = 1;
	float ftemp_1 = 1.0f;
	//ADSW_0((int*)&x, *(unsigned int*)(&ftemp_1));	
	x = fmodf(x, 2*PI);
	ADSW_0((int*)&x, *(unsigned int*)(&ftemp_1));
	float x_abs = fabsf(x);
	if(x_abs >= 0 && x_abs <= PI/2){ 		//0 to PI/2	
		x = x;
	}
	else if(x_abs > PI/2 && x_abs <= PI){		//PI/2 to PI
		x = PI - x;
	}
	else if(x_abs > PI && x_abs <= 3*PI/2){	//PI to 3/2 PI
		x = x - PI;
		a = -1;
	}
	else if(x_abs > 3*PI/2 && x_abs <= 2*PI){	//3/2 PI to 2 PI
		x = 2 * PI - x;
		a = -1;
	}
	float y = a * sine_kernel(x);
	return y;
}

/*//KANN SIN(4), SIN(5) und SIN(6) nicht berechnen... WARUM?
float sine(float x)
{
	float y = 1.0f;
	x = fmodf(x, 2.0f*PI);
	float x_abs = fabsf(x);
	float ftemp_1 = 1.0f;
	ADSW_0((int*)&x, *(unsigned int*)(&ftemp_1));		
	if(x_abs >= 0 && x_abs <= PI/2){	
		y = sine_kernel(x);
	}
	else if(x_abs > PI/2 && x_abs <= PI){
		y = sine_kernel(PI - x);
	}
	else if(x_abs > PI && x_abs <= 3*PI/2){
		y = -sine_kernel(x - PI);
	}
	else if(x_abs > 3*PI/2 && x_abs <= 2*PI){
		y = -sine_kernel(2*PI - x);
	}
	return y;
}
*/

/*//WRONG SIGN OF DERIVATIVE!!!, BUT AT LEAST AD VALUES FOR EACH X
float sine(float x)
{
	float a;
	x = fmodf(x, 2*PI);
	float x_abs = fabsf(x);
	if(x_abs >= 0 && x_abs <= PI/2){	
		x = x;
		a = 1.0f; 
	}
	else if(x_abs > PI/2 && x_abs <= PI){
		x = PI - x;
		a = 1.0f;
	}
	else if(x_abs > PI && x_abs <= 3*PI/2){
		x = x - PI;
		a = -1.0f;
	}
	else if(x_abs > 3*PI/2 && x_abs <= 2*PI){
		x = 2*PI-x;
		a = -1.0f;
	}
	float ftemp_1 = 1.0;
	ADSW_0((int*)&x, *(unsigned int*)(&ftemp_1));	
	float y = sine_kernel(x);
	return y * a;
}
*/

int main(){
	float res[8];
	res[0] = sine(1);
	res[1] = sine(2);
	res[2] = sine(3);
	res[3] = sine(4);
	res[4] = sine(5);
	res[5] = sine(6);
	res[6] = sine(7);
	res[7] = sine(8);		
	for (int i = 0; i < 8; i++){
		store_in_fpos((float*) &res[i], i);
	}
	return EXIT_SUCCESS;
}
/*
int old_main(){
	float res[25];
	res[0] = test(0.1);
	res[1] = test(0.2);
	res[2] = test(0.3);
	res[3] = test(0.4);
	res[4] = test(0.5);
	res[5] = test(0.6);
	res[6] = test(0.7);
	res[7] = test(0.8);	
	res[8] = test(0.9);
	res[9] = test(1);
	res[10] = test(2);
	res[11] = test(3);
	res[12] = test(4);
	res[13] = test(5);
	res[14] = test(6);
	res[15] = test(7);
	res[16] = test(8);
	res[17] = test(PI/4);
	res[18] = test(PI/2);
	res[19] = test(3*PI/4);
	res[20] = test(4);
	res[21] = test(5);
	res[22] = test(6);
	res[23] = test(7);
	res[24] = test(42);			
	for (int i = 0; i < 25; i++){
		store_in_fpos((float*) &res[i], i);
	}
	return EXIT_SUCCESS;
}*/
