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

void main()
{
        float results[50];
        float a, b, c, x0, x1, x2, solution;
        int counter = 0;

        for(a = 2.0; a < 12.0; a += 2.0)
        {
                for(b = 10.0; b > 5.0; b -= 1)
                {
                        for(c = -4.0; c < -3.0; c += 0.5)
                        {
                                x0 = a / b;
                                x1 = a + b - c;
                                x2 = b * c / a;
                                solution = x2 * x2 * x2 + x1 * x1 + x0;
								float fsolution = (float) solution;  
								store_in_fpos(&fsolution, counter);
                                counter++;
                        }
                }
        }
        return;
}

