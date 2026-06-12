//author: Elias Kandler
//comparison results: https://www.rechner.club/stochastik/normalverteilung-berechnen

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


// square root calculation with Heron's method
float my_sqrt(float base)
{
    if(base <= 0.0) return 0.0;
    
    float diff = 1e-10, x0 = base / 2.0, x1;
    while(1)
    {
        x1 = (x0 + (base / x0)) / 2.0;
        if(x0 - x1 < diff) break;
        x0 = x1;
    }
    return x0;
}

float riscv_sqrt(float* valp){
    float val = *valp;
	float res;     
	asm("fsqrt %0, %1" ::"r" (res), "r" (val):);
}

// calculates the factorial value of n, it returns 1 if negative n is given 
float factorial(int n)
{
    float result = 1.0;
    if(n <= 0) return result;

    for(int i = 1; i <= n; i++)
    {
        result *= i;
    }
    return result;
}

// calculates the potency of a float base and an int exponent
float intexp_pow(float base, int expo)
{
    float result = 1.0;
    for(int i = 0; i < expo; i++)
    {
        result *= base;
    }
    return result;
}

// this function calculates e ^ z;
float eb_pow(float z)
{
    float acc = 1e-5, val, result = 0.0, fac;
    int n = 0, negative = 0;
    // turn negative exp positve, as e ^ -z = 1 / e ^ z
    if(z < 0.0) 
    {
        z *= -1.0;
        negative = 1;
    }
    while(1)
    {
        fac = factorial(n);
        // check if the fac is too large to fit float (constant is FLT_MAX / 10)
        if(fac >= (float) 34028234663852886000000000000000000000.0) break;

        val = intexp_pow(z, n) / fac;
        result += val; 
        if(val < acc) break;
        n++;
    }
    // do the 1 / e ^ z for negative z's
    if(negative)
    {
        return 1.0 / result;
    }
    return result;
}

void main()
{
    float sigma = 1.0, x = 0.1, mu = 0.0, result, expo, pi = 3.1415926535897932;
    int address = 0;
    for(float i = 0.0; i < 4.0; i += 0.1)
    {
         x = i;
         expo = (x - mu) / sigma;
         result = (1.0 / (sigma * my_sqrt(2.0 * pi))) * eb_pow(-0.5 * expo * expo);

         store_in_fpos(&result, address);
         address++;
    }
    return;
}


