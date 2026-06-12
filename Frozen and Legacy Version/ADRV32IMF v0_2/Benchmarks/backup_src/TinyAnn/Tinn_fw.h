#ifndef __TINN_FW
#define __TINN_FW

#pragma once

#define NORMAL_PRINTS
//#define USE_RAND
//#define USE_CONST_SEED

#ifdef NORMAL_PRINTS
#include <stdlib.h>
#include <stdio.h>
#endif

#ifdef USE_RAND
#include <stdlib.h>
#ifndef USE_CONST_SEED
#include <time.h>
#endif
#endif

#define ROWS 10
//original
//#define ROWS 1593
//short
//#define ROWS 199
//very short
//#define ROWS 50
//ridiculous short
//#define ROWS 10
#define COLS 32
// Input and output size is harded coded here as machine learning
// repositories usually don't include the input and output size in the data itself.
#define NIPS 256
#define NOPS 10
#define NHID 28

#define CHECK_BIT(var,pos) (((var) & (1<<pos)) > 0 ? 1 : 0)

typedef unsigned int uint;
void init_lfsr(uint seed);
int get_random(void);

typedef struct
{
    // All the weights.
    float* w;
    // Hidden to output layer weights.
    float* x;
    // Biases.
    float* b;
    // Hidden layer.
    float* h;
    // Output layer.
    float* o;
    // Number of biases - always two - Tinn only supports a single hidden layer.
    int nb;
    // Number of weights.
    int nw;
    // Number of inputs.
    int nips;
    // Number of hidden neurons.
    int nhid;
    // Number of outputs.
    int nops;
}
Tinn;

float* xtpredict(Tinn, const float* in);

float xttrain(Tinn, const float* in, const float* tg, float rate);

Tinn xtbuild(int nips, int nhid, int nops);

void xtfree(Tinn);

void xtprint(float* arr, const int size, const int address);

#endif
