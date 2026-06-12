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
	int results[50];
	int a, b, solution;
	int counter = 0;

	for(a = 10; a < 20; a += 2)
	{
		for(b = 0; b < 10; b += 1)
		{
			solution = a + b;
			store_in_pos((int*) &solution, 0);
			store_in_pos((int*) &counter, 1);
			store_in_pos((int*) &a, a);
			//store_in_pos((int*) &b, b);
			counter++;
		}
	}
	return;
}

