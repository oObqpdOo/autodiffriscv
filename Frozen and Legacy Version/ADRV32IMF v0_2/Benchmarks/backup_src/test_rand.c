#include <time.h>
#include <stdlib.h>

void store_in_pos(int* valp, int mem){
    mem <<= 2;
    int val = *valp;
    asm("sw %0, 0(%1)" ::"r" (val), "r" (mem):);
}

int main()
{
    srand(23);
	int b = rand() % 5;
	store_in_pos(&b, 0);

	return 1;
}
