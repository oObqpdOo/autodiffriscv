#include "debug_multicycle_pipeline_ip.h"
#include "ad_multicycle_pipeline_ip.h"
#include "new_cycle.h"
#include "fetch.h"
#include "decode.h"
#include "issue.h"
#include "execute_1.h"
#include "execute_2.h"
#include "mem_access.h"
#include "wb.h"
#include "lock_unlock_reg.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include <stdio.h>
#endif
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_REG_FILE
#include "print.h"
#endif
#endif

static void init_reg_file(
  reg_t *reg_file){
  reg_num_p1_t r;
  max_der_t k;
  for (r=0; r<NB_REGISTER; r++){
	reg_file->is_reg_computed[r] = 0;
    reg_file->gp_reg_file[r] = 0;
    for (k=0; k<AD_DERIV_CNT; k++){
    	reg_file->ad_reg_file[r][k] = 0;
    }
  }
}

static void old_running_cond_update(
  from_e_2_to_w_t w_from_e_2,
  bit_t          *is_running){
#pragma HLS INLINE
  *is_running = ((!w_from_e_2.is_exit) &&
    (!w_from_e_2.is_valid ||
    !w_from_e_2.is_ret   ||
     w_from_e_2.result != 0));
}

static void running_cond_update(
  from_e_2_to_w_t w_from_e_2,
  bit_t          *is_running){
#pragma HLS INLINE
  *is_running = (
		  	!w_from_e_2.is_exit
			);/*  				 ||
		    !w_from_e_2.is_valid ||
		    !w_from_e_2.is_ret   ||
		     w_from_e_2.result != 0);*/
}

static void statistic_update(
  from_i_to_e_1_t e_1_from_i,
  counter_t      *nbi,
  counter_t      *nbc){
#pragma HLS INLINE
  *nbi = *nbi + (unsigned int)(e_1_from_i.is_valid);
  *nbc = *nbc + 1;
}

static void copy_global_mem_to_local(
	instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
	instruction_t  code_mem  [CODE_MEM_SIZE/sizeof(int)],
	char    g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
	char    g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
	char    g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
	char    g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
	char    g_ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	char    g_ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	char    g_ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	char    g_ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	data_mem_t    *data_mem
	){
	data_address_t r;
	code_address_t c;
	max_der_t k;
	for (r=0; r<DATA_MEM_SIZE/sizeof(int); r++){
	#pragma HLS PIPELINE II=128 //2
		data_mem->data_mem_0[r] = g_data_mem_0[r];
		data_mem->data_mem_1[r] = g_data_mem_1[r];
		data_mem->data_mem_2[r] = g_data_mem_2[r];
		data_mem->data_mem_3[r] = g_data_mem_3[r];
		for (k=0; k<AD_DERIV_CNT; k++){
			data_mem->ad_data_mem_0[r][k] = g_ad_data_mem_0[r][k];
			data_mem->ad_data_mem_1[r][k] = g_ad_data_mem_1[r][k];
			data_mem->ad_data_mem_2[r][k] = g_ad_data_mem_2[r][k];
			data_mem->ad_data_mem_3[r][k] = g_ad_data_mem_3[r][k];
		}
	}
	for (c=0; c<CODE_MEM_SIZE/sizeof(int); c++){
		code_mem[c] = g_code_mem[c];
	}
}

static void copy_local_mem_to_global(
	char    g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
	char    g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
	char    g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
	char    g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
	char    g_ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	char    g_ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	char    g_ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	char    g_ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	data_mem_t    *data_mem
	){
	data_address_t r;
	max_der_t k;
	for (r=0; r<DATA_MEM_SIZE/sizeof(int); r++){
	#pragma HLS PIPELINE II=128 //2
		g_data_mem_0[r] = data_mem->data_mem_0[r];
		g_data_mem_1[r] = data_mem->data_mem_1[r];
		g_data_mem_2[r] = data_mem->data_mem_2[r];
		g_data_mem_3[r] = data_mem->data_mem_3[r];
		for (k=0; k<AD_DERIV_CNT; k++){
			g_ad_data_mem_0[r][k] = data_mem->ad_data_mem_0[r][k];
			g_ad_data_mem_1[r][k] = data_mem->ad_data_mem_1[r][k];
			g_ad_data_mem_2[r][k] = data_mem->ad_data_mem_2[r][k];
			g_ad_data_mem_3[r][k] = data_mem->ad_data_mem_3[r][k];
		}
	}
}


void ad_multicycle_pipeline_ip(
  code_address_t start_pc,
  instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  //data_mem_t    *data_mem,
  char    g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
  char    g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
  char    g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
  char    g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
  char    g_ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    g_ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    g_ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    g_ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  unsigned int  *nb_instruction,
  unsigned int  *nb_cycle){
#pragma HLS INTERFACE s_axilite port=start_pc
#pragma HLS INTERFACE s_axilite port=g_code_mem //bundle=code_mem

//AD_ADATION:
//WARNING: [HLS 214-237] The INTERFACE pragma actions in object field. If on struct field, disaggregate pragma is required; If on array element, array_partition pragma is required. If no, this interface pragma will be viewed as invalid and ignored. In function 'ad_multicycle_pipeline_ip(ap_uint<11>, unsigned int*, data_mem_s*, unsigned int*, unsigned int*)' (ad_multicycle_pipeline_ip.cpp:59:0)
//#pragma HLS disaggregate variable=data_mem //https://www.xilinx.com/html_docs/xilinx2021_1/vitis_doc/hls_pragmas.html#lbk1584844390084

//FOR MINIZED FPGA

#pragma HLS INTERFACE s_axilite port=g_data_mem_0 //bundle=data_0 //latency=2 //storage_type=bram
#pragma HLS INTERFACE s_axilite port=g_data_mem_1 //bundle=data_0 //latency=2 //storage_type=bram
#pragma HLS INTERFACE s_axilite port=g_data_mem_2 //bundle=data_1 //latency=2 //storage_type=bram
#pragma HLS INTERFACE s_axilite port=g_data_mem_3 //bundle=data_1 //latency=2 //storage_type=bram
#pragma HLS INTERFACE s_axilite port=g_ad_data_mem_0 //bundle=ad_data_0 //latency=2 //storage_type=bram
#pragma HLS INTERFACE s_axilite port=g_ad_data_mem_1 //bundle=ad_data_0 //latency=2 //storage_type=bram
#pragma HLS INTERFACE s_axilite port=g_ad_data_mem_2 //bundle=ad_data_1 //latency=2 //storage_type=bram
#pragma HLS INTERFACE s_axilite port=g_ad_data_mem_3 //bundle=ad_data_1 //latency=2 //storage_type=bram

//FOR ALVEO CARD
/*
#pragma HLS INTERFACE m_axi port=g_data_mem_0 bundle=data_0 latency=2 //storage_type=bram
#pragma HLS INTERFACE m_axi port=g_data_mem_1 bundle=data_0 latency=2 //storage_type=bram
#pragma HLS INTERFACE m_axi port=g_data_mem_2 bundle=data_1 latency=2 //storage_type=bram
#pragma HLS INTERFACE m_axi port=g_data_mem_3 bundle=data_1 latency=2 //storage_type=bram
#pragma HLS INTERFACE m_axi port=g_ad_data_mem_0 bundle=ad_data_0 latency=2 //storage_type=bram
#pragma HLS INTERFACE m_axi port=g_ad_data_mem_1 bundle=ad_data_0 latency=2 //storage_type=bram
#pragma HLS INTERFACE m_axi port=g_ad_data_mem_2 bundle=ad_data_1 latency=2 //storage_type=bram
#pragma HLS INTERFACE m_axi port=g_ad_data_mem_3 bundle=ad_data_1 latency=2 //storage_type=bram
*/

//AD_ADATION_END:
#pragma HLS INTERFACE s_axilite port=nb_instruction
#pragma HLS INTERFACE s_axilite port=nb_cycle
#pragma HLS INTERFACE s_axilite port=return
//AD_ADATION:
//  int               reg_file[NB_REGISTER];
//#pragma HLS ARRAY_PARTITION variable=reg_file        dim=1 complete
//  bit_t             is_reg_computed[NB_REGISTER];
//#pragma HLS ARRAY_PARTITION variable=is_reg_computed dim=1 complete
  reg_t reg_file;
//see Prof. Goossens doc p. 110
#pragma HLS ARRAY_PARTITION variable=reg_file.gp_reg_file dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=reg_file.ad_reg_file dim=2 complete
//GOSH, I am stupid, dim=2 means only 2nd dimension will be partitioned...
#pragma HLS ARRAY_PARTITION variable=reg_file.ad_reg_file dim=0 complete
#pragma HLS ARRAY_PARTITION variable=reg_file.is_reg_computed dim=1 complete
//AD_ADATION_END:

//"LOCAL" memory
instruction_t code_mem  [CODE_MEM_SIZE/sizeof(int)];
data_mem_t    data_mem;
//PARTITION:
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_0 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_1 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_2 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_3 dim=0 type=block factor=4
//Hack to use pragma with defines (in main to partition memory): https://support.xilinx.com/s/article/46111?language=en_US
//#define PRAGMA_SUB(x) _Pragma (#x)
//#define DO_PRAGMA(x) PRAGMA_SUB(x)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_0 dim=2 type=block factor=AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_1 dim=2 type=block factor=AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_2 dim=2 type=block factor=AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_3 dim=2 type=block factor=AD_DERIV_CNT)
//COPY global to local memory
copy_global_mem_to_local(
					g_code_mem,
					code_mem,
					g_data_mem_0,
					g_data_mem_1,
					g_data_mem_2,
					g_data_mem_3,
					g_ad_data_mem_0,
					g_ad_data_mem_1,
					g_ad_data_mem_2,
					g_ad_data_mem_3,
					&data_mem);


  from_d_to_f_t     f_from_d;
  from_e_1_to_f_t   f_from_e_1;
  f_state_t         f_state;
  from_f_to_d_t     f_to_d;
  from_f_to_d_t     d_from_f;
  bit_t             d_state_is_full;
  d_state_t         d_state;
  from_d_to_f_t     d_to_f;
  from_d_to_i_t     d_to_i;
  from_d_to_i_t     i_from_d;
  bit_t             i_state_is_full;
  i_state_t         i_state;
  reg_num_t         i_destination;
  from_i_to_e_1_t   i_to_e_1;
  from_i_to_e_1_t   e_1_from_i;
  bit_t             e_1_state_is_full;
  e_1_state_t       e_1_state;
  from_e_1_to_f_t   e_1_to_f;
  from_e_1_to_m_t   e_1_to_m;
  from_e_1_to_e_2_t e_1_to_e_2;
  from_e_1_to_e_2_t e_2_from_e_1;
  bit_t             e_2_state_is_full;
  e_2_state_t       e_2_state;
  from_e_2_to_w_t   e_2_to_w;
  from_e_1_to_m_t   m_from_e_1;
  m_state_t         m_state;
  from_m_to_w_t     m_to_w;
  from_e_2_to_w_t   w_from_e_2;
  from_m_to_w_t     w_from_m;
  w_state_t         w_state;
  reg_num_t         w_destination;
  bit_t             is_running;
  counter_t         nbi;
  counter_t         nbc;
  init_f_state  (&f_state);
  init_d_state  (&d_state);
  init_i_state  (&i_state);
  init_e_1_state(&e_1_state);
  init_e_2_state(&e_2_state);
  init_reg_file (&reg_file);
  i_destination       = 0;
  w_destination       = 0;
  f_to_d.is_valid     = 0;
  d_to_f.target_pc    = start_pc;
  d_to_f.is_valid     = 1;
  d_to_i.is_valid     = 0;
  d_state_is_full     = 0;
  i_to_e_1.is_valid   = 0;
  i_state_is_full     = 0;
  e_1_to_f.is_valid   = 0;
  e_1_to_m.is_valid   = 0;
  e_1_to_e_2.is_valid = 0;
  e_1_state_is_full   = 0;
  e_2_to_w.is_valid   = 0;
  e_2_state_is_full   = 0;
  m_to_w.is_valid     = 0;
  nbi                 = 0;
  nbc                 = 0;

  do{
//AD_ADAPTION:
#pragma HLS PIPELINE II=2 //2
#pragma HLS LATENCY max=2 //1 cannot be honored any more due to multiplication?
//Original:
//#pragma HLS PIPELINE II=2
//#pragma HLS LATENCY max=1
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("==============================================\n");
    printf("cycle %d\n", (int)nbc);
#endif
#endif
    new_cycle(f_to_d,
              d_to_f,
              d_to_i,
              i_to_e_1,
              e_1_to_f,
              e_1_to_e_2,
              e_1_to_m,
              e_2_to_w,
              m_to_w,
             &f_from_d,
             &f_from_e_1,
             &d_from_f,
             &i_from_d,
             &e_1_from_i,
             &e_2_from_e_1,
             &m_from_e_1,
             &w_from_e_2,
             &w_from_m);
    fetch(f_from_d,
          f_from_e_1,
          d_state_is_full,
          code_mem,
         &f_state,
         &f_to_d);
    decode(d_from_f,
           i_state_is_full,
          &d_state,
          &d_to_f,
          &d_to_i,
          &d_state_is_full);
    issue(i_from_d,
          e_1_state_is_full,
         &i_destination,
         &reg_file,
         &i_state,
         &i_to_e_1,
         &i_state_is_full);
    execute_1(e_1_from_i,
              e_2_state_is_full,
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
             &reg_file,
#endif
#endif
             &e_1_state,
             &e_1_to_f,
             &e_1_to_m,
             &e_1_to_e_2,
             &e_1_state_is_full);
    execute_2(m_from_e_1.is_valid,
              e_2_from_e_1,
             &e_2_state,
             &e_2_to_w,
             &e_2_state_is_full);
    mem_access(m_from_e_1,
              &data_mem,
              &m_state,
              &m_to_w);
    write_back(w_from_e_2,
               w_from_m,
              &reg_file,
              &w_destination,
              &w_state);
    lock_unlock_reg(i_destination,
                    w_destination,
                    &reg_file);
    statistic_update(e_1_from_i, &nbi, &nbc);
    running_cond_update(w_from_e_2, &is_running);
  } while (is_running);
  //COPY local back to global memory
  copy_local_mem_to_global(
  					g_data_mem_0,
  					g_data_mem_1,
  					g_data_mem_2,
  					g_data_mem_3,
  					g_ad_data_mem_0,
  					g_ad_data_mem_1,
  					g_ad_data_mem_2,
  					g_ad_data_mem_3,
  					&data_mem);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  printf("==============================================\n");
#endif
#endif
  *nb_cycle       = nbc;
  *nb_instruction = nbi;
#ifndef __SYNTHESIS__
#ifdef DEBUG_REG_FILE
  print_reg(&reg_file);
#endif
#endif
}


void copy_array_8bit(
  char a[],
  char b[],
  max_der_t length){
#pragma HLS INLINE
//TODO: understand why the following creates extreme neg. slack due to dependencies?
//#pragma HLS ARRAY_PARTITION variable=a dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=b dim=1 complete
	max_der_t i;
	for(i = 0; i < length; i++){
	#pragma HLS unroll
	//TODO: PRAGMAS CHECK
	//#pragma HLS PIPELINE II=1
	//#pragma HLS LATENCY max=1
		*(a+i) = *(b+i);
	}
	return;
}

void copy_array_32bit(
  int a[],
  int b[],
  max_der_t length){
#pragma HLS INLINE
//TODO: understand why the following creates extreme neg. slack due to dependencies?
//#pragma HLS ARRAY_PARTITION variable=a dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=b dim=1 complete
	max_der_t i;
	for(i = 0; i < length; i++){
	#pragma HLS unroll
	//TODO: PRAGMAS CHECK
	//#pragma HLS PIPELINE II=1
	//#pragma HLS LATENCY max=1
		*(a+i) = *(b+i);
	}
	return;
}

void copy_value_8bit(
  char a[],
  char *b,
  max_der_t length){
#pragma HLS INLINE
//TODO: understand why the following creates extreme neg. slack due to dependencies?
//#pragma HLS ARRAY_PARTITION variable=a dim=1 complete
	max_der_t i;
	for(i = 0; i < length; i++){
	//TODO: PRAGMAS CHECK
	#pragma HLS unroll
	//#pragma HLS PIPELINE II=1
	//#pragma HLS LATENCY max=1
		*(a+i) = *(b);
	}
	return;
}

void copy_value_32bit(
  int a[],
  int b[],
  max_der_t length){
#pragma HLS INLINE
//TODO: understand why the following creates extreme neg. slack due to dependencies?
//#pragma HLS ARRAY_PARTITION variable=a dim=1 complete
	max_der_t i;
	for(i = 0; i < length; i++){
	//TODO: PRAGMAS CHECK
	#pragma HLS unroll
	//#pragma HLS PIPELINE II=1
	//#pragma HLS LATENCY max=1

		*(a+i) = *(b);
	}
	return;
}

//TODO: Not possible, HLS won't accept typecast on top of reg_file variables due to disaggregate pragma, n
//need for 2 separate functions
/*
void copy_value(
  void *a,
  void *b,
  unsigned long int length,
  unsigned char size){
	unsigned long int i;
	for(i = 0; i < length; i++){
	//TODO: PRAGMAS CHECK
	#pragma HLS PIPELINE II=1
	#pragma HLS LATENCY max=1
	#pragma HLS unroll
		if (length == 4)
			*((unsigned int *)a+(i*size)) = *((unsigned int *)b);
		else if (length == 1)
			*((unsigned char *)a+(i*size)) = *((unsigned char *)b);
	}
	return;
}*/

