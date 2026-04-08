#include "debug_adrv32imf_mp_ip.h"
#include "adrv32imf_mp_ip.h"
#include "ap_int.h"
#include "mem.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include "stdio.h"
#include "emulate.h"
#endif
#endif

void init_m_state(m_state_t *m_state){
  m_state->is_full = 0;
}
static void get_input(
  from_e_1_to_m_t m_from_e_1,
  m_state_t      *m_state){
#pragma HLS INLINE
  m_state->value_0             = m_from_e_1.value_0;
  m_state->value_1             = m_from_e_1.value_1;
  m_state->value_2             = m_from_e_1.value_2;
  m_state->value_3             = m_from_e_1.value_3;
#ifdef AD
  copy_array_8bit(m_state->ad_value_0, m_from_e_1.ad_value_0, AD_DERIV_CNT);
  copy_array_8bit(m_state->ad_value_1, m_from_e_1.ad_value_1, AD_DERIV_CNT);
  copy_array_8bit(m_state->ad_value_2, m_from_e_1.ad_value_2, AD_DERIV_CNT);
  copy_array_8bit(m_state->ad_value_3, m_from_e_1.ad_value_3, AD_DERIV_CNT);
//TODO: understand why the following creates extreme neg. slack due to dependencies?
#pragma HLS ARRAY_PARTITION variable=m_state->ad_value_0 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=m_state->ad_value_1 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=m_state->ad_value_2 dim=0 complete
#pragma HLS ARRAY_PARTITION variable=m_state->ad_value_3 dim=0 complete
#pragma HLS ARRAY_RESHAPE variable=m_state->ad_value_0 dim=0 complete
#pragma HLS ARRAY_RESHAPE variable=m_state->ad_value_1 dim=0 complete
#pragma HLS ARRAY_RESHAPE variable=m_state->ad_value_2 dim=0 complete
#pragma HLS ARRAY_RESHAPE variable=m_state->ad_value_3 dim=0 complete
#pragma HLS bind_storage variable=m_state->ad_value_0 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=m_state->ad_value_1 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=m_state->ad_value_2 type=RAM_T2P impl=BRAM latency=1
#pragma HLS bind_storage variable=m_state->ad_value_3 type=RAM_T2P impl=BRAM latency=1

  m_state->ad_channel		   = m_from_e_1.ad_channel;
  m_state->is_ad_store         = m_from_e_1.is_ad_store;
  m_state->is_ad_op            = m_from_e_1.is_ad_op;
#endif
  m_state->address             = m_from_e_1.address;
  m_state->rd                  = m_from_e_1.rd;
#ifdef AD
  m_state->rs2                 = m_from_e_1.rs2;
  m_state->rv1                 = m_from_e_1.rv1;
#endif
  m_state->has_no_dest         = m_from_e_1.has_no_dest;
  m_state->is_load             = m_from_e_1.is_load; //this includes FP LOADS (execute1)
  m_state->is_store            = m_from_e_1.is_store;//this includes FP STORES (execute1)
  m_state->func3               = m_from_e_1.func3;
#ifdef FPU
  m_state->wb_info_fpu		   = m_from_e_1.wb_info_fpu;
#endif
#ifndef __SYNTHESIS__
  m_state->pc                  = m_from_e_1.pc;
  m_state->instruction         = m_from_e_1.instruction;
  m_state->decoded_instruction = m_from_e_1.decoded_instruction;
#endif
}
static void stage_job(
  ap_uint<2>     a01,
  bit_t          a1,
  data_address_t a2,
  func3_t        msize,
  m_state_t     *m_state,
#ifdef AD
#ifdef LOCAL_MEMORY
  data_mem_t	 *data_mem_s
#else
  char* data_mem_0,
  char* data_mem_1,
  char* data_mem_2,
  char* data_mem_3,
  char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT]
#endif

#else
  char          *data_mem_0,
  char          *data_mem_1,
  char          *data_mem_2,
  char          *data_mem_3
#endif
  ){
#pragma HLS INLINE
  if (m_state->is_load){
#ifdef AD
	m_state->is_ad_op = true; //load also loads ad_regs, have to be written back in wb-stage
#endif
    mem_load(
#ifdef AD
#ifdef LOCAL_MEMORY
    	data_mem_s,
#else
	    data_mem_0,
	    data_mem_1,
	    data_mem_2,
	    data_mem_3,
	    ad_data_mem_0,
	    ad_data_mem_1,
	    ad_data_mem_2,
	    ad_data_mem_3,
#endif
#else
        data_mem_0,
        data_mem_1,
        data_mem_2,
        data_mem_3,
#endif
        a2,
       &(m_state->value_0),
       &(m_state->value_1),
       &(m_state->value_2),
       &(m_state->value_3)
#ifdef AD
	   ,m_state->ad_value_0,
	   m_state->ad_value_1,
	   m_state->ad_value_2,
	   m_state->ad_value_3,
	   (ap_uint<2>)msize
#endif
    );
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
	  printf("AD_DEBUG: calling mem_load ");
	  printf("MEM VALUE is: %d\n", (unsigned int)(m_state->value_0 + (m_state->value_1 << 8) + (m_state->value_2 << 16) + (m_state->value_3 << 24)));
#endif
#endif
  }
#ifdef AD
  //it is very! important to get this before! the else if (m_state->is_store)
  else if (m_state->is_ad_store){
#ifndef __SYNTHESIS__
#ifdef DEBUG_AD
	printf("AD_DEBUG: calling ad_mem_store \n");
	printf("passing rv1: %d\n", m_state->rv1);
#endif
#endif
    ad_mem_store(
#ifdef LOCAL_MEMORY
      data_mem_s,
#else
	  data_mem_0,
	  data_mem_1,
	  data_mem_2,
	  data_mem_3,
	  ad_data_mem_0,
	  ad_data_mem_1,
	  ad_data_mem_2,
	  ad_data_mem_3,
#endif
      a01,
      a1,
      a2,
	  m_state->rv1,
      &(m_state->value_0),
      &(m_state->value_1),
      &(m_state->value_2),
      &(m_state->value_3),
	  m_state->ad_value_0,
	  m_state->ad_value_1,
	  m_state->ad_value_2,
	  m_state->ad_value_3,
	  m_state->ad_channel,
	  &(m_state->has_no_dest),
	  &(m_state->is_ad_op),
	  &(m_state->rd),
	  &(m_state->rs2),
      (ap_uint<3>)msize);
#ifndef __SYNTHESIS__
#ifdef DEBUG_AD
	printf("MEM VALUE is: %d\n", (unsigned int)(m_state->value_0 + (m_state->value_1 << 8) + (m_state->value_2 << 16) + (m_state->value_3 << 24)));
#endif
#endif
  }
  else if (m_state->is_store){
	//m_state->is_ad_op = false; //TODO: inconsistent, technically it IS an AD_OP, but without WriteBack to Reg
#else
  else{ //if (m_state->is_store)
#endif
    mem_store(
#ifdef AD
#ifdef LOCAL_MEMORY
      data_mem_s,
#else
	  data_mem_0,
	  data_mem_1,
	  data_mem_2,
	  data_mem_3,
	  ad_data_mem_0,
	  ad_data_mem_1,
	  ad_data_mem_2,
	  ad_data_mem_3,
#endif

#else
      data_mem_0,
      data_mem_1,
      data_mem_2,
      data_mem_3,
#endif
      a01,
      a1,
      a2,
      m_state->value_0,
      m_state->value_1,
      m_state->value_2,
      m_state->value_3,
#ifdef AD
	  m_state->ad_value_0,
	  m_state->ad_value_1,
	  m_state->ad_value_2,
	  m_state->ad_value_3,
#endif
      (ap_uint<2>)msize);
#ifndef __SYNTHESIS__
#ifdef DEBUG_AD
	  printf("AD_DEBUG: calling mem_store ");
	  printf("MEM VALUE is: %d\n", (unsigned int)(m_state->value_0 + (m_state->value_1 << 8) + (m_state->value_2 << 16) + (m_state->value_3 << 24)));
#endif
#endif
  }
}
static void set_output(
  ap_uint<2>     a01,
  bit_t          a1,
  func3_t        msize,
  m_state_t      m_state,
  from_m_to_w_t *m_to_w){
#pragma HLS INLINE
  m_to_w->value_0             = m_state.value_0;
  m_to_w->value_1             = m_state.value_1;
  m_to_w->value_2             = m_state.value_2;
  m_to_w->value_3             = m_state.value_3;
#ifdef AD
  copy_array_8bit(m_to_w->ad_value_0, m_state.ad_value_0, AD_DERIV_CNT);
  copy_array_8bit(m_to_w->ad_value_1, m_state.ad_value_1, AD_DERIV_CNT);
  copy_array_8bit(m_to_w->ad_value_2, m_state.ad_value_2, AD_DERIV_CNT);
  copy_array_8bit(m_to_w->ad_value_3, m_state.ad_value_3, AD_DERIV_CNT);
  m_to_w->is_ad_op         	  = m_state.is_ad_op;
  m_to_w->is_ad_store      	  = m_state.is_ad_store;
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
	for(int i = 0; i < AD_DERIV_CNT; i++){
		printf("AD_DEBUG:SENDING FROM MEM TO WB due to IS_AD_STORE ADSW OR ADLR\n");
		printf("AD_DEBUG: char0 = %d\n", m_state.ad_value_0[i]);
		printf("AD_DEBUG: char1 = %d\n", m_state.ad_value_1[i]);
		printf("AD_DEBUG: char2 = %d\n", m_state.ad_value_2[i]);
		printf("AD_DEBUG: char3 = %d\n", m_state.ad_value_3[i]);
	}
#endif
#endif
#endif
  m_to_w->a01                 = a01;
  m_to_w->a1                  = a1;
  m_to_w->msize               = msize;
  m_to_w->rd                  = m_state.rd;
  m_to_w->has_no_dest         = m_state.has_no_dest;
#ifdef FPU
  m_to_w->wb_info_fpu		  = m_state.wb_info_fpu;
#endif
#ifndef __SYNTHESIS__
  m_to_w->pc                  = m_state.pc;
  m_to_w->instruction         = m_state.instruction;
  m_to_w->decoded_instruction = m_state.decoded_instruction;
#endif
}
void mem_access(
  from_e_1_to_m_t m_from_e_1,
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  int            *reg_file,
#ifdef FPU
  float			 *freg_file,
  int			 *fcsr,
#endif
#endif
#endif
#ifdef AD
#ifdef LOCAL_MEMORY
  data_mem_t	 *data_mem_s,
#else
  char* data_mem_0,
  char* data_mem_1,
  char* data_mem_2,
  char* data_mem_3,
  char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
#endif
#else
  char           *data_mem_0,
  char           *data_mem_1,
  char           *data_mem_2,
  char           *data_mem_3,
#endif
  m_state_t      *m_state,
  from_m_to_w_t  *m_to_w,
  bit_t          *m_state_is_full){
#pragma HLS INLINE off
  bit_t          save_input;
  bit_t          a1;
  ap_uint<2>     a01;
  data_address_t a2;
  func3_t        msize;
  bit_t          valid_or_full;
  valid_or_full = m_from_e_1.is_valid ||  m_state->is_full;
  save_input    = m_from_e_1.is_valid && !m_state->is_full;
  m_state->is_full = valid_or_full;
  if (save_input)
    get_input(m_from_e_1, m_state);
  m_to_w->is_valid =
#if defined(__SYNTHESIS__) || defined(DEBUG_PIPELINE)
   !m_state->is_store &&
#ifdef AD
   //THIS was commented out of my AD processor - is this still a considerable issue?
   //TODO=: CHECK AND VERIFY -> if is_ad_store AND fnc3 == 0b111, is still valid, has to be written back
   //!m_state->is_ad_store &&
#endif
#endif
    valid_or_full;
  if (valid_or_full){
    a01   =  m_state->address;
    a1    = (m_state->address >> 1);
    a2    = (m_state->address >> 2);
    msize =  m_state->func3;
    stage_job(a01,
              a1,
              a2,
              msize,
              m_state,
#ifdef AD
#ifdef LOCAL_MEMORY
			  data_mem_s
#else
			  data_mem_0,
			  data_mem_1,
			  data_mem_2,
			  data_mem_3,
			  ad_data_mem_0,
			  ad_data_mem_1,
			  ad_data_mem_2,
			  ad_data_mem_3
#endif
#else
              data_mem_0,
              data_mem_1,
              data_mem_2,
              data_mem_3
#endif
    );
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("mem      ");
    printf("%04d\n", (int)m_state->pc);
    if (m_state->is_store){
#ifdef FPU
	  emulate(reg_file,
			  freg_file,
			  *fcsr,
			  m_state->decoded_instruction,
			  0);
#else
      emulate(reg_file,
              m_state->decoded_instruction,
              0);
#endif
    }
#endif
#endif
    set_output(a01, a1, msize, *m_state, m_to_w);
  }
  *m_state_is_full = 0;
  m_state->is_full = 0;
}
