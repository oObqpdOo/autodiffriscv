#include "debug_adrv32imf_mp_ip.h"
#include "adrv32imf_mp_ip.h"
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
#ifdef DEBUG_FREG_FILE
#include "print.h"
#endif
#ifdef DEBUG_FCSR_REG
#include "print.h"
#endif
#endif


#ifdef AD
#ifdef LOCAL_MEMORY
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
#endif//endif LOCAL_MEMORY

static void init_reg_file(
  reg_t *reg_file){
  reg_num_p1_t r;
  max_der_t k;
  reg_file->fcsr = 0;
  reg_file->is_fcsr_computed = 0;
  reg_file->fflag = 0;
  for (r=0; r<NB_REGISTER; r++){
	reg_file->is_reg_computed[r] = 0;
	reg_file->is_freg_computed[r] = 0;
    reg_file->reg_file[r] = 0;
    reg_file->freg_file[r] = 0.0;
    for (k=0; k<AD_DERIV_CNT; k++){
    	reg_file->ad_reg_file[r][k] = 0;
    	reg_file->ad_freg_file[r][k] = 0.0;
    }
  }
}
#else
static void init_reg_file(
  int   *reg_file,
  bit_t *is_reg_computed){
  reg_num_p1_t r;
  for (r=0; r<NB_REGISTER; r++){
    is_reg_computed[r] = 0;
    reg_file       [r] = 0;
  }
}
#ifdef FPU
static void init_freg_file(
  float *freg_file,
  int	*fcsr,
  bit_t *is_freg_computed,
  bit_t *is_fcsr_computed){
  reg_num_p1_t r;
  *is_fcsr_computed = 0;
  *fcsr = 0;
  for (r=0; r<NB_REGISTER; r++){
    is_freg_computed[r] = 0;
    freg_file       [r] = 0.0;
  }
}
#endif
#endif


#if defined(LOCAL_MEMORY) && !defined (AD)
static void copy_global_mem_to_local(
	instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
	instruction_t  code_mem  [CODE_MEM_SIZE/sizeof(int)],
	char    g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
	char    g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
	char    g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
	char    g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
	data_mem_t    *data_mem
	){
	data_address_t r;
	code_address_t c;
	for (r=0; r<DATA_MEM_SIZE/sizeof(int); r++){
	#pragma HLS PIPELINE II=128 //2
		data_mem->data_mem_0[r] = g_data_mem_0[r];
		data_mem->data_mem_1[r] = g_data_mem_1[r];
		data_mem->data_mem_2[r] = g_data_mem_2[r];
		data_mem->data_mem_3[r] = g_data_mem_3[r];

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
	data_mem_t    *data_mem
	){
	data_address_t r;
	for (r=0; r<DATA_MEM_SIZE/sizeof(int); r++){
	#pragma HLS PIPELINE II=128 //2
		g_data_mem_0[r] = data_mem->data_mem_0[r];
		g_data_mem_1[r] = data_mem->data_mem_1[r];
		g_data_mem_2[r] = data_mem->data_mem_2[r];
		g_data_mem_3[r] = data_mem->data_mem_3[r];
	}
}
#endif


#ifdef RUNNING_CONDITION_EXIT
static void running_cond_update(
  from_e_2_to_w_t w_from_e_2,
  bit_t          *is_running){
#pragma HLS INLINE
     *is_running = (!w_from_e_2.is_exit);
#ifndef __SYNTHESIS__
#ifdef DEBUG_EXIT
       printf("HELLO; RUNNING: %d\n", *is_running);
#endif
#endif
}
#else
static void running_cond_update(
  from_e_2_to_w_t w_from_e_2,
  bit_t          *is_running){
#pragma HLS INLINE off
  *is_running =
    !w_from_e_2.is_valid ||
    !w_from_e_2.is_ret   ||
     w_from_e_2.result.i != 0;
}
#endif

static void statistic_update(
  from_i_to_e_1_t e_1_from_i,
  counter_t      *nbi,
  counter_t      *nbc){
#pragma HLS INLINE off
  *nbi = *nbi + (unsigned int)(e_1_from_i.is_valid);
  *nbc = *nbc + 1;
}
static void update_mul_latency(
  bit_t          e_1_state_is_mul,
  mul_latency_t  mul_latency_in,
  mul_latency_t *mul_latency_out){
#pragma HLS INLINE off
  if (e_1_state_is_mul && (mul_latency_in != MUL_LATENCY))
	*mul_latency_out = mul_latency_in + 1;
}
static void update_div_latency(
  bit_t          e_1_state_is_div,
  div_latency_t  div_latency_in,
  div_latency_t *div_latency_out){
#pragma HLS INLINE off
  if (e_1_state_is_div && (div_latency_in != DIV_LATENCY))
	*div_latency_out = div_latency_in + 1;
}
#ifdef FPU
static void update_fpu_latency(
  bit_t          e_1_state_is_fpu,
  fpu_latency_t  fpu_latency_in,
  fpu_latency_t *fpu_latency_out){
#pragma HLS INLINE off
  if (e_1_state_is_fpu && (fpu_latency_in != FPU_LATENCY))
	*fpu_latency_out = fpu_latency_in + 1;
}
#endif


// DUMMY CODE FOR TESTING OPENCL INTERFACES AND BUFFERS
/*
void adrv32imf_mp_ip_dummy(
    int            start_pc,
    int            g_code_mem  [CODE_MEM_SIZE],
    char           g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
    char           g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
    char           g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
    char           g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
    char           g_ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)*AD_DERIV_CNT],
    char    	   g_ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)*AD_DERIV_CNT],
    char    	   g_ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)*AD_DERIV_CNT],
    char    	   g_ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)*AD_DERIV_CNT],
    unsigned int   *nb_instruction,
    unsigned int   *nb_cycle
)
{
    //#pragma HLS INTERFACE s_axilite port=start_pc
    #pragma HLS INTERFACE m_axi port=g_code_mem bundle=code0 latency=2
	#pragma HLS INTERFACE m_axi port=g_data_mem_0 bundle=data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_data_mem_1 bundle=data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_data_mem_2 bundle=data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_data_mem_3 bundle=data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_0 bundle=ad_data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_1 bundle=ad_data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_2 bundle=ad_data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_3 bundle=ad_data_1 latency=2 //storage_type=bram
    #pragma HLS INTERFACE m_axi port=nb_instruction
    #pragma HLS INTERFACE m_axi port=nb_cycle
    //#pragma HLS INTERFACE s_axilite port=return
    for(int i = 0; i < CODE_MEM_SIZE; ++i)
    {
        g_code_mem[i] = g_data_mem_0[i] + g_data_mem_1[i] + g_data_mem_2[i] + g_data_mem_3[i];
    }
    *nb_cycle = 100;
    *nb_instruction = 1000;
}*/


#ifdef __cplusplus
extern "C" {
#endif

void adrv32imf_mp_ip(
  //code_address_t start_pc,
  unsigned int  *start_pc_ptr,
#ifdef AD
  //instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  unsigned int   g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  char           g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
  char    		 g_ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    		 g_ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    		 g_ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    		 g_ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
#else
#ifdef LOCAL_MEMORY
  //instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  unsigned int   g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  char           g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
#else
  //instruction_t  code_mem  [CODE_MEM_SIZE/sizeof(int)],
  unsigned int   code_mem  [CODE_MEM_SIZE/sizeof(int)],
  char           data_mem_0[DATA_MEM_SIZE/sizeof(int)],
  char           data_mem_1[DATA_MEM_SIZE/sizeof(int)],
  char           data_mem_2[DATA_MEM_SIZE/sizeof(int)],
  char           data_mem_3[DATA_MEM_SIZE/sizeof(int)],
#endif
#endif
  unsigned int  *nb_instruction,
  unsigned int  *nb_cycle){

//#####################################################################################
// INTERFACES: S_AXI OR M_AXI
//#####################################################################################

//Lessons Learned: g_code_mem no bundle, no latency pls...

#ifdef AD
//#if defined(MINIZED_INSTEAD_OF_ALVEO) || defined (KRIA_INSTEAD_OF_ALVEO)
#ifdef USE_S_AXI_INTERFACE
    //#pragma HLS INTERFACE s_axilite port=start_pc 
	#pragma HLS INTERFACE s_axilite port=start_pc_ptr 
	#pragma HLS INTERFACE s_axilite port=g_code_mem
	#pragma HLS INTERFACE s_axilite port=g_data_mem_0 //bundle=data_0 //latency=2 //storage_type=bram
	#pragma HLS INTERFACE s_axilite port=g_data_mem_1 //bundle=data_0 //latency=2 //storage_type=bram
	#pragma HLS INTERFACE s_axilite port=g_data_mem_2 //bundle=data_1 //latency=2 //storage_type=bram
	#pragma HLS INTERFACE s_axilite port=g_data_mem_3 //bundle=data_1 //latency=2 //storage_type=bram
	#pragma HLS INTERFACE s_axilite port=g_ad_data_mem_0 //bundle=ad_data_0 //latency=2 //storage_type=bram
	#pragma HLS INTERFACE s_axilite port=g_ad_data_mem_1 //bundle=ad_data_0 //latency=2 //storage_type=bram
	#pragma HLS INTERFACE s_axilite port=g_ad_data_mem_2 //bundle=ad_data_1 //latency=2 //storage_type=bram
	#pragma HLS INTERFACE s_axilite port=g_ad_data_mem_3 //bundle=ad_data_1 //latency=2 //storage_type=bram
	#pragma HLS INTERFACE s_axilite port=nb_instruction
 	#pragma HLS INTERFACE s_axilite port=nb_cycle
	#pragma HLS INTERFACE s_axilite port=return
#else //FOR ALVEO CARD, no USE_S_AXI_INTERFACE
	//#pragma HLS INTERFACE s_axilite port=g_code_mem 
    //#pragma HLS INTERFACE m_axi port=start_pc //not needed for OpenCL
#ifdef PRAGMA_USE_BRAM_ALVEO
    #pragma HLS INTERFACE m_axi port=start_pc_ptr
	#pragma HLS INTERFACE m_axi port=g_code_mem   //bundle=code_mem latency=2
	#pragma HLS INTERFACE m_axi port=g_data_mem_0 bundle=data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_data_mem_1 bundle=data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_data_mem_2 bundle=data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_data_mem_3 bundle=data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_0 bundle=ad_data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_1 bundle=ad_data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_2 bundle=ad_data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_3 bundle=ad_data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=nb_instruction
	#pragma HLS INTERFACE m_axi port=nb_cycle
	//#pragma HLS INTERFACE s_axilite port=return
#else //PRAGMA_USE_BRAM_ALVEO
    #pragma HLS INTERFACE m_axi port=start_pc_ptr
	#pragma HLS INTERFACE m_axi port=g_code_mem   //bundle=code_mem latency=2
	#pragma HLS INTERFACE m_axi port=g_data_mem_0 bundle=data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_data_mem_1 bundle=data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_data_mem_2 bundle=data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_data_mem_3 bundle=data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_0 bundle=ad_data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_1 bundle=ad_data_0 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_2 bundle=ad_data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=g_ad_data_mem_3 bundle=ad_data_1 latency=2 //storage_type=bram
	#pragma HLS INTERFACE m_axi port=nb_instruction
	#pragma HLS INTERFACE m_axi port=nb_cycle
	//#pragma HLS INTERFACE s_axilite port=return
#endif //PRAGMA_USE_BRAM_ALVEO

#endif

#else //#ifndef AD

//#if defined(MINIZED_INSTEAD_OF_ALVEO) || defined (KRIA_INSTEAD_OF_ALVEO)
#ifdef USE_S_AXI_INTERFACE
#ifdef LOCAL_MEMORY
	#pragma HLS INTERFACE s_axilite port=g_code_mem
	#pragma HLS INTERFACE s_axilite port=g_data_mem_0
	#pragma HLS INTERFACE s_axilite port=g_data_mem_1
	#pragma HLS INTERFACE s_axilite port=g_data_mem_2
	#pragma HLS INTERFACE s_axilite port=g_data_mem_3
#else //LOCAL_MEMORY
	#pragma HLS INTERFACE s_axilite port=code_mem
	#pragma HLS INTERFACE s_axilite port=data_mem_0
	#pragma HLS INTERFACE s_axilite port=data_mem_1
	#pragma HLS INTERFACE s_axilite port=data_mem_2
	#pragma HLS INTERFACE s_axilite port=data_mem_3
#endif
#else //if no USE_S_AXI_INTERFACE
#ifdef LOCAL_MEMORY
	#pragma HLS INTERFACE m_axi port=g_code_mem   //bundle=code_0 latency=2 
	#pragma HLS INTERFACE m_axi port=g_data_mem_0 bundle=data_0 latency=2 
	#pragma HLS INTERFACE m_axi port=g_data_mem_1 bundle=data_1 latency=2 
	#pragma HLS INTERFACE m_axi port=g_data_mem_2 bundle=data_2 latency=2 
	#pragma HLS INTERFACE m_axi port=g_data_mem_3 bundle=data_3 latency=2 
#else //LOCAL_MEMORY
	#pragma HLS INTERFACE m_axi port=code_mem   //bundle=code_0 latency=2 
	#pragma HLS INTERFACE m_axi port=data_mem_0 bundle=data_0 latency=2 
	#pragma HLS INTERFACE m_axi port=data_mem_1 bundle=data_1 latency=2 
	#pragma HLS INTERFACE m_axi port=data_mem_2 bundle=data_2 latency=2 
	#pragma HLS INTERFACE m_axi port=data_mem_3 bundle=data_3 latency=2 
#endif
#endif //endif USE_S_AXI_INTERFACE

//#if defined(MINIZED_INSTEAD_OF_ALVEO) || defined (KRIA_INSTEAD_OF_ALVEO)
 #ifdef USE_S_AXI_INTERFACE
    //#pragma HLS INTERFACE s_axilite port=start_pc //not needed for OpenCL
	#pragma HLS INTERFACE s_axilite port=start_pc_ptr 
	#pragma HLS INTERFACE s_axilite port=nb_instruction
	#pragma HLS INTERFACE s_axilite port=nb_cycle
	#pragma HLS INTERFACE s_axilite port=return
#else
    //#pragma HLS INTERFACE m_axi port=start_pc //not needed for OpenCL
	#pragma HLS INTERFACE m_axi port=start_pc_ptr //not needed for OpenCL
	#pragma HLS INTERFACE m_axi port=nb_instruction
	#pragma HLS INTERFACE m_axi port=nb_cycle
	//pragma HLS INTERFACE s_axilite port=return
#endif
#endif //#ifdef AD

//TODO: passing via OpenCL failed previously, now passing a pointer
int start_pc = *start_pc_ptr;



//######################################################################################################
// IF USING AD, PARTITION ARRAYS
//######################################################################################################
#ifdef AD

//########################################################
//PARTITION CODE MEMORY:
//########################################################

//  int               reg_file[NB_REGISTER];
//#pragma HLS ARRAY_PARTITION variable=reg_file        dim=1 complete
//  bit_t             is_reg_computed[NB_REGISTER];
//#pragma HLS ARRAY_PARTITION variable=is_reg_computed dim=1 complete
reg_t reg_file;
//see Prof. Goossens doc p. 110
#pragma HLS ARRAY_PARTITION variable=reg_file.reg_file dim=1 complete
#pragma HLS ARRAY_PARTITION variable=reg_file.freg_file dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=reg_file.ad_reg_file dim=2 complete
//GOSH, I am stupid, dim=2 means only 2nd dimension will be partitioned...
#pragma HLS ARRAY_PARTITION variable=reg_file.ad_reg_file dim=0 complete
#pragma HLS ARRAY_PARTITION variable=reg_file.ad_freg_file dim=0 complete
#pragma HLS ARRAY_PARTITION variable=reg_file.is_reg_computed dim=1 complete
#pragma HLS ARRAY_PARTITION variable=reg_file.is_freg_computed dim=1 complete
//"LOCAL" memory meant to stick to BRAM
//TODO: in Vitis flow in contrast to Vivado flow, this is in HBM apparently... 

#ifdef LOCAL_MEMORY
instruction_t code_mem[CODE_MEM_SIZE/sizeof(int)];
data_mem_t    data_mem;
#endif

//########################################################
//PARTITION CODE MEMORY:
//ATTENTION: THIS is a really stupid idea
//reason - it changes the M_AXI interface from 1 int array to 4 arrays
//So: leave it as it was ... unpartitioned
//########################################################
/*
#ifdef LOCAL_MEMORY
//#pragma HLS ARRAY_PARTITION variable=g_code_mem type=complete
#pragma HLS ARRAY_PARTITION variable=g_code_mem dim=0 type=block factor=4
#else //LOCAL_MEMORY
#ifdef AD //TODO: I think this is already in AD block
//#pragma HLS ARRAY_PARTITION variable=code_mem type=complete
#pragma HLS ARRAY_PARTITION variable=g_code_mem dim=0 type=block factor=4
#else //AD
//#pragma HLS ARRAY_PARTITION variable=code_mem type=complete
#pragma HLS ARRAY_PARTITION variable=code_mem dim=0 type=block factor=4
#endif //AD
#endif //LOCAL_MEMORY
*/

//#######################################
//Hack to use pragma with defines (in main to partition memory): https://support.xilinx.com/s/article/46111?language=en_US
//defined in adrv32imf_mp_ip.h
//#######################################
//#define PRAGMA_SUB(x) _Pragma (#x)
//#define DO_PRAGMA(x) PRAGMA_SUB(x)

//########################################################
//PARTITION DATA MEMORY:
//########################################################
#ifdef LOCAL_MEMORY
//*************
//ARRAY PARTITION FOR NORMAL MEMORY, when using LOCAL_MEMORY
//*************
#ifdef PRAGMA_USE_BRAM_ALVEO
//Stick to BRAM? Trying INTERFACE bram, alternatively try ARRAY_PARTITION type=complete
/*#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_0 dim=0 type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_1 dim=0 type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_2 dim=0 type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_3 dim=0 type=complete*/
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_0 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_1 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_2 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_3 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
/*
#pragma HLS ARRAY_RESHAPE variable=data_mem.data_mem_0 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_RESHAPE variable=data_mem.data_mem_1 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_RESHAPE variable=data_mem.data_mem_2 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_RESHAPE variable=data_mem.data_mem_3 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
*/
/*#pragma HLS ARRAY_RESHAPE variable=data_mem.data_mem_0 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_RESHAPE variable=data_mem.data_mem_1 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_RESHAPE variable=data_mem.data_mem_2 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_RESHAPE variable=data_mem.data_mem_3 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4*/
#pragma HLS bind_storage variable=data_mem.data_mem_0 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem.data_mem_1 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem.data_mem_2 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem.data_mem_3 type=RAM_T2P impl=BRAM latency=1
/*#pragma HLS INTERFACE bram port=data_mem.data_mem_0
#pragma HLS INTERFACE bram port=data_mem.data_mem_1
#pragma HLS INTERFACE bram port=data_mem.data_mem_2
#pragma HLS INTERFACE bram port=data_mem.data_mem_3*/
#else //original, NO PRAGMA_USE_BRAM_ALVEO
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_0 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_1 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_2 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_3 dim=0 type=block factor=4
#endif //endif PRAGMA_USE_BRAM_ALVEO
//*************
//ARRAY PARTITION FOR AD MEMORY, when using LOCAL_MEMORY
//*************
#ifdef PRAGMA_USE_BRAM_ALVEO
//Stick to BRAM? Trying INTERFACE bram, alternatively try ARRAY_PARTITION type=complete
/*#pragma HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_0 dim=2 type=complete
#pragma HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_1 dim=2 type=complete
#pragma HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_2 dim=2 type=complete
#pragma HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_3 dim=2 type=complete*/
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_0 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_1 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_2 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_3 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
/*DO_PRAGMA(HLS ARRAY_RESHAPE variable=data_mem.ad_data_mem_0 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_RESHAPE variable=data_mem.ad_data_mem_1 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_RESHAPE variable=data_mem.ad_data_mem_2 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_RESHAPE variable=data_mem.ad_data_mem_3 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)*/
/*DO_PRAGMA(HLS ARRAY_RESHAPE variable=data_mem.ad_data_mem_0 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_RESHAPE variable=data_mem.ad_data_mem_1 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_RESHAPE variable=data_mem.ad_data_mem_2 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_RESHAPE variable=data_mem.ad_data_mem_3 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)*/
#pragma HLS bind_storage variable=data_mem.ad_data_mem_0 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem.ad_data_mem_1 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem.ad_data_mem_2 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem.ad_data_mem_3 type=RAM_T2P impl=BRAM latency=1
/*#pragma HLS INTERFACE bram port=data_mem.ad_data_mem_0
#pragma HLS INTERFACE bram port=data_mem.ad_data_mem_1
#pragma HLS INTERFACE bram port=data_mem.ad_data_mem_2
#pragma HLS INTERFACE bram port=data_mem.ad_data_mem_3*/
#else //original NO PRAGMA_USE_BRAM_ALVEO
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_0 dim=2 type=block factor=AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_1 dim=2 type=block factor=AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_2 dim=2 type=block factor=AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=data_mem.ad_data_mem_3 dim=2 type=block factor=AD_DERIV_CNT)
#endif //endif PRAGMA_USE_BRAM_ALVEO
//=====================================================================================================================
#else //else NO LOCAL_MEMORY
//=====================================================================================================================
//*************
//ARRAY PARTITION FOR AD MEMORY, when using LOCAL_MEMORY
//*************
#ifdef PRAGMA_USE_BRAM_ALVEO 
//Stick to BRAM? Trying INTERFACE bram, alternatively try ARRAY_PARTITION type=complete
/*#pragma HLS ARRAY_PARTITION variable=g_data_mem_0 dim=0 type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_1 dim=0 type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_2 dim=0 type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_3 dim=0 type=complete*/
#pragma HLS ARRAY_PARTITION variable=g_data_mem_0 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=g_data_mem_1 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=g_data_mem_2 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=g_data_mem_3 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
/*#pragma HLS ARRAY_RESHAPE variable=g_data_mem_0 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_RESHAPE variable=g_data_mem_1 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_RESHAPE variable=g_data_mem_2 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_RESHAPE variable=g_data_mem_3 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4*/
#pragma HLS bind_storage variable=g_data_mem_0 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=g_data_mem_1 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=g_data_mem_2 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=g_data_mem_3 type=RAM_T2P impl=BRAM latency=1
/*#pragma HLS INTERFACE bram port=g_ad_data_mem_0
#pragma HLS INTERFACE bram port=g_ad_data_mem_1
#pragma HLS INTERFACE bram port=g_ad_data_mem_2
#pragma HLS INTERFACE bram port=g_ad_data_mem_3*/
#else  //original NO PRAGMA_USE_BRAM_ALVEO
#pragma HLS ARRAY_PARTITION variable=g_data_mem_0 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=g_data_mem_1 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=g_data_mem_2 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=g_data_mem_3 dim=0 type=block factor=4
#endif //endif PRAGMA_USE_BRAM_ALVEO
//*************
//ARRAY PARTITION FOR AD MEMORY, when NOT using LOCAL_MEMORY
//*************
#ifdef PRAGMA_USE_BRAM_ALVEO
//Stick to BRAM? Trying INTERFACE bram, alternatively try ARRAY_PARTITION type=complete
/*#pragma HLS ARRAY_PARTITION variable=g_ad_data_mem_0 dim=2 type=complete
#pragma HLS ARRAY_PARTITION variable=g_ad_data_mem_1 dim=2 type=complete
#pragma HLS ARRAY_PARTITION variable=g_ad_data_mem_2 dim=2 type=complete
#pragma HLS ARRAY_PARTITION variable=g_ad_data_mem_3 dim=2 type=complete*/
DO_PRAGMA(HLS ARRAY_PARTITION variable=g_ad_data_mem_0 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=g_ad_data_mem_1 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=g_ad_data_mem_2 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=g_ad_data_mem_3 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
/*
DO_PRAGMA(HLS ARRAY_RESHAPE variable=g_ad_data_mem_0 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_RESHAPE variable=g_ad_data_mem_1 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_RESHAPE variable=g_ad_data_mem_2 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_RESHAPE variable=g_ad_data_mem_3 dim=2 type=block factor=ARRAY_PARTITION_FACTOR*AD_DERIV_CNT)
*/
#pragma HLS bind_storage variable=g_ad_data_mem_0 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=g_ad_data_mem_1 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=g_ad_data_mem_2 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=g_ad_data_mem_3 type=RAM_T2P impl=BRAM latency=1
/*#pragma HLS INTERFACE bram port=g_ad_data_mem_0
#pragma HLS INTERFACE bram port=g_ad_data_mem_1
#pragma HLS INTERFACE bram port=g_ad_data_mem_2
#pragma HLS INTERFACE bram port=g_ad_data_mem_3*/
#else //original NO PRAGMA_USE_BRAM_ALVEO
DO_PRAGMA(HLS ARRAY_PARTITION variable=g_ad_data_mem_0 dim=2 type=block factor=AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=g_ad_data_mem_1 dim=2 type=block factor=AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=g_ad_data_mem_2 dim=2 type=block factor=AD_DERIV_CNT)
DO_PRAGMA(HLS ARRAY_PARTITION variable=g_ad_data_mem_3 dim=2 type=block factor=AD_DERIV_CNT)
#endif //endif PRAGMA_USE_BRAM_ALVEO
#endif //endif LOCAL_MEMORY


//########################################################
// Start copying global to local if LOCAL_MEMORY
//########################################################

#ifdef LOCAL_MEMORY
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
#endif


//######################################################################################################
// IF NOT! USING AD, PARTITION ARRAYS? What about DATA MEMORY in this case?
//######################################################################################################

#else //if NO AD

  int            	reg_file       [NB_REGISTER];
#pragma HLS ARRAY_PARTITION variable=reg_file        dim=1 complete
  bit_t          	is_reg_computed[NB_REGISTER];
#pragma HLS ARRAY_PARTITION variable=is_reg_computed dim=1 complete
#ifdef FPU
  float          	freg_file[NB_REGISTER];
#pragma HLS ARRAY_PARTITION variable=freg_file        dim=1 complete
  bit_t          	is_freg_computed[NB_REGISTER];
#pragma HLS ARRAY_PARTITION variable=is_freg_computed dim=1 complete
  int            	fcsr;
  bit_t			 	is_fcsr_computed;
  int            	fflag;
#endif

/*
#ifdef LOCAL_MEMORY
#pragma HLS ARRAY_PARTITION variable=g_code_mem   dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_0 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_1 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_2 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_3 dim=0 type=block factor=4 //type=complete 
#else //LOCAL_MEMORY
#pragma HLS ARRAY_PARTITION variable=code_mem   dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem_0 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem_1 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem_2 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem_3 dim=0 type=block factor=4 //type=complete 
#endif //LOCAL_MEMORY
*/


#ifdef LOCAL_MEMORY //#if defined(LOCAL_MEMORY) && !defined (AD)
  //"LOCAL" memory
  instruction_t 	code_mem  [CODE_MEM_SIZE/sizeof(int)];
  data_mem_t    	data_mem;


#ifdef PRAGMA_USE_BRAM_ALVEO

/*
#pragma HLS ARRAY_PARTITION variable=g_code_mem   dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_0 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_1 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_2 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_3 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4 //type=complete 
*/
//#pragma HLS ARRAY_PARTITION variable=code_mem            dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_0 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_1 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_2 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_3 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS bind_storage variable=data_mem.data_mem_0 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem.data_mem_1 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem.data_mem_2 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem.data_mem_3 type=RAM_T2P impl=BRAM latency=1

#else //PRAGMA_USE_BRAM_ALVEO

/*
#pragma HLS ARRAY_PARTITION variable=g_code_mem   dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_0 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_1 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_2 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=g_data_mem_3 dim=0 type=block factor=4 //type=complete 
#pragma HLS interface bram port=code_mem 
#pragma HLS interface bram port=data_mem.data_mem_0 
#pragma HLS interface bram port=data_mem.data_mem_1 
#pragma HLS interface bram port=data_mem.data_mem_2 
#pragma HLS interface bram port=data_mem.data_mem_3 
*/

//#pragma HLS ARRAY_PARTITION variable=code_mem            dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_0 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_1 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_2 dim=0 type=block factor=4
#pragma HLS ARRAY_PARTITION variable=data_mem.data_mem_3 dim=0 type=block factor=4

    //COPY global to local memory
    copy_global_mem_to_local(
                        g_code_mem,
                        code_mem,
                        g_data_mem_0,
                        g_data_mem_1,
                        g_data_mem_2,
                        g_data_mem_3,
                        &data_mem);
#endif //PRAGMA_USE_BRAM_ALVEO

#else //NO LOCAL_MEMORY

#ifdef PRAGMA_USE_BRAM_ALVEO

#pragma HLS ARRAY_PARTITION variable=data_mem_0 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=data_mem_1 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=data_mem_2 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS ARRAY_PARTITION variable=data_mem_3 dim=0 type=block factor=ARRAY_PARTITION_FACTOR*4
#pragma HLS bind_storage variable=data_mem_0 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem_1 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem_2 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=data_mem_3 type=RAM_T2P impl=BRAM latency=1

#else //PRAGMA_USE_BRAM_ALVEO

#pragma HLS ARRAY_PARTITION variable=code_mem   dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem_0 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem_1 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem_2 dim=0 type=block factor=4 //type=complete 
#pragma HLS ARRAY_PARTITION variable=data_mem_3 dim=0 type=block factor=4 //type=complete 

#endif //PRAGMA_USE_BRAM_ALVEO
#endif //LOCAL_MEMORY
#endif //AD
#ifdef FETCH_EVERY_CYCLE
  from_f_to_f_t     f_from_f;
#endif
  from_d_to_f_t     f_from_d;
  from_e_1_to_f_t   f_from_e_1;
  bit_t             f_state_is_full_in = 0;
  bit_t             f_state_is_full_out = 0;
  f_state_t         f_state;
  from_f_to_d_t     f_to_d;
  from_f_to_d_t     d_from_f;
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
  bit_t             d_state_is_full_in = 0;
  bit_t             d_state_is_full_out = 0;
#else
  bit_t             d_state_is_full = 0;
#endif
  d_state_t         d_state;
  from_f_to_f_t     f_to_f;
  from_d_to_f_t     d_to_f;
  from_d_to_i_t     d_to_i;
  from_d_to_i_t     i_from_d;
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
  bit_t             i_state_is_full_in = 0;
  bit_t             i_state_is_full_out = 0;
#else
  bit_t             i_state_is_full = 0;
#endif
  i_state_t         i_state;
  reg_num_t         i_destination;
#ifdef FPU
  bit_t				i_type;
#endif
  from_i_to_e_1_t   i_to_e_1;
  from_i_to_e_1_t   e_1_from_i;
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
  bit_t             e_1_state_is_full_in = 0;
  bit_t             e_1_state_is_full_out = 0;
#else
  bit_t             e_1_state_is_full;
#endif
  e_1_state_t       e_1_state;
  int               mul_result_in [1];
  int               mul_result_out[1];
#ifdef AD
  //TODO: WHAT ABOUT THE HLS DEPENDENCE PRAGMA HERE???
  float_int_t		ad_mul_result_in[AD_DERIV_CNT];
  float_int_t		ad_mul_result_out[AD_DERIV_CNT];
#pragma HLS DEPENDENCE type=inter variable=ad_mul_result_out dependent=false
#ifdef FIX_II_2
//This alone is not sufficient
//Leave this here, even if I get a warning "Cannot apply memory assignment of "RAM_T2P_BRAM"", otherwise II=38!
#pragma HLS BIND_STORAGE variable=ad_mul_result_out type=RAM_T2P //latency=1 impl=AUTO//latency=1 impl=AUTO
//This alone is ALSO not sufficient, something worsened fpu latency by 2 from 5 to 7 AND increases resources!
#pragma HLS ARRAY_RESHAPE variable=ad_mul_result_out type=complete //dim=0 
#endif
#endif
#pragma HLS DEPENDENCE type=inter variable=mul_result_out dependent=false
#ifdef FIX_II_2
//This alone is not sufficient
//Leave this here, even if I get a warning "Cannot apply memory assignment of "RAM_T2P_BRAM"", otherwise II=38!
#pragma HLS BIND_STORAGE variable=mul_result_out type=RAM_T2P //latency=1 impl=AUTO//latency=1 impl=AUTO
//This alone is ALSO not sufficient, something worsened fpu latency by 2 from 5 to 7 AND increases resources!
#pragma HLS ARRAY_RESHAPE variable=mul_result_out type=complete //dim=0 
#endif
  int               div_result_in [1];
  int               div_result_out[1];
#ifdef AD
  //TODO: WHAT ABOUT THE HLS DEPENDENCE PRAGMA HERE???
  float_int_t		ad_div_result_in[AD_DERIV_CNT];
  float_int_t		ad_div_result_out[AD_DERIV_CNT];
#pragma HLS DEPENDENCE type=inter variable=ad_div_result_out dependent=false
#ifdef FIX_II_2
//This alone is not sufficient
//Leave this here, even if I get a warning "Cannot apply memory assignment of "RAM_T2P_BRAM"", otherwise II=38!
#pragma HLS BIND_STORAGE variable=ad_div_result_out type=RAM_T2P //latency=1  impl=AUTO//latency=1 impl=AUTO
//This alone is ALSO not sufficient, something worsened fpu latency by 2 from 5 to 7 AND increases resources!
#pragma HLS ARRAY_RESHAPE variable=ad_div_result_out type=complete //dim=0 
#endif
#endif
#pragma HLS DEPENDENCE type=inter variable=div_result_out dependent=false
#ifdef FIX_II_2
//This alone is not sufficient
//Leave this here, even if I get a warning "Cannot apply memory assignment of "RAM_T2P_BRAM"", otherwise II=38!
#pragma HLS BIND_STORAGE variable=div_result_out type=RAM_T2P //latency=1 impl=AUTO//latency=1 impl=AUTO 
//This alone is ALSO not sufficient, something worsened fpu latency by 2 from 5 to 7 AND increases resources!
#pragma HLS ARRAY_RESHAPE variable=div_result_out type=complete //dim=0 
#endif
#ifdef FPU
  float_int_t       fpu_result_in [1];
  float_int_t       fpu_result_out[1];
#ifdef AD
  //TODO: WHAT ABOUT THE HLS DEPENDENCE PRAGMA HERE???
  float_int_t		ad_fpu_result_in[AD_DERIV_CNT];
  float_int_t		ad_fpu_result_out[AD_DERIV_CNT];
#pragma HLS DEPENDENCE type=inter variable=ad_fpu_result_out dependent=false
#ifdef FIX_II_2
//This alone is not sufficient
//Leave this here, even if I get a warning "Cannot apply memory assignment of "RAM_T2P_BRAM"", otherwise II=38!
#pragma HLS BIND_STORAGE variable=ad_fpu_result_out type=RAM_T2P //latency=1 impl=AUTO//impl=AUTO 
//This alone is ALSO not sufficient, something worsened fpu latency by 2 from 5 to 7 AND increases resources!
#pragma HLS ARRAY_RESHAPE variable=ad_fpu_result_out type=complete //dim=0
#endif
#endif
#pragma HLS DEPENDENCE type=inter variable=fpu_result_out dependent=false
#ifdef FIX_II_2
//This alone is not sufficient
//Leave this here, even if I get a warning "Cannot apply memory assignment of "RAM_T2P_BRAM"", otherwise II=38!
#pragma HLS BIND_STORAGE variable=fpu_result_out type=RAM_T2P //latency=1  impl=AUTO//latency=1 impl=AUTO 
//This alone is ALSO not sufficient, something worsened fpu latency by 2 from 5 to 7 AND increases resources!
#pragma HLS ARRAY_RESHAPE variable=fpu_result_out type=complete //dim=0 
#endif
#endif
  bit_t             is_mul_in_flight_in;
  bit_t             is_mul_in_flight_out;
  bit_t             is_div_in_flight_in;
  bit_t             is_div_in_flight_out;
#ifdef FPU
  bit_t             is_fpu_in_flight_in;
  bit_t             is_fpu_in_flight_out;
#endif
  from_e_1_to_f_t   e_1_to_f;
  from_e_1_to_m_t   e_1_to_m;
  from_e_1_to_e_2_t e_1_to_e_2;
  from_e_1_to_e_2_t e_2_from_e_1;
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
  bit_t             e_2_state_is_full_in = 0;
  bit_t             e_2_state_is_full_out = 0;
#else
  bit_t             e_2_state_is_full;
#endif
  e_2_state_t       e_2_state;
  from_e_2_to_w_t   e_2_to_w;
  from_e_1_to_m_t   m_from_e_1;
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
  bit_t             m_state_is_full_in = 0;
  bit_t             m_state_is_full_out = 0;
#else
  bit_t             m_state_is_full;
#endif
  m_state_t         m_state;
  from_m_to_w_t     m_to_w;
  from_e_2_to_w_t   w_from_e_2;
  from_m_to_w_t     w_from_m;
  w_state_t         w_state;
  reg_num_t         w_destination;
#ifdef FPU
  bit_t				wb_type;
#endif
  mul_latency_t     mul_latency_in;
  mul_latency_t     mul_latency_out;
  div_latency_t     div_latency_in;
  div_latency_t     div_latency_out;
#ifdef FPU
  fpu_latency_t     fpu_latency_in;
  fpu_latency_t     fpu_latency_out;
#endif
  bit_t             is_running = 1;
  counter_t         nbi;
  counter_t         nbc;

  init_f_state  (&f_state);
  init_d_state  (&d_state);
  init_i_state  (&i_state);
  init_e_1_state(&e_1_state);
  init_e_2_state(&e_2_state);
  init_m_state  (&m_state);
#ifdef AD
  init_reg_file(&reg_file);
#else
  init_reg_file (reg_file, is_reg_computed);
#ifdef FPU
  init_freg_file(freg_file, &fcsr, is_freg_computed, &is_fcsr_computed);
#endif
#endif
  clear_cycle(
#ifdef FETCH_EVERY_CYCLE
   &(f_to_f.is_valid),
#endif
   &(f_to_d.is_valid),
   &(d_to_f.is_valid),
   &(d_to_i.is_valid),
   &(i_to_e_1.is_valid),
   &(e_1_to_f.is_valid),
   &(e_1_to_e_2.is_valid),
   &(e_1_to_m.is_valid),
   &(e_2_to_w.is_valid),
   &(m_to_w.is_valid)
#ifdef RUNNING_CONDITION_EXIT
   ,&(w_from_e_2.is_exit)
#endif
   );
  i_destination        = 0;
  w_destination        = 0;
  mul_latency_out      = 0;
  div_latency_out      = 0;
  f_state_is_full_out  = 0;
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
  d_state_is_full_out  = 0;
  i_state_is_full_out  = 0;
  e_1_state_is_full_out= 0;
  e_2_state_is_full_out= 0;
  m_state_is_full_out  = 0;
#else 
  d_state_is_full      = 0;
  i_state_is_full      = 0;
  e_1_state_is_full    = 0;
  e_2_state_is_full    = 0;
  m_state_is_full      = 0;
#endif
  is_mul_in_flight_out = 0;
  is_div_in_flight_out = 0;
#ifdef FPU
  is_fpu_in_flight_out = 0;
#endif
  nbi                  = 0;
  nbc                  = 0;
#ifdef FPU
  wb_type 			   = WB_TYPE_NO_FP;
  i_type			   = I_TYPE_NO_FP;
#endif

#ifdef FETCH_EVERY_CYCLE
  f_to_f.is_valid      = 1;
  f_to_f.next_pc       = start_pc;
#else
  d_to_f.is_valid    = 1;
  d_to_f.target_pc   = start_pc;
#endif

#ifdef LIMIT_CYCLES
  for (unsigned long int i=0; i<MAX_CYCLES; i++){
#else
  do{
#endif

#ifdef PRAGMA_USE_DATAFLOW
#pragma HLS DATAFLOW
#endif

#pragma HLS PIPELINE II=1

#ifdef PRAGMA_USE_LATENCY
#pragma HLS LATENCY max=1
#endif

#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("==============================================\n");
    printf("cycle %d\n", (int)nbc);
#endif
#endif


//=======================================================
// ALL OF THE DIFFERENT PIPELINE STAGES:
//=======================================================

//=======================================================
    new_cycle(
#ifdef FETCH_EVERY_CYCLE
              f_to_f,
#endif
              f_to_d,
              d_to_f,
              d_to_i,
              i_to_e_1,
              e_1_to_f,
              e_1_to_e_2,
              e_1_to_m,
              e_2_to_w,
              m_to_w,
              mul_latency_out,
              div_latency_out,
#ifdef FPU
			  fpu_latency_out,
#endif
              mul_result_out[0],
              div_result_out[0],
#ifdef FPU
			  fpu_result_out[0],
#ifdef AD
			  ad_mul_result_out,
			  ad_div_result_out,
			  ad_fpu_result_out,
#endif
#endif
              is_mul_in_flight_out,
              is_div_in_flight_out,
#ifdef FPU
			  is_fpu_in_flight_out,
#endif
              f_state_is_full_out,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
              d_state_is_full_out,
              i_state_is_full_out,
              e_1_state_is_full_out,
              e_2_state_is_full_out,
              m_state_is_full_out,
#endif
#ifdef FETCH_EVERY_CYCLE
             &f_from_f,
#endif
             &f_from_d,
             &f_from_e_1,
             &d_from_f,
             &i_from_d,
             &e_1_from_i,
             &e_2_from_e_1,
             &m_from_e_1,
             &w_from_e_2,
             &w_from_m,
             &mul_latency_in,
             &div_latency_in,                  
#ifdef FPU
			 &fpu_latency_in,
#endif
              mul_result_in,
              div_result_in,                  
#ifdef FPU
			  fpu_result_in,
#ifdef AD
			  ad_mul_result_in,
			  ad_div_result_in,
			  ad_fpu_result_in,
#endif
#endif
             &is_mul_in_flight_in,
             &is_div_in_flight_in,
#ifdef FPU
			 &is_fpu_in_flight_in,
#endif
             &f_state_is_full_in
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
            ,&d_state_is_full_in,
             &i_state_is_full_in,
             &e_1_state_is_full_in,
             &e_2_state_is_full_in,
             &m_state_is_full_in
#endif
             );
//=======================================================
    update_mul_latency(e_1_state.decoded_instruction.is_mul,
                       mul_latency_in,
                      &mul_latency_out);                 
//=======================================================
    update_div_latency(e_1_state.decoded_instruction.is_div,
                       div_latency_in,
                      &div_latency_out);             
//=======================================================
#ifdef FPU
    update_fpu_latency(e_1_state.decoded_instruction.is_op_fp || e_1_state.decoded_instruction.is_fused,
                       fpu_latency_in,
                      &fpu_latency_out);
#endif     
//==============================================================================================================
//REVERSE ORDER MAIN LOOP: 
//==============================================================================================================
#ifdef REVERSE_MAIN_LOOP
//==============================================================================================================
//==============================================================================================================

//=======================================================
    write_back(w_from_e_2,
               w_from_m,
#ifdef AD
			   &reg_file,
#else

               reg_file,
#ifdef FPU
			   freg_file,
			   &fcsr,
#endif
#endif
              &w_destination,
#ifdef FPU
			  &wb_type,
#endif
              &w_state);             
//=======================================================
    mem_access(m_from_e_1,
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
              reg_file.reg_file,
#ifdef FPU
			  reg_file.freg_file,
			  &(reg_file.fcsr),
#endif
#else
              reg_file,
#ifdef FPU
  			  freg_file,
     		  &fcsr,
#endif
#endif
#endif
#endif
#ifdef AD

#ifdef LOCAL_MEMORY
			   &data_mem,
#else
			   g_data_mem_0,
			   g_data_mem_1,
			   g_data_mem_2,
			   g_data_mem_3,
			   g_ad_data_mem_0,
			   g_ad_data_mem_1,
			   g_ad_data_mem_2,
			   g_ad_data_mem_3,
#endif

#else
#ifdef LOCAL_MEMORY
               data_mem.data_mem_0,
			   data_mem.data_mem_1,
			   data_mem.data_mem_2,
			   data_mem.data_mem_3,
#else
               data_mem_0,
               data_mem_1,
               data_mem_2,
               data_mem_3,
#endif
#endif
              &m_state,
              &m_to_w,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
              &m_state_is_full_out
#else              
              &m_state_is_full
#endif              
              );
//=======================================================
    execute_2(e_2_from_e_1,
             &e_2_state,
             &e_2_to_w,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
             &e_2_state_is_full_out
#else                
             &e_2_state_is_full
#endif
    );
//=======================================================
    prepare_execute_1(
              e_1_from_i,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
              e_2_state_is_full_in,
              m_state_is_full_in,
#else                
              e_2_state_is_full,
              m_state_is_full,
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
			  reg_file.reg_file,
#ifdef FPU
  			  reg_file.freg_file,
#endif
#else
			  reg_file,
#ifdef FPU
  			  freg_file,
#endif
#endif
#endif
#endif
#ifdef FPU
#ifdef AD
			 &(reg_file.fcsr), //only for emulation
#else
			 &fcsr, //only for emulation
#endif
#endif
             &e_1_state,
             &e_1_to_f,
             &e_1_to_m,
             &e_1_to_e_2,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
             &e_1_state_is_full_out
#else             
             &e_1_state_is_full
#endif  
             );     
//=======================================================
    if (e_1_state.is_full &&
       (e_1_state.decoded_instruction.is_mul && !is_mul_in_flight_in)){
      mul_execute_1(e_1_state,
                    mul_result_out
#ifdef AD
					,ad_mul_result_out
#endif
      	  	  	  	);
      is_mul_in_flight_out = 1;
    }         
//=======================================================
    if (e_1_state.is_full &&
       (e_1_state.decoded_instruction.is_div && !is_div_in_flight_in)){
       div_execute_1(e_1_state,
	                div_result_out
#ifdef AD
					,ad_div_result_out
#endif
					);
      is_div_in_flight_out = 1;
    }
//=======================================================
#ifdef FPU
    if (e_1_state.is_full &&
       ((e_1_state.decoded_instruction.is_op_fp || e_1_state.decoded_instruction.is_fused) && !is_fpu_in_flight_in)){
      fpu_execute_1(e_1_state,
                    fpu_result_out
#ifdef AD
					,ad_fpu_result_out
#endif
					);
      is_fpu_in_flight_out = 1;
    }
#endif        
//=======================================================
    if (e_1_state.is_full                    &&
        e_1_state.decoded_instruction.is_mul &&
       (mul_latency_in == MUL_LATENCY)){
      end_mul_execute_1(mul_result_in[0],
#ifdef AD
    		  	  	    ad_mul_result_in,
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
	  	  	  		   reg_file.reg_file,
#else
                       reg_file,
#endif
#endif
#endif
                       &e_1_state,
                       &e_1_to_e_2,
                       &mul_latency_out,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
                       &e_1_state_is_full_out
#else             
                       &e_1_state_is_full
#endif
                       );
      is_mul_in_flight_out = 0;
    }
//=======================================================
    if (e_1_state.is_full                    &&
        e_1_state.decoded_instruction.is_div &&
       (div_latency_in == DIV_LATENCY)){
      end_div_execute_1(
                        div_result_in[0],
#ifdef AD
    		  	  	    ad_div_result_in,
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
    		  	  	  	reg_file.reg_file,
#else
                        reg_file,
#endif
#endif
#endif
                       &e_1_state,
                       &e_1_to_e_2,
                       &div_latency_out,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
                       &e_1_state_is_full_out
#else             
                       &e_1_state_is_full
#endif
                       );
      is_div_in_flight_out =0;
    }
//=======================================================
#ifdef FPU
    if (e_1_state.is_full                      &&
       (e_1_state.decoded_instruction.is_op_fp || e_1_state.decoded_instruction.is_fused) &&
       (fpu_latency_in == FPU_LATENCY)){
      end_fpu_execute_1(fpu_result_in[0],
#ifdef AD
    		  	  	    ad_fpu_result_in,
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
    		  	  	   reg_file.freg_file,
#else
                       freg_file,
#endif
#endif
#endif
                       &e_1_state,
                       &e_1_to_e_2,
                       &fpu_latency_out,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
                       &e_1_state_is_full_out
#else             
                       &e_1_state_is_full
#endif
                       );
      is_fpu_in_flight_out =0;
    }
#endif   
//=======================================================
    issue(i_from_d,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
          e_1_state_is_full_in,
#else
          e_1_state_is_full,
#endif
         &i_destination,
#ifdef FPU
		 &i_type,
#endif
#ifdef AD
		  &reg_file, //careful - it is a local struct here but a pointer there - pass address
#else
          reg_file,
#ifdef FPU
		  freg_file,
		  &fcsr,
#endif
          is_reg_computed,
#ifdef FPU
		  is_freg_computed,
		  &is_fcsr_computed,
#endif
#endif
         &i_state,
         &i_to_e_1,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
         &i_state_is_full_out
#else
         &i_state_is_full
#endif
         );         
//=======================================================
    decode(d_from_f,
           f_state_is_full_in,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
           i_state_is_full_in,
#else
           i_state_is_full,
#endif
          &d_state,
          &d_to_f,
          &d_to_i,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
          &d_state_is_full_out
#else
          &d_state_is_full
#endif
          );

//=======================================================
    fetch(
#ifdef FETCH_EVERY_CYCLE
    	  f_from_f,
#endif
          f_from_d,
          f_from_e_1,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
          d_state_is_full_in,
#else
          d_state_is_full,
#endif
#ifdef LOCAL_MEMORY
          code_mem,
#else
#ifdef AD
          g_code_mem,
#else
          code_mem,
#endif
#endif
         &f_state,
#ifdef FETCH_EVERY_CYCLE
         &f_to_f,
#endif
         &f_to_d,
         &f_state_is_full_out);          


//==============================================================================================================
//NORMAL ORDER MAIN LOOP: 
//==============================================================================================================
#else //REVERSE_MAIN_LOOP
//==============================================================================================================
//==============================================================================================================

//=======================================================
    fetch(
#ifdef FETCH_EVERY_CYCLE
    	  f_from_f,
#endif
          f_from_d,
          f_from_e_1,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
          d_state_is_full_in,
#else
          d_state_is_full,
#endif
#ifdef LOCAL_MEMORY
          code_mem,
#else
#ifdef AD
          g_code_mem,
#else
          code_mem,
#endif
#endif
         &f_state,
#ifdef FETCH_EVERY_CYCLE
         &f_to_f,
#endif
         &f_to_d,
         &f_state_is_full_out);       
//=======================================================
    decode(d_from_f,
           f_state_is_full_in,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
           i_state_is_full_in,
#else
           i_state_is_full,
#endif
          &d_state,
          &d_to_f,
          &d_to_i,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
          &d_state_is_full_out
#else
          &d_state_is_full
#endif
          );

//=======================================================
    issue(i_from_d,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
          e_1_state_is_full_in,
#else
          e_1_state_is_full,
#endif
         &i_destination,
#ifdef FPU
		 &i_type,
#endif
#ifdef AD
		  &reg_file, //careful - it is a local struct here but a pointer there - pass address
#else
          reg_file,
#ifdef FPU
		  freg_file,
		  &fcsr,
#endif
          is_reg_computed,
#ifdef FPU
		  is_freg_computed,
		  &is_fcsr_computed,
#endif
#endif
         &i_state,
         &i_to_e_1,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
         &i_state_is_full_out
#else
         &i_state_is_full
#endif
         );           
//=======================================================
    prepare_execute_1(
              e_1_from_i,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
              e_2_state_is_full_in,
              m_state_is_full_in,
#else                
              e_2_state_is_full,
              m_state_is_full,
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
			  reg_file.reg_file,
#ifdef FPU
  			  reg_file.freg_file,
#endif
#else
			  reg_file,
#ifdef FPU
  			  freg_file,
#endif
#endif
#endif
#endif
#ifdef FPU
#ifdef AD
			 &(reg_file.fcsr), //only for emulation
#else
			 &fcsr, //only for emulation
#endif
#endif
             &e_1_state,
             &e_1_to_f,
             &e_1_to_m,
             &e_1_to_e_2,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
             &e_1_state_is_full_out
#else             
             &e_1_state_is_full
#endif
             
             );       
//=======================================================
    if (e_1_state.is_full &&
       (e_1_state.decoded_instruction.is_mul && !is_mul_in_flight_in)){
      mul_execute_1(e_1_state,
                    mul_result_out
#ifdef AD
					,ad_mul_result_out
#endif
      	  	  	  	);
      is_mul_in_flight_out = 1;
    }         
//=======================================================
    if (e_1_state.is_full &&
       (e_1_state.decoded_instruction.is_div && !is_div_in_flight_in)){
       div_execute_1(e_1_state,
	                div_result_out
#ifdef AD
					,ad_div_result_out
#endif
					);
      is_div_in_flight_out = 1;
    }
//=======================================================
#ifdef FPU
    if (e_1_state.is_full &&
       ((e_1_state.decoded_instruction.is_op_fp || e_1_state.decoded_instruction.is_fused) && !is_fpu_in_flight_in)){
      fpu_execute_1(e_1_state,
                    fpu_result_out
#ifdef AD
					,ad_fpu_result_out
#endif
					);
      is_fpu_in_flight_out = 1;
    }
#endif        
//=======================================================
    if (e_1_state.is_full                    &&
        e_1_state.decoded_instruction.is_mul &&
       (mul_latency_in == MUL_LATENCY)){
      end_mul_execute_1(mul_result_in[0],
#ifdef AD
    		  	  	    ad_mul_result_in,
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
	  	  	  		   reg_file.reg_file,
#else
                       reg_file,
#endif
#endif
#endif
                       &e_1_state,
                       &e_1_to_e_2,
                       &mul_latency_out,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
                       &e_1_state_is_full_out
#else             
                       &e_1_state_is_full
#endif
                       );
      is_mul_in_flight_out = 0;
    }
//=======================================================
    if (e_1_state.is_full                    &&
        e_1_state.decoded_instruction.is_div &&
       (div_latency_in == DIV_LATENCY)){
      end_div_execute_1(
                        div_result_in[0],
#ifdef AD
    		  	  	    ad_div_result_in,
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
    		  	  	  	reg_file.reg_file,
#else
                        reg_file,
#endif
#endif
#endif
                       &e_1_state,
                       &e_1_to_e_2,
                       &div_latency_out,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
                       &e_1_state_is_full_out
#else             
                       &e_1_state_is_full
#endif
                       );
      is_div_in_flight_out =0;
    }
//=======================================================
#ifdef FPU
    if (e_1_state.is_full                      &&
       (e_1_state.decoded_instruction.is_op_fp || e_1_state.decoded_instruction.is_fused) &&
       (fpu_latency_in == FPU_LATENCY)){
      end_fpu_execute_1(fpu_result_in[0],
#ifdef AD
    		  	  	    ad_fpu_result_in,
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
    		  	  	   reg_file.freg_file,
#else
                       freg_file,
#endif
#endif
#endif
                       &e_1_state,
                       &e_1_to_e_2,
                       &fpu_latency_out,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
                       &e_1_state_is_full_out
#else             
                       &e_1_state_is_full
#endif
                       );
      is_fpu_in_flight_out =0;
    }
#endif   
//=======================================================
    execute_2(e_2_from_e_1,
             &e_2_state,
             &e_2_to_w,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
             &e_2_state_is_full_out
#else                
             &e_2_state_is_full
#endif
            );
//=======================================================
    mem_access(m_from_e_1,
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef AD
              reg_file.reg_file,
#ifdef FPU
			  reg_file.freg_file,
			  &(reg_file.fcsr),
#endif
#else
              reg_file,
#ifdef FPU
  			  freg_file,
     		  &fcsr,
#endif
#endif
#endif
#endif
#ifdef AD

#ifdef LOCAL_MEMORY
			   &data_mem,
#else
			   g_data_mem_0,
			   g_data_mem_1,
			   g_data_mem_2,
			   g_data_mem_3,
			   g_ad_data_mem_0,
			   g_ad_data_mem_1,
			   g_ad_data_mem_2,
			   g_ad_data_mem_3,
#endif

#else
#ifdef LOCAL_MEMORY
               data_mem.data_mem_0,
			   data_mem.data_mem_1,
			   data_mem.data_mem_2,
			   data_mem.data_mem_3,
#else
               data_mem_0,
               data_mem_1,
               data_mem_2,
               data_mem_3,
#endif
#endif
              &m_state,
              &m_to_w,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
              &m_state_is_full_out
#else              
              &m_state_is_full
#endif              
              );
//=======================================================
    write_back(w_from_e_2,
               w_from_m,
#ifdef AD
			   &reg_file,
#else

               reg_file,
#ifdef FPU
			   freg_file,
			   &fcsr,
#endif
#endif
              &w_destination,
#ifdef FPU
			  &wb_type,
#endif
              &w_state);             
         
//==============================================================================================================
//Rest of the main loop in NORMAL ORDER: 
//==============================================================================================================
#endif //REVERSE_MAIN_LOOP
//==============================================================================================================
//==============================================================================================================         
                   
//=======================================================
    lock_unlock_reg(i_destination,
#ifdef FPU
    				i_type,
#endif
                    w_destination,
#ifdef FPU
    				wb_type,
#endif
#ifdef AD
					&reg_file
#else
                    is_reg_computed
#endif
					);
//=======================================================
#ifdef FPU              
    lock_unlock_freg(i_destination,
    				i_type,
                    w_destination,
    				wb_type,
#ifdef AD
					&reg_file
#else
                    is_freg_computed,
					&is_fcsr_computed
#endif
					);
#endif              
//=======================================================
    statistic_update(e_1_from_i, &nbi, &nbc);
//=======================================================
    running_cond_update(w_from_e_2, &is_running);
//=======================================================


#ifndef __SYNTHESIS__
#ifdef DEBUG_EXIT
       printf("w_from_e_2.is_exit = %d\n", w_from_e_2.is_exit);
#endif
#endif
#ifdef LIMIT_CYCLES
    if (!is_running) break;
#endif
  }
#ifndef LIMIT_CYCLES
  while (is_running);
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  printf("==============================================\n");
#endif
#endif
  *nb_cycle       = nbc;
  *nb_instruction = nbi;

#if defined(LOCAL_MEMORY) && !defined(AD)
  //COPY local back to global memory
    copy_local_mem_to_global(
    					g_data_mem_0,
    					g_data_mem_1,
    					g_data_mem_2,
    					g_data_mem_3,
    					&data_mem);
#endif


#ifdef AD

#ifdef LOCAL_MEMORY
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
#endif

#ifndef __SYNTHESIS__
#ifdef DEBUG_REG_FILE
#ifdef AD
  print_reg(&reg_file);
#else
  print_reg(reg_file.reg_file);
#endif
#endif
#ifdef DEBUG_FREG_FILE
#ifdef AD
  print_freg(&reg_file);
#else
  print_freg(reg_file.freg_file);
#endif
#endif
#ifdef DEBUG_FCSR_REG
  print_fcsr(reg_file.fcsr);
#endif
#endif
#else
#ifndef __SYNTHESIS__
#ifdef DEBUG_REG_FILE
  print_reg(reg_file);
#endif
#ifdef FPU
#ifdef DEBUG_FREG_FILE
  print_freg(freg_file);
#endif
#ifdef DEBUG_FCSR_REG
  print_fcsr(fcsr);
#endif
#endif
#endif
#endif
}
#ifdef __cplusplus
}
#endif


#ifdef AD
//copies all entries to all respective entries, a=desination, b=source
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
//copies all entries to all respective entries, a=desination, b=source
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

//copies all entries to all respective entries, a=desination, b=source
void copy_array_float_int_t(
  float_int_t a[],
  float_int_t b[],
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
//copies a single entry to all entries, a=destination, b=source
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
//copies a single entry to all entries, a=destination, b=source
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
//copies a single entry to all entries, a=destination, b=source
void copy_value_float(
  float a[],
  float *b,
  max_der_t length
  ){
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
//copies a single entry to all entries, a=destination, b=source
void copy_value_float_int_t(
  float_int_t a[],
  float_int_t b[],
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

//copies all entries to all respective entries, a=desination, b=source
void copy_int_array_to_float_int_t(
  float_int_t a[],
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
		a[i].i = *(b+i);
	}
	return;
}
//copies all entries to all respective entries, a=desination, b=source
void copy_float_array_to_float_int_t(
  float_int_t a[],
  float b[],
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
		a[i].f = *(b+i);
	}
	return;
}

//copies all entries to all respective entries, a=desination, b=source
void copy_float_int_t_array_to_float(
  float a[],
  float_int_t b[],
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
		*(a+i) = b[i].f;
	}
	return;
}

//copies all entries to all respective entries, a=desination, b=source
void copy_float_int_t_array_to_int(
  int a[],
  float_int_t b[],
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
		*(a+i) = b[i].i;
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
#endif
