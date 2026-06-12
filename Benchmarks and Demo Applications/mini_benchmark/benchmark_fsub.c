#define AD_MUL
#include<stdio.h>
#include<math.h>
#include<stdlib.h>

#define ALPHA	0.16666667163372039794921875f
#define BETA	0.008333333767950534820556640625f
#define GAMMA	0.000198412701138295233249664306640625f
#define DELTA	0.000002755731884462875314056873321533203125f
#define PI	3.1415927410125732421875f

int main();

void EXIT(void){
    asm(".word 0x0000002b, 0x0000002b,0x0000002b":::);
	return;
}
/*
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

void debug_inst_div(void){
    asm volatile(
        "li     a5,9998336              \n\t" //a5 = 10,000,000
        "addi   a5,a5,1664              \n\t"
        "li     a1, 1                   \n\t" 
        :
        :
        : "a1", "a5"
        );
start:
    asm goto(
        "addi   a5,a5,-1                \n\t"
        "div    a2, a1, a1              \n\t" 
        "div    a2, a1, a1              \n\t"
        "div    a2, a1, a1              \n\t"
        "div    a2, a1, a1              \n\t"
        "div    a2, a1, a1              \n\t"
        "div    a2, a1, a1              \n\t"
        "div    a2, a1, a1              \n\t"
        "div    a2, a1, a1              \n\t"
        "div    a2, a1, a1              \n\t"
        "div    a2, a1, a1              \n\t"
        "bne    a5, zero, %l[start]     \n\t"
        :
        :
        : "a1", "a2", "a5"
        : start
        );
    return;
}

void debug_inst_sub_raw(void){
    asm volatile(
        "li     a5,9998336              \n\t" //a5 = 10,000,000
        "addi   a5,a5,1664              \n\t"
        "li     a1, 1                   \n\t" 
        :
        :
        : "a1", "a5"
        );
start:
    asm goto(
        "addi   a5,a5,-1                \n\t"
        "sub    a2, a1, a1              \n\t" 
        "sub    a2, a1, a1              \n\t"
        "sub    a2, a1, a1              \n\t"
        "sub    a2, a1, a1              \n\t"
        "sub    a2, a1, a1              \n\t"
        "sub    a2, a1, a1              \n\t"
        "sub    a2, a1, a1              \n\t"
        "sub    a2, a1, a1              \n\t"
        "sub    a2, a1, a1              \n\t"
        "sub    a2, a1, a1              \n\t"
        "bne    a5, zero, %l[start]     \n\t"
        :
        :
        : "a1", "a2", "a5"
        : start
        );
    return;
}
*/
void debug_inst_sub(void){
    asm volatile(
        "li     a5,1              \n\t" //a5 = 10,000,000
        "li     a1, 1             \n\t" 
	"addi   a5,a5,1            \n\t"
        :
        :
        : "a1", "a5"
        );
start:
    asm goto(
        "addi   a5,a5,-1                \n\t"
        "sub    s2, a1, a1              \n\t" 
        "sub    s3, a1, a1              \n\t"
        "sub    s4, a1, a1              \n\t"
        "sub    s5, a1, a1              \n\t"
        "sub    s6, a1, a1              \n\t"
        "sub    s7, a1, a1              \n\t"
        "sub    s8, a1, a1              \n\t"
        "sub    s9, a1, a1              \n\t"
        "sub    s10, a1, a1             \n\t"
        "sub    s11, a1, a1             \n\t"
        "bne    a5, zero, %l[start]     \n\t"
        :
        :
        : "a5","s2","s3","s4","s5","s6","s7","s8","s9","s10","s11"
        : start
        );
    //EXIT();
    return;
}

/*	//100,000,000
        "li     a5,99999744              \n\t" //a5 = 100,000,000
        "li     a1, 1                   \n\t" 
	"addi   a5,a5,256              \n\t"
*/


/*	//10,000,000
        "li     a5,9998336              \n\t" //a5 = 10,000,000
        "li     a1, 1                   \n\t" 
	"addi   a5,a5,1664              \n\t"
*/

/*	//1,000,000
        "li     a5,999424               \n\t" //a5 = 1,000,000
        "li     a1, 1                   \n\t" 
	"addi   a5,a5,576               \n\t"
*/

/*	//100,000
        "li     a5,98304                \n\t" //a5 = 100,000
        "li     a1, 1                   \n\t" 
	"addi   a5,a5,1696              \n\t"
*/

/*	//10,000
        "li     a5,8192                \n\t" //a5 = 10,000
        "li     a1, 1                   \n\t" 
	"addi   a5,a5,1808              \n\t"
*/

/*	//1,000
        "li     a5,1000              	  \n\t" //a5 = 1,000
        "li     a1, 1                    \n\t" 
*/

/*	//1
        "li     a5,1             	  \n\t" //a5 = 1
        "li     a1, 1                    \n\t" 
*/


void debug_inst_fsub(void){
    asm volatile(
        "li     a5,9998336		\n\t" //a5 = 10,000,000
        "li     a1, 1             	\n\t" 
	"addi   a5,a5,1664		\n\t"
        :
        :
        : "a1", "a5"
        );
start:
    asm goto(
        "addi      a5,a5,-1                \n\t"
        "fsub.s    f2, f1, f1              \n\t" 
        "fsub.s    f3, f1, f1              \n\t"
        "fsub.s    f4, f1, f1              \n\t"
        "fsub.s    f5, f1, f1              \n\t"
        "fsub.s    f6, f1, f1              \n\t"
        "fsub.s    f7, f1, f1              \n\t"
        "fsub.s    f8, f1, f1              \n\t"
        "fsub.s    f9, f1, f1              \n\t"
        "fsub.s    f10, f1, f1             \n\t"
        "fsub.s    f11, f1, f1             \n\t"
        "bne       a5, zero, %l[start]     \n\t"
        :
        :
        : "a5","f2","f3","f4","f5","f6","f7","f8","f9","f10","f11"
        : start
        );
    //EXIT();
    return;
}

void debug_ret(void){
	return;
}

int main(){
	//debug_inst_sub_raw();
	//debug_inst_div();
	//debug_ret();
	debug_inst_fsub();
	EXIT();
	return EXIT_SUCCESS;
}






