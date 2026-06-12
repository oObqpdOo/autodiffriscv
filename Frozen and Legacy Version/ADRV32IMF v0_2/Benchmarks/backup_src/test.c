void main();
void CALL_MAIN_FIRST(void){ //tiny hack, so that main gets called first when not using stdlib, proc does not have a start address yet
    	main();
	return;
}

void EXIT(void){
    asm(".word 0x0000002b, 0x0000002b,0x0000002b":::);
	return;
}

void ADSW_0(int* mem, int seed){
    asm(".word 0x00b5200b":::);
}
	
void ADSW_1(int* mem, int seed){
    asm(".word 0x00b5208b":::);
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


void main(){
	int x = 3; 
	int y = 5;
	ADSW_0(&x, 1);
	ADSW_1(&y, 1);

	int z = x + y * y; 

	store_in_pos(&x, 0);
	store_in_pos(&y, 1);
	store_in_pos(&z, 2);
	
	EXIT();
	return; 
}
