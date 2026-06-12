#define ALPHA  0.16666667163372039794921875f
#define BETA   0.008333333767950534820556640625f
#define GAMMA  0.000198412701138295233249664306640625f
#define DELTA  0.000002755731884462875314056873321533203125f
#define PI	   3.1415927410125732421875f

#define ALPHAD  0.16666667163372039794921875f
#define BETAD   0.008333333767950534820556640625f
#define GAMMAD  0.000198412701138295233249664306640625f
#define DELTAD  0.000002755731884462875314056873321533203125f
#define PID	    3.1415927410125732421875f

void main();

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

void ADSW_1(int* mem, int seed){
    asm(".word 0x00b5208b":::);
}

double sine_taylor_double(double x)
{
	double x2 = x*x;
	double t1 = GAMMAD - DELTAD * x2;
	double t2 = BETAD - x2 * t1;
	double t3 = ALPHAD - x2 * t2;
	double t4 = 1 - x2 * t3;
	double result = x * t4;
	return result;
}

void main(){
	float a = PID/6.0;
	float b = 4.86;
	ADSW_0((int*)&a, 0x3f800000);
	double res =  sine_taylor_double((double) a);
	float var1 = (float) res;
	store_in_fpos(&a, 0);
	store_in_fpos(&b, 1);
	store_in_fpos(&var1, 2);
	
	EXIT();
	return;
}
