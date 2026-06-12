#include <stdio.h>
#include "ad_multicycle_pipeline_ip.h"
#include "print.h"

//#include "test_branch.h"
//#include "test_jal_jalr.h"
//#include "test_load_store.h"
//#include "test_lui_auipc.h"
//#include "test_mem.h"
#include "test_ad_mem.h"

//Actually the next one should be the test file
//#include "test_ad_mem_auto.h"


//#include "test_op.h"
//#include "test_op_imm.h"
//#include "test_sum.h"

char    data_mem_0[DATA_MEM_SIZE/sizeof(int)];
char    data_mem_1[DATA_MEM_SIZE/sizeof(int)];
char    data_mem_2[DATA_MEM_SIZE/sizeof(int)];
char    data_mem_3[DATA_MEM_SIZE/sizeof(int)];
char    ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char    ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char    ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char    ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];

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
int main() {
  unsigned int  i, j;
  unsigned int  nbi;
  unsigned int  nbc;
  unsigned char b0, b1, b2, b3;
  int           w;
  clear_data_mem(data_mem_0,
  			data_mem_1,
  			data_mem_2,
  			data_mem_3,
  			ad_data_mem_0,
  			ad_data_mem_1,
  			ad_data_mem_2,
  			ad_data_mem_3);
  copy_code_to_data_mem(code_mem,
		  	data_mem_0,
			data_mem_1,
			data_mem_2,
			data_mem_3,
			ad_data_mem_0,
			ad_data_mem_1,
			ad_data_mem_2,
			ad_data_mem_3);
  ad_multicycle_pipeline_ip(
			0,
			code_mem,
			data_mem_0,
			data_mem_1,
			data_mem_2,
			data_mem_3,
			ad_data_mem_0,
			ad_data_mem_1,
			ad_data_mem_2,
			ad_data_mem_3,
			&nbi,
			&nbc);
  printf("\ndata memory dump (non null words)\n");
  for (i=0; i<DATA_MEM_SIZE/4; i++){
    b0 = data_mem_0[i];
    b1 = data_mem_1[i];
    b2 = data_mem_2[i];
    b3 = data_mem_3[i];
    w = ((unsigned int)b3<<24) |
        ((unsigned int)b2<<16) |
        ((unsigned int)b1<< 8) 
       | (unsigned int)b0;
    if (w != 0)
      printf("m[%4d] = %16d (%8x)\n", i, w, (unsigned int)w);
	for (j = 0; j<AD_DERIV_CNT; j++){
		b0 = ad_data_mem_0[i][j];
		b1 = ad_data_mem_1[i][j];
		b2 = ad_data_mem_2[i][j];
		b3 = ad_data_mem_3[i][j];
		w = ((unsigned int)b3<<24) |
			((unsigned int)b2<<16) |
			((unsigned int)b1<< 8)
		   | (unsigned int)b0;
		if (w != 0)
		  printf("ad_m[%4d][%4d] = %16d (%8x)\n", i, j, w, (unsigned int)w);
	}
  }
  printf("\n%d fetched and decoded instructions\
 in %d cycles (ipc = %2.2f)\n", nbi, nbc, ((float)nbi)/nbc);

  return 0;
}
