// Minimalistic AD C++ program to overload basic operations
// This program adds two AD numbers
// More infos at: https://distill.pub/2017/momentum/
// and: https://machinelearningmastery.com/gradient-descent-with-momentum-from-scratch/
// and: https://www.programiz.com/cpp-programming/operator-overloading
// and: https://www.geeksforgeeks.org/measure-execution-time-function-cpp/

#include <bits/stdc++.h>

#define DTYPE float			//double
#define MAX_DIR 15			//100
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
class AD {
    private:
        DTYPE val = 0;
        DTYPE adval[MAX_DIR] = {0}; //Values if default constructor was used
    public:
		// constructor to initialize val and adval to 0
		AD():val(), adval(){};
		//constructor - see below
		AD(DTYPE v, DTYPE adv[MAX_DIR])
		{
			val = v;
			for(int i = 0; i < MAX_DIR; ++i)
				adval[i] = adv[i];
		}
		~AD(){}
		// overload the + operator
		friend AD operator + (const AD& obj1, const AD& obj2) {
			AD temp;
			temp.val = obj1.val + obj2.val;
			for(int i = 0; i < MAX_DIR; i++){
				temp.adval[i] = obj1.adval[i] + obj2.adval[i];
			}
			return temp;
		}
		// overload the - operator
		friend AD operator - (const AD& obj1, const AD& obj2) {
			AD temp;
			temp.val = obj1.val - obj2.val;
			for(int i = 0; i < MAX_DIR; i++){
				temp.adval[i] = obj1.adval[i] - obj2.adval[i];
			}
			return temp;
		}
		// overload the * operator
		friend AD operator * (const AD& obj1, const AD& obj2) {
			AD temp;
			temp.val = obj1.val * obj2.val;
			for(int i = 0; i < MAX_DIR; i++){	
				temp.adval[i] = obj1.val * obj2.adval[i] + obj2.val * obj1.adval[i];
			}	
			return temp;
		}
		// overload the / operator
		friend AD operator / (const AD& obj1, const AD& obj2) {
			AD temp;
			temp.val = obj1.val / obj2.val;
			for(int i = 0; i < MAX_DIR; i++){
				temp.adval[i] = obj1.adval[i] / obj2.val - obj2.adval[i] * obj1.val / obj2.val * obj2.val;
			}			
			return temp;
		}
		DTYPE get_adval(int dir) {
			return adval[dir];
		}
		DTYPE get_val(void) {
			return val;
		}
		void set_adval(DTYPE nval, int dir) {
			adval[dir] = nval;
			return;
		}
		void reset_adval(void) {
			for(int i = 0; i < MAX_DIR; i++){
				adval[i] = 0.0f;
			}
			return;
		}
		void set_val(DTYPE nval) {
			val = nval;
			return;
		}
};


AD rosenbrock(AD x[MAX_DIR]){
	//Constant values
	DTYPE seed_a[MAX_DIR] = {0}; //const	
	DTYPE seed_b[MAX_DIR] = {0}; //const
	DTYPE seed_r[MAX_DIR] = {0}; //const
	AD a(1.0f, seed_a); 
	AD b(100.0f, seed_b);
	AD res(0.0f, seed_r);
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
	if(a == NAN)
		return true;
	return false;
}

bool check_abort(AD tmp){
	int check_abort = 0;	
	bool abort = false;
	for (int i = 0; i < MAX_DIR; i++) {
		if(fabsf(tmp.get_adval(i)) < STOP){
			check_abort++;		
		}
		if(myisnanf(tmp.get_adval(i))){
			abort = true;
			break;		
		}
	}
	if(check_abort >= MAX_DIR-1){
		abort = true;		
	}
	return abort;
}

//x[0] = 0;//x[1] = -0.5;
void optimize(void){
	int itercounter = 0;
	bool abort = false;
	AD x[MAX_DIR];
	AD tmp;
	DTYPE change[MAX_DIR] = {0};
	//set all x[] to random start value and all seeds
	for (int i = 0; i < MAX_DIR; i++) {
		x[i].set_val(0.0f);
		x[i].set_adval(1.0f, i);
	}
	//now iterate
	while((abort == false) && (itercounter < MAX_ITER)){
		//compute rosenbrock with current seeding - we get all partial derivatives with one call
		tmp = rosenbrock(x);
		//after this, we have MAX_DIR partial derivatives in tmp, thus we can update the start values of x[] and continue
		for (int i = 0; i < MAX_DIR; i++) {
			change[i] = (tmp.get_adval(i) * STEP) + (MOMENTUM * change[i]);
			x[i].set_val(x[i].get_val() - change[i]);	
			//new seeding - set all but one seed to 0! (reset previous seeding and set new seeding)
			//x[i].reset_adval();			
			//x[i].set_adval(1.0f, i);
	    	}
		abort = check_abort(tmp);
		itercounter++;				
	}
	float res = tmp.get_val();
 	store_in_pos((int*)&res, 0);
	for (int i = 0; i < MAX_DIR; i++){
		float resval = x[i].get_val();
		store_in_pos((int*)&resval, i+1);
	}
 	store_in_pos(&itercounter, MAX_DIR+1);
	return; 
}

int main() {
	optimize();
 	EXIT();
	return 0;
}

