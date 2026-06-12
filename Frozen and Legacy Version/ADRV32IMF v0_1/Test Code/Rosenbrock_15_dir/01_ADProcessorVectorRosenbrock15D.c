//https://www.geeksforgeeks.org/measure-execution-time-function-cpp/
//https://www.programiz.com/cpp-programming/operator-overloading
// Minimalistic AD C++ program to overload basic operations
// This program adds two AD numbers

#include<stdlib.h>

//#include<math.h>

#define MAX_DIR 15 			//has to be a multiple of 4! for the sake of NEON SIMD
#define MOMENTUM 0.999f 	//0.999f
#define STEP 0.00001f 		//0.00001f
#define STOP 0.001f			//0.001f
#define MAX_ITER 100000		//100000

//#define NAN (0.0f/0.0f)
#define NAN (__builtin_nanf(""))

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
void ADSW_1(int* mem, int seed){
    asm(".word 0x00b5208b":::);
}
void ADSW_2(int* mem, int seed){
    asm(".word 0x00b5210b":::);
}
void ADSW_3(int* mem, int seed){
    asm(".word 0x00b5218b":::);
}
void ADSW_4(int* mem, int seed){
    asm(".word 0x00b5220b":::);
}
void ADSW_5(int* mem, int seed){
    asm(".word 0x00b5228b":::);
}
void ADSW_6(int* mem, int seed){
    asm(".word 0x00b5230b":::);
}
void ADSW_7(int* mem, int seed){
    asm(".word 0x00b5238b":::);
}
void ADSW_8(int* mem, int seed){
    asm(".word 0x00b5240b":::);
}
void ADSW_9(int* mem, int seed){
    asm(".word 0x00b5248b":::);
}
void ADSW_10(int* mem, int seed){
    asm(".word 0x00b5250b":::);
}
void ADSW_11(int* mem, int seed){
    asm(".word 0x00b5258b":::);
}
void ADSW_12(int* mem, int seed){
    asm(".word 0x00b5260b":::);
}
void ADSW_13(int* mem, int seed){
    asm(".word 0x00b5268b":::);
}
void ADSW_14(int* mem, int seed){
    asm(".word 0x00b5270b":::);
}
void ADSW_15(int* mem, int seed){
    asm(".word 0x00b5278b":::);
}

void ADSW(int* mem, int seed, int dir){
	switch(dir){
		case 0: 
			ADSW_0(mem, seed);
			break; 	
		case 1:
			ADSW_1(mem, seed);
			break; 
		case 2:
			ADSW_2(mem, seed);
			break; 
		case 3: 
			ADSW_3(mem, seed);
			break; 	
		case 4:
			ADSW_4(mem, seed);
			break; 
		case 5:
			ADSW_5(mem, seed);
			break; 
		case 6: 
			ADSW_6(mem, seed);
			break; 	
		case 7:
			ADSW_7(mem, seed);
			break; 
		case 8:
			ADSW_8(mem, seed);
			break; 
		case 9:
			ADSW_9(mem, seed);
			break; 	
		case 10:
			ADSW_10(mem, seed);
			break; 
		case 11:
			ADSW_11(mem, seed);
			break; 
		case 12: 
			ADSW_12(mem, seed);
			break; 	
		case 13:
			ADSW_13(mem, seed);
			break; 
		case 14:
			ADSW_14(mem, seed);
			break;  
		case 15:
			ADSW_15(mem, seed);
			break;  	
	}
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
int ADLW_4(int* mem){
	int retval;
	asm(".word 0x00b5620b":::); //0b0000000 01011 01010 110 00000 0001010
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_5(int* mem){
	int retval;
	asm(".word 0x00b5628b":::);
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_6(int* mem){
	int retval;
	asm(".word 0x00b5630b":::);
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_7(int* mem){
	int retval;
	asm(".word 0x00b5638b":::);
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_8(int* mem){
	int retval;
	asm(".word 0x00b5640b":::); //0b0000000 01011 01010 110 00000 0001010
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_9(int* mem){
	int retval;
	asm(".word 0x00b5648b":::);
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_10(int* mem){
	int retval;
	asm(".word 0x00b5650b":::);
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_11(int* mem){
	int retval;
	asm(".word 0x00b5658b":::);
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_12(int* mem){
	int retval;
	asm(".word 0x00b5660b":::); //0b0000000 01011 01010 110 00000 0001010
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_13(int* mem){
	int retval;
	asm(".word 0x00b5668b":::);
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_14(int* mem){
	int retval;
	asm(".word 0x00b5670b":::);
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}
int ADLW_15(int* mem){
	int retval;
	asm(".word 0x00b5678b":::);
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}

int ADLW(int* mem, int dir){
	int retval;
	switch(dir){
		case 0: 
			retval = ADLW_0(mem);
			break; 	
		case 1:
			retval = ADLW_1(mem);
			break; 
		case 2:
			retval = ADLW_2(mem);
			break; 
		case 3: 
			retval = ADLW_3(mem);
			break; 	
		case 4:
			retval = ADLW_4(mem);
			break; 
		case 5:
			retval = ADLW_5(mem);
			break; 
		case 6: 
			retval = ADLW_6(mem);
			break; 	
		case 7:
			retval = ADLW_7(mem);
			break; 
		case 8:
			retval = ADLW_8(mem);
			break; 
		case 9: 
			retval = ADLW_9(mem);
			break; 	
		case 10:
			retval = ADLW_10(mem);
			break; 
		case 11:
			retval = ADLW_11(mem);
			break; 
		case 12: 
			retval = ADLW_12(mem);
			break; 	
		case 13:
			retval = ADLW_13(mem);
			break; 
		case 14:
			retval = ADLW_14(mem);			
			break; 
		case 15:
			retval = ADLW_15(mem);
			break;  	
	}
	return retval;
}
float rosenbrock(float x[MAX_DIR]){
	float a = 1.0f; 
	float b = 100.0f;
	float res = 0.0f;
	for(int i = 0; i < MAX_DIR-1; i++){
		res = res + (b*((x[i+1]-x[i]*x[i])*(x[i+1]-x[i]*x[i]))+((a-x[i])*(a-x[i])));
	}
	return res;
}


float fabsf(float a){
	if(a < 0.0f)
		return -a;
	return a;
}

bool isnanf(float a){
	if(a == NAN){
	//if(a != a) //trick to test for NAN? //https://stackoverflow.com/questions/1923837/how-to-use-nan-and-inf-in-c
		return true;
	}
	return false;
}


bool check_abort(float tmp){
	int check_abort = 0;	
	bool abort = false;
	for (int i = 0; i < MAX_DIR; i++) {
		int gradi = ADLW((int *) &tmp, i);
		float grad = *((float*)&gradi);	
	
		if(fabsf(grad) < STOP){
			check_abort++;		
		}
		if(isnanf(grad)){
			abort = true;
			break;		
		}
	}
	if(check_abort >= MAX_DIR-1){
		abort = true;		
	}
	return abort;
}

float optimize(void){
	int itercounter = 0;
	float x[MAX_DIR];
	float change[MAX_DIR];
	float tmp = 1.0f;
	
	for (int i = 0; i < MAX_DIR; i++){
		x[i] = 0.0f; 
		change[i] = 0.0f; 
		ADSW((int*)&x[i], *(int*)(&tmp), i);
	}

	bool abort = false;
	while((abort == false) && (itercounter < MAX_ITER)){
		tmp = rosenbrock(x);
		for (int i = 0; i < MAX_DIR; i++) {
			int gradi = ADLW((int *)&tmp, i);
			float grad = *((float*)&gradi);	
			
			change[i] = (grad * STEP) + (MOMENTUM * change[i]);
			x[i] = x[i] - change[i];
			//NEW SEEDING NOT REQUIRED; SEEDS OF X SHOULD NOT CHANGE!	
	    	}
		abort = check_abort(tmp);
		itercounter++;				
	}
	
	for (int i = 0; i < MAX_DIR; i++){
 		store_in_fpos(&x[i], i+1);
	}
 	store_in_pos(&itercounter, MAX_DIR+1);
	return tmp; 
}


int main() {
	float result = optimize();
 	store_in_fpos(&result, 0);
	EXIT();
	return 0;
}
