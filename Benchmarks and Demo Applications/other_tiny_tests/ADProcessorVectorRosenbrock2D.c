//https://www.geeksforgeeks.org/measure-execution-time-function-cpp/
//https://www.programiz.com/cpp-programming/operator-overloading
// Minimalistic AD C++ program to overload basic operations
// This program adds two AD numbers
#include<stdlib.h>

#define MAX_DIR 4 			//has to be a multiple of 4! for the sake of NEON SIMD
#define STEP 0.005f
#define STOP 0.00001f
#define MAX_ITER 10000

int main();

void CALL_MAIN_FIRST(void){ //tiny hack, so that main gets called first when not using stdlib, proc does not have a start address yet
    	main();
	return;
}
void EXIT(void){
    asm(".word 0x0000002b,0x0000002b,0x0000002b":::);
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

int ADLW_0(int* mem){
	int retval;
    asm(".word 0x00b5600b":::); //0b0000000 01011 01010 110 00000 0001010
    asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_1(int* mem){
	int retval;
    asm(".word 0x00b5608b":::);
    asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_2(int* mem){
	int retval;
    asm(".word 0x00b5610b":::);
    asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_3(int* mem){
	int retval;
    asm(".word 0x00b5618b":::);
    asm("mv %0, a1":"=r"(retval)::);
	return retval;
}




//x1 = 0; x2 = -0.5 -> dx1 = -2, dx2 = -10, y=3.5
float rosenbrock(float v1, float v2){
	//Seed Matrix
	float a = 1.0f;
	float b = 10.0f;
	float ftemp = 1.0f;
    //float seed_dv1[] = {1, 0};
	//float seed_dv2[] = {0, 1};
	ADSW_0((int*)&v1, *(unsigned int*)(&ftemp));
	ADSW_1((int*)&v2, *(unsigned int*)(&ftemp));

	//Constant values
	//float seed_a[MAX_DIR] = {0}; //const	
	//float seed_b[MAX_DIR] = {0}; //const
    //Create Variables
	//AD x1(v1, seed_dv1);
	//AD x2(v2, seed_dv2);
	//AD a(1.0f, seed_a); 
	//AD b(10.0f, seed_b); 
	float res = ((a-v1)*(a-v1))+(b*(v2 -v1*v1)*(v2 -v1*v1));
	/*
	int adval_0_i = ADLW_0((int*)&res);
	int adval_1_i = ADLW_1((int*)&res);
	float* adval_0 = (float*) &adval_0_i;
	float* adval_1 = (float*) &adval_1_i;
 	store_in_fpos(adval_0, 2);
 	store_in_fpos(adval_1, 3);
	*/
	return res;
}

float optimize(void){
	int itercounter = 0;
	float startval[MAX_DIR];
	float ftemp = 1.0f;
	startval[0] = 0.0f;
	startval[1] = -0.5f;
	ADSW_0((int*)&startval[0], *(unsigned int*)(&ftemp));
	ADSW_1((int*)&startval[1], *(unsigned int*)(&ftemp));

    float tmp = rosenbrock(startval[0], startval[1]);
	
	int adval_0_i = ADLW_0((int*)&tmp);
	int adval_1_i = ADLW_1((int*)&tmp);
	float* adval_0 = (float*) &adval_0_i;
	float* adval_1 = (float*) &adval_1_i;

	while((*adval_0 > STOP || *adval_1 > STOP || *adval_0 < -STOP || *adval_1 < -STOP) && itercounter < MAX_ITER){
		startval[0] -=  *adval_0 * STEP;
		startval[1] -=  *adval_1 * STEP;
		tmp = rosenbrock(startval[0], startval[1]);	
		adval_0_i = ADLW_0((int*)&tmp);
		adval_1_i = ADLW_1((int*)&tmp);
		adval_0 = (float*) &adval_0_i;
		adval_1 = (float*) &adval_1_i;		
		itercounter++;	
	}
 	store_in_pos(&itercounter, 4);
 	store_in_fpos(&startval[0], 5);
 	store_in_fpos(&startval[1], 6);
	return tmp; 
}


int main() {
    // calls the overloaded + operator
    float result;
	result = rosenbrock(0.0f, -0.5f);

	int adval_0_i = ADLW_0((int*)&result);
	float adval_0_f = *((float*)&adval_0_i);	
	int adval_1_i = ADLW_1((int*)&result);
	float adval_1_f = *((float*)&adval_1_i);	
	//int adval_2_i = ADLW_2((int*)&result);
	//int adval_3_i = ADLW_3((int*)&result);

    //result.display();
 	store_in_fpos(&result, 1);
 	store_in_fpos(&adval_0_f, 2);
 	store_in_fpos(&adval_1_f, 3);
 	//store_in_pos(&adval_2_i, 4);
 	//store_in_pos(&adval_3_i, 5);

    //auto start = high_resolution_clock::now();
	result = optimize();
 	store_in_fpos(&result, 0);
    //auto stop = high_resolution_clock::now();
    //auto duration = duration_cast<microseconds>(stop - start);

	EXIT();

    return 0;
}
