// Minimalistic AD C++ program to overload basic operations
// This program adds two AD numbers
// More infos at: https://distill.pub/2017/momentum/
// and: https://machinelearningmastery.com/gradient-descent-with-momentum-from-scratch/
// and: https://www.programiz.com/cpp-programming/operator-overloading
// and: https://www.geeksforgeeks.org/measure-execution-time-function-cpp/

#include <bits/stdc++.h>

#define DTYPE float			//double
#define MAX_DIR 15			//10
#define MOMENTUM 0.999f 		//0.999
#define STEP 0.00001f 			//0.00001f
#define STOP 0.001f			//0.001f
#define MAX_ITER 100000		//100000

//#define NAN (0.0f/0.0f)
#define NAN (__builtin_nanf(""))

//#define PRINT
//#define PRINT_SEEDS
//#define PRINT_START

using namespace std;

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


class AD {
    private:
        DTYPE val;
        DTYPE adval;

    public:
		// constructor to initialize val and adval to 0
		AD() : val(0), adval(0) {}
		AD(DTYPE val, DTYPE adval) : val(val), adval(adval){}

		// overload the + operator
		friend AD operator + (const AD& obj1, const AD& obj2) {
			AD temp;
			temp.val = obj1.val + obj2.val;
			temp.adval = obj1.adval + obj2.adval;
			return temp;
		}
		// overload the - operator
		friend AD operator - (const AD& obj1, const AD& obj2) {
			AD temp;
			temp.val = obj1.val - obj2.val;
			temp.adval = obj1.adval - obj2.adval;
			return temp;
		}
		// overload the * operator
		friend AD operator * (const AD& obj1, const AD& obj2) {
			AD temp;
			temp.val = obj1.val * obj2.val;
			temp.adval = obj1.val * obj2.adval + obj2.val * obj1.adval;
			return temp;
		}
		// overload the / operator
		friend AD operator / (const AD& obj1, const AD& obj2) {
			AD temp;
			temp.val = obj1.val / obj2.val;
			temp.adval = obj1.adval / obj2.val - obj2.adval * obj1.val / obj2.val * obj2.val;
			return temp;
		}
		DTYPE get_adval(void) {
			return adval;
		}
		DTYPE get_val(void) {
			return val;
		}
		void set_adval(DTYPE nval) {
			adval = nval;
			return;
		}
		void reset_adval(void) {
			adval = 0.0f;
			return;
		}
		void set_val(DTYPE nval) {
			val = nval;
			return;
		}
};

AD rosenbrock(AD x[MAX_DIR]){
	//Constant values
	AD a(1.0f, 0); 
	AD b(100.0f, 0);
	AD res(0.0f, 0);
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

bool myisnanf(float a){
	if(a == NAN){
	//if(a != a) //trick to test for NAN? //https://stackoverflow.com/questions/1923837/how-to-use-nan-and-inf-in-c
		return true;
	}
	return false;
}

bool check_abort(DTYPE* gradient){
	int check_abort = 0;	
	bool abort = false;
	for (int i = 0; i < MAX_DIR; i++) {
		if(fabsf(gradient[i]) < STOP){
			check_abort++;		
		}
		if(myisnanf(gradient[i])){
			abort = true;
			break;		
		}
	}
	if(check_abort >= MAX_DIR-1){
		abort = true;		
	}
	return abort;
}

void optimize(void){
	int itercounter = 0;
	bool abort = false;
	AD x[MAX_DIR];
	AD tmp;
	DTYPE change[MAX_DIR] = {0};
	DTYPE gradient[MAX_DIR] = {0};

	//set all x[] to random start value and all seeds to 0
	for (int i = 0; i < MAX_DIR; i++) {
		x[i].set_val(0.0f);
	}
	//now iterate
	while((abort == false) && (itercounter < MAX_ITER)){
		//we have to call rosenbrock not once, but max_dir times, every time with different seeding
		for (int i = 0; i < MAX_DIR; i++) {	
			//seeding - set all but one seed to 0! (reset previous seeding and set new seeding)
			for (int j = 0; j < MAX_DIR; j++) {	
				x[j].reset_adval();
			}		
			x[i].set_adval(1.0f);
			//compute rosenbrock with current seeding - we get one partial derivative
			tmp = rosenbrock(x);		
			gradient[i] = tmp.get_adval();
		}
		//after this, we have MAX_DIR values in gradient[], thus we can update the start values and continue
		for (int i = 0; i < MAX_DIR; i++) {
			change[i] = (gradient[i] * STEP) + (MOMENTUM * change[i]);
			x[i].set_val(x[i].get_val() - change[i]);	
	    	}
		abort = check_abort(gradient);
		itercounter++;				
	}
	float res = tmp.get_val();
 	store_in_fpos(&res, 0);
	for (int i = 0; i < MAX_DIR; i++){
		float resval = x[i].get_val();
		store_in_fpos(&resval, i+1);
	}
 	store_in_pos(&itercounter, MAX_DIR+1);
	return; 
}


int main() {
	optimize();
	EXIT();
	return 0;
}
