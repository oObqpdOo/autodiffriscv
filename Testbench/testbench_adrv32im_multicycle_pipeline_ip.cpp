#include <stdio.h>
#include "../ProcessorCode/adrv32imf_mp_ip.h"
#include "../ProcessorCode/print.h"

//#include "test_branch.h"
//#include "test_div.h"

//#include "test_div_zero.h"

//#include "test_jal_jalr.h"
//#include "test_load_store.h"
//#include "test_lui_auipc.h"
//#include "test_mem.h"
//#include "test_mul.h"
//#include "test_mulh.h"
//#include "test_op.h"

//#include "test_op_imm.h"

//#include "test_rem.h"
//#include "test_sum.h"

//#include "test_op_fp.h"
//#include "test_load_store_fp.h"
//#include "test_load_store.h"

//#include "pi-leibniz.h"
//#include "../Compilation/Make/runnable_ad_proc_code.h"

#include "../Compilation/runnable_ad_proc_code.h"

//#include "../../Test_RISC_V/Make/runnable_ad_proc_code.h"
//#include "../../Vitis_HLS/ProcessorCode/adrv32imf_mp_ip.h"

//#include "test_ad_mem.h"
//#include "test_ad_mem_rosenbrock.h" //Rosenbrock
//#include "pi-leibniz.h"
//#include "test_ad_sine.h"
//#include "test_improv.h" //Rosenbrock

//#include "test_ad_math_sine.h"
//#include "test_ad_mem_sine.h"


#define PRINT_FIRST_VARS 10

char data_mem_0[DATA_MEM_SIZE/4];
char data_mem_1[DATA_MEM_SIZE/4];
char data_mem_2[DATA_MEM_SIZE/4];
char data_mem_3[DATA_MEM_SIZE/4];

#ifdef AD
char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];

void copy_code_to_data_mem (instruction_t code_mem[CODE_MEM_SIZE/sizeof(int)],
							char    *data_mem_0,
							char    *data_mem_1,
							char    *data_mem_2,
							char    *data_mem_3,
							char    ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
							char    ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
							char    ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
							char    ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT]){
	long i, j;
	for (i = 0; i < DATA_MEM_SIZE/sizeof(int); i++){
		if(i < CODE_MEM_SIZE/sizeof(int)){
			data_mem_0[i] = code_mem[i] & 0xff;
			data_mem_1[i] = (code_mem[i] >> 8) & 0xff;
			data_mem_2[i] = (code_mem[i] >> 16) & 0xff;
			data_mem_3[i] = (code_mem[i] >> 24) & 0xff;
		}
		else{
			data_mem_0[i] = 0;
			data_mem_1[i] = 0;
			data_mem_2[i] = 0;
			data_mem_3[i] = 0;
		}
		for(j = 0; j < AD_DERIV_CNT; j++){
			ad_data_mem_0[i][j] = 0;
			ad_data_mem_1[i][j] = 0;
			ad_data_mem_2[i][j] = 0;
			ad_data_mem_3[i][j] = 0;
		}
	}
}

void clear_data_mem (
		  char    *data_mem_0,
		  char    *data_mem_1,
		  char    *data_mem_2,
		  char    *data_mem_3,
		  char    ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
		  char    ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
		  char    ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
		  char    ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT]){
	long i, j;
	for (i = 0; i < DATA_MEM_SIZE/sizeof(int); i++){
		data_mem_0[i] = 0;
		data_mem_1[i] = 0;
		data_mem_2[i] = 0;
		data_mem_3[i] = 0;
		for(j = 0; j < AD_DERIV_CNT; j++){
			ad_data_mem_0[i][j] = 0;
			ad_data_mem_1[i][j] = 0;
			ad_data_mem_2[i][j] = 0;
			ad_data_mem_3[i][j] = 0;
		}
	}
}
#else
void copy_code_to_data_mem (instruction_t code_mem[CODE_MEM_SIZE/sizeof(int)],
							char    *data_mem_0,
							char    *data_mem_1,
							char    *data_mem_2,
							char    *data_mem_3){
	long i, j;
	for (i = 0; i < DATA_MEM_SIZE/sizeof(int); i++){
		if(i < CODE_MEM_SIZE/sizeof(int)){
			data_mem_0[i] = code_mem[i] & 0xff;
			data_mem_1[i] = (code_mem[i] >> 8) & 0xff;
			data_mem_2[i] = (code_mem[i] >> 16) & 0xff;
			data_mem_3[i] = (code_mem[i] >> 24) & 0xff;
		}
		else{
			data_mem_0[i] = 0;
			data_mem_1[i] = 0;
			data_mem_2[i] = 0;
			data_mem_3[i] = 0;
		}
	}
}

void clear_data_mem (
		  char    *data_mem_0,
		  char    *data_mem_1,
		  char    *data_mem_2,
		  char    *data_mem_3){
	long i, j;
	for (i = 0; i < DATA_MEM_SIZE/sizeof(int); i++){
		data_mem_0[i] = 0;
		data_mem_1[i] = 0;
		data_mem_2[i] = 0;
		data_mem_3[i] = 0;
	}
}
#endif

int main() {
  unsigned int  i;
  unsigned long int  nbi;
  unsigned long int  nbc;
  unsigned char b0, b1, b2, b3;
  int  			w;
  unsigned int  start_pc_local = 0;
#ifdef AD
  clear_data_mem(data_mem_0,
  			data_mem_1,
  			data_mem_2,
  			data_mem_3,
  			ad_data_mem_0,
  			ad_data_mem_1,
  			ad_data_mem_2,
  			ad_data_mem_3);
#ifdef UNIFIED_MEMORY
  copy_code_to_data_mem(code_mem,
		  	data_mem_0,
			data_mem_1,
			data_mem_2,
			data_mem_3,
			ad_data_mem_0,
			ad_data_mem_1,
			ad_data_mem_2,
			ad_data_mem_3);
#endif

#else 

  clear_data_mem(data_mem_0,
  			data_mem_1,
  			data_mem_2,
  			data_mem_3);
#ifdef UNIFIED_MEMORY
  copy_code_to_data_mem(code_mem,
		  	data_mem_0,
			data_mem_1,
			data_mem_2,
			data_mem_3);
#endif

#endif

#ifdef AD
#ifdef CUSTOM_START_PC
  adrv32imf_mp_ip(&__start_pc, code_mem, data_mem_0, data_mem_1, data_mem_2, data_mem_3, ad_data_mem_0, ad_data_mem_1, ad_data_mem_2, ad_data_mem_3, &nbi, &nbc);
#else
  adrv32imf_mp_ip(&start_pc_local, code_mem, data_mem_0, data_mem_1, data_mem_2, data_mem_3, ad_data_mem_0, ad_data_mem_1, ad_data_mem_2, ad_data_mem_3, &nbi, &nbc);
#endif
#else
#ifdef CUSTOM_START_PC
  adrv32imf_mp_ip(&__start_pc, code_mem, data_mem_0, data_mem_1, data_mem_2, data_mem_3, &nbi, &nbc);
#else
  adrv32imf_mp_ip(&start_pc_local, code_mem, data_mem_0, data_mem_1, data_mem_2, data_mem_3, &nbi, &nbc);
#endif
#endif

  printf("\ndata memory dump (non null words)\n");
  for (i=0; i<DATA_MEM_SIZE/4; i++){
    b0 = data_mem_0[i];
    b1 = data_mem_1[i];
    b2 = data_mem_2[i];
    b3 = data_mem_3[i];
    w = ((unsigned int)b3<<24) | ((unsigned int)b2<<16) | ((unsigned int)b1<< 8) | (unsigned int)b0;
    if (w != 0 || i < PRINT_FIRST_VARS){ // always print at least the first 10
    	//printf("m[%4d] = %16d (%8x) (%f)\n", i, w, (unsigned int)w, *(float*)(&w));
    	printf("m[%4d(%4x)] = %16d (%8x) (%.16f)\n", i, 4*i, w, (unsigned int)w, *(float*)(&w));
    }
#ifdef AD
    for (unsigned int j = 0; j<AD_DERIV_CNT; j++){
		b0 = ad_data_mem_0[i][j];
		b1 = ad_data_mem_1[i][j];
		b2 = ad_data_mem_2[i][j];
		b3 = ad_data_mem_3[i][j];
		w = ((unsigned int)b3<<24) |
			((unsigned int)b2<<16) |
			((unsigned int)b1<< 8)
		   | (unsigned int)b0;
		if (w != 0){
		  //printf("ad_m[%4d][%4d] = %16d (%8x) (%f)\n", i, j, w, (unsigned int)w, *(float*)(&w));
		  printf("ad_m[%4d(%4x)][%4d] = %16d (%8x) (%.16f)\n", i, 4*i, j, w, (unsigned int)w, *(float*)(&w));
		}
	}
#endif
    if (i == PRINT_FIRST_VARS-1){
      printf("====================================================\n");
    }
  }
  printf("\n%d fetched and decoded instructions\
 in %d cycles (ipc = %2.2f)\n", nbi, nbc, ((float)nbi)/nbc);

  return 0;
}
