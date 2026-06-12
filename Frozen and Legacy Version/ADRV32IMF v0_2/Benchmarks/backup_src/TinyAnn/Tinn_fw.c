#include "Tinn_fw.h"

//#include <stdarg.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>

//USING LFSR to generate random number generators! (16-bit -> 0 to 65535)
//https://www.analog.com/en/resources/design-notes/random-number-generation-using-lfsr.html
#define POLY_MASK_32 0xB4BCD35C
#define POLY_MASK_31 0x7A5BC2E3
#define MY_RAND_MAX 65535
uint lfsr32, lfsr31;
int shift_lfsr(uint *lfsr, uint polynomial_mask){
	int feedback; 
	feedback = *lfsr & 1;
	*lfsr >>= 1;
	if(feedback == 1){
		*lfsr ^= polynomial_mask;
	}
	return *lfsr;
}
void init_lfsr(uint seed){
	lfsr32 = seed;		//0xABCDE; //SEED_VALUE
	lfsr31 = 0x23456789;
}
int get_random(void){
	//This random number generator shifts the 32-bit LFSR twice before XORing it with the 31-bit LFSR. The bottom 16 bits are used for the random number
	shift_lfsr(&lfsr32, POLY_MASK_32);
	return(shift_lfsr(&lfsr32, POLY_MASK_32)  ^ shift_lfsr(&lfsr31, POLY_MASK_31)) & 0xFFFF;
}
/*
//USE AS FOLLOWS
void main(void)
{
	int random_value;
	init_lfsrs();
	random_value = get_random(); 
}
*/

float t_w[NHID*(NIPS+NOPS)];
float t_b[2];
float t_h[NHID];
float t_o[NOPS];

float __int_as_float(int a){
	return *((float*) &a);
}

float fmaf(float a, float b, float c){
	return (a*b+c);
}

float fabsf(float a){
	if(a < 0)
		return -a;
	return a;
}

//https://forums.developer.nvidia.com/t/a-more-accurate-performance-competitive-implementation-of-expf/47528
 float my_expf (float a)
{
    float f, r, j, s, t;
    int i, ia;

    // exp(a) = 2**i * exp(f); i = rintf (a / log(2))
    j = fmaf (1.442695f, a, 12582912.f) - 12582912.f; // 0x1.715476p0, 0x1.8p23
    f = fmaf (j, -6.93145752e-1f, a); // -0x1.62e400p-1  // log_2_hi 
    f = fmaf (j, -1.42860677e-6f, f); // -0x1.7f7d1cp-20 // log_2_lo 
    i = (int)j;
    // approximate r = exp(f) on interval [-log(2)/2, +log(2)/2]
    r =             1.37805939e-3f;  // 0x1.694000p-10
    r = fmaf (r, f, 8.37312452e-3f); // 0x1.125edcp-7
    r = fmaf (r, f, 4.16695364e-2f); // 0x1.555b5ap-5
    r = fmaf (r, f, 1.66664720e-1f); // 0x1.555450p-3
    r = fmaf (r, f, 4.99999851e-1f); // 0x1.fffff6p-2
    r = fmaf (r, f, 1.00000000e+0f); // 0x1.000000p+0
    r = fmaf (r, f, 1.00000000e+0f); // 0x1.000000p+0
    // exp(a) = 2**i * r
    ia = (i > 0) ?  0 : 0x83000000;
    s = __int_as_float (0x7f000000 + ia);
    t = __int_as_float ((i << 23) - ia);
    r = r * s;
    r = r * t;
    // handle special cases: severe overflow / underflow
    if (fabsf (a) >= 104.0f) r = s * s;
    return r;
}

//AD Processor finish execution
void EXIT(void){
    asm(".word 0x0000002b, 0x0000002b,0x0000002b":::);
	return;
}

#ifndef NORMAL_PRINTS
//Print eqivalent for ints
void store_in_pos(int* valp, int mem){
    mem <<= 2;
    int val = *valp;
    asm("sw %0, 0(%1)" ::"r" (val), "r" (mem):);
}

//Print eqivalent for floats
void store_in_fpos(float* valp, int mem){
    mem <<= 2;
    float val = *valp;
    asm("fsw %0, 0(%1)" ::"f" (val), "r" (mem):);
}
#endif

// Computes error.
static float err(const float a, const float b)
{
    return 0.5f * (a - b) * (a - b);
}

// Returns partial derivative of error function.
static float pderr(const float a, const float b)
{
    return a - b;
}

// Computes total error of target to output.
static float toterr(const float* const tg, const float* const o, const int size)
{
    float sum = 0.0f;
    for(int i = 0; i < size; i++)
        sum += err(tg[i], o[i]);
    return sum;
}

// Activation function.
static float act(const float a)
{
    return 1.0f / (1.0f + my_expf(-a));
}

// Returns partial derivative of activation function.
static float pdact(const float a)
{
    return a * (1.0f - a);
}

// Returns floating point random from 0.0 - 1.0.
static float frand()
{
#ifdef USE_RAND    
	return rand() / (float) RAND_MAX;
#else
	return get_random() / (float) MY_RAND_MAX;
#endif
}

// Performs back propagation.
static void bprop(const Tinn t, const float* const in, const float* const tg, float rate)
{
    for(int i = 0; i < t.nhid; i++)
    {
        float sum = 0.0f;
        // Calculate total error change with respect to output.
        for(int j = 0; j < t.nops; j++)
        {
            const float a = pderr(t.o[j], tg[j]);
            const float b = pdact(t.o[j]);
            sum += a * b * t.x[j * t.nhid + i];
            // Correct weights in hidden to output layer.
            t.x[j * t.nhid + i] -= rate * a * b * t.h[i];
        }
        // Correct weights in input to hidden layer.
        for(int j = 0; j < t.nips; j++)
            t.w[i * t.nips + j] -= rate * sum * pdact(t.h[i]) * in[j];
    }
}

// Performs forward propagation.
static void fprop(const Tinn t, const float* const in)
{
    // Calculate hidden layer neuron values.
    for(int i = 0; i < t.nhid; i++)
    {
        float sum = 0.0f;
        for(int j = 0; j < t.nips; j++)
            sum += in[j] * t.w[i * t.nips + j];
        t.h[i] = act(sum + t.b[0]);
    }
    // Calculate output layer neuron values.
    for(int i = 0; i < t.nops; i++)
    {
        float sum = 0.0f;
        for(int j = 0; j < t.nhid; j++)
            sum += t.h[j] * t.x[i * t.nhid + j];
        t.o[i] = act(sum + t.b[1]);
    }
}

// Randomizes tinn weights and biases.
static void wbrand(const Tinn t)
{
    for(int i = 0; i < t.nw; i++) t.w[i] = frand() - 0.5f;
    for(int i = 0; i < t.nb; i++) t.b[i] = frand() - 0.5f;
}

// Returns an output prediction given an input.
float* xtpredict(const Tinn t, const float* const in)
{
    fprop(t, in);
    return t.o;
}

// Trains a tinn with an input and target output with a learning rate. Returns target to output error.
float xttrain(const Tinn t, const float* const in, const float* const tg, float rate)
{
    fprop(t, in);
    bprop(t, in, tg, rate);
    return toterr(tg, t.o, t.nops);
}

// Constructs a tinn with number of inputs, number of hidden neurons, and number of outputs
Tinn xtbuild(const int nips, const int nhid, const int nops)
{
    Tinn t;
    // Tinn only supports one hidden layer so there are two biases.
    t.nb = 2;
    t.nw = nhid * (nips + nops);
    /*
	t.w = (float*) calloc(t.nw, sizeof(*t.w));
	*/
	t.w = t_w;
    t.x = t.w + nhid * nips;
    /*
	t.b = (float*) calloc(t.nb, sizeof(*t.b));
    t.h = (float*) calloc(nhid, sizeof(*t.h));
    t.o = (float*) calloc(nops, sizeof(*t.o));
	*/
	t.b = t_b;
	t.h = t_h;
	t.o = t_o;
    t.nips = nips;
    t.nhid = nhid;
    t.nops = nops;
    wbrand(t);
    return t;
}

// Frees object from heap.
void xtfree(const Tinn t)
{
/*
    free(t.w);
    free(t.b);
    free(t.h);
    free(t.o);
*/
}

// Prints an array of floats. Useful for printing predictions.
void xtprint(float* arr, const int size, const int address)
{
    for(int i = 0; i < size; i++){
        //printf("%f ", (double) arr[i]);
    	//store_in_fpos((float*) &var0, 0);
	//store_in_fpos((float*) &var1, 1);
#ifdef NORMAL_PRINTS
	printf("%f \n", (double) arr[i]);
#else
    	store_in_fpos((float*) &arr[i], address+i);
#endif
	}
}
