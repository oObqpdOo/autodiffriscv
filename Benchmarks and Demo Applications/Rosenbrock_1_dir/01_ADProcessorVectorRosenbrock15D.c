//https://www.geeksforgeeks.org/measure-execution-time-function-cpp/
//https://www.programiz.com/cpp-programming/operator-overloading
// Minimalistic AD C++ program to overload basic operations
// This program adds two AD numbers

#include<stdlib.h>

//#include<math.h>

#define MAX_DIR 15 			//has to be a multiple of 4! for the sake of NEON SIMD
#define MOMENTUM 0.999f 		//0.999f
#define STEP 0.00001f 			//0.00001f
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

void ADSW(int* mem, int seed, int dir){
	switch(dir){
		case 0: 
			ADSW_0(mem, seed);
			break; 	
	}
}

int ADLW_0(int* mem){
	int retval;
	asm(".word 0x00b5600b":::); //0b0000000 01011 01010 110 00000 0001010
	asm("mv %0, a1":"=r"(retval)::);
	return retval;
}

int ADLW(int* mem, int dir){
	int retval;
	switch(dir){
		case 0: 
			retval = ADLW_0(mem);
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


bool check_abort(float tmp[MAX_DIR]){
	int check_abort = 0;	
	bool abort = false;
	for (int i = 0; i < MAX_DIR; i++) {
		if(fabsf(tmp[i]) < STOP){
			check_abort++;		
		}
		if(isnanf(tmp[i])){
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
	float tmp_0 = 0.0f;
	float tmp_1 = 1.0f;
	float tmp = 0.0f;
	int gradi;
	float grad[MAX_DIR];
	
	for (int i = 0; i < MAX_DIR; i++){
		x[i] = 0.0f; 
		change[i] = 0.0f; 
		ADSW((int*)&x[i], *(int*)(&tmp_0), 0);
	}

	bool abort = false;
	while((abort == false) && (itercounter < MAX_ITER)){
		//compute all partial derivatives
		for (int dirs = 0; dirs < MAX_DIR; dirs++){
			ADSW((int*)&x[dirs], *(int*)(&tmp_1), 0); 	//setting the seed
			tmp = rosenbrock(x);				//computing
			ADSW((int*)&x[dirs], *(int*)(&tmp_0), 0); 	//resetting the seed
			gradi = ADLW((int *)&tmp, 0);			//retrieving the partial derivative
			grad[dirs] = *((float*)&gradi);		//storing it as float
			
		}
		//afterwardsalter all 
		for (int i = 0; i < MAX_DIR; i++) {	
			change[i] = (grad[i] * STEP) + (MOMENTUM * change[i]);
			x[i] = x[i] - change[i];
	    	}
		abort = check_abort(grad);
		itercounter++;				
	}
	
	for (int i = 0; i < MAX_DIR; i++){
 		store_in_fpos(&x[i], i+1);	//1 bis 15 = x (1 bis MAX_DIR)
	}
	for (int i = 0; i < MAX_DIR; i++){
 		store_in_fpos(&grad[i], i + MAX_DIR + 1); //16 bis 30 = grad (MAX_DIR + 1 bis MAX_DIR + MAX_DIR)
	}
 	store_in_pos(&itercounter, MAX_DIR + MAX_DIR + 1); //32 (MAX_DIR + MAX_DIR + 1)
	return tmp; 
}


int main() {
	float result = optimize();
 	store_in_fpos(&result, 0); //0 = result
	EXIT();
	return 0;
}
