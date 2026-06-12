#include "debug_multicycle_pipeline_ip.h"
#include "ad_multicycle_pipeline_ip.h"
#include "ap_int.h"
#include "mem.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include "stdio.h"
#endif
#endif

static void get_input(
  from_e_1_to_m_t m_from_e_1,
  m_state_t      *m_state){
#pragma HLS INLINE
  m_state->value_0             = m_from_e_1.value_0;
  m_state->value_1             = m_from_e_1.value_1;
  m_state->value_2             = m_from_e_1.value_2;
  m_state->value_3             = m_from_e_1.value_3;
  copy_array_8bit(m_state->ad_value_0, m_from_e_1.ad_value_0, AD_DERIV_CNT);
  copy_array_8bit(m_state->ad_value_1, m_from_e_1.ad_value_1, AD_DERIV_CNT);
  copy_array_8bit(m_state->ad_value_2, m_from_e_1.ad_value_2, AD_DERIV_CNT);
  copy_array_8bit(m_state->ad_value_3, m_from_e_1.ad_value_3, AD_DERIV_CNT);
//TODO: understand why the following creates extreme neg. slack due to dependencies?
//#pragma HLS ARRAY_PARTITION variable=m_state->ad_value_0 dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=m_state->ad_value_1 dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=m_state->ad_value_2 dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=m_state->ad_value_3 dim=1 complete
  m_state->address             = m_from_e_1.address;
  m_state->rd                  = m_from_e_1.rd;
  m_state->rs2                 = m_from_e_1.rs2;
  m_state->rv1                 = m_from_e_1.rv1;
  m_state->has_no_dest         = m_from_e_1.has_no_dest;
  m_state->is_load             = m_from_e_1.is_load;
  m_state->is_store            = m_from_e_1.is_store;
  //AD_ADATION:
  m_state->ad_channel		   = m_from_e_1.ad_channel;
  m_state->is_ad_store         = m_from_e_1.is_ad_store;
  m_state->is_ad_op            = m_from_e_1.is_ad_op;
  //END_AD_ADATION:
  m_state->func3               = m_from_e_1.func3;
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
  data_mem_t    *data_mem){
#pragma HLS INLINE
  if (m_state->is_load){
	m_state->is_ad_op = true; //load also loads ad_regs, have to be written back in wb-stage
    mem_load(
		data_mem,
		a2,
       &(m_state->value_0),
       &(m_state->value_1),
       &(m_state->value_2),
       &(m_state->value_3),
	   m_state->ad_value_0,
	   m_state->ad_value_1,
	   m_state->ad_value_2,
	   m_state->ad_value_3,
	  (ap_uint<2>)msize);
  }
  else if (m_state->is_store)
	//m_state->is_ad_op = false; //TODO: inconsistent, technically it IS an AD_OP, but without WriteBack to Reg
    mem_store(
      data_mem,
      a01,
      a1,
      a2,
      m_state->value_0,
      m_state->value_1,
      m_state->value_2,
      m_state->value_3,
	  m_state->ad_value_0,
	  m_state->ad_value_1,
	  m_state->ad_value_2,
	  m_state->ad_value_3,
      (ap_uint<2>)msize);
  else if (m_state->is_ad_store)
    ad_mem_store(
      data_mem,
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
  copy_array_8bit(m_to_w->ad_value_0, m_state.ad_value_0, AD_DERIV_CNT);
  copy_array_8bit(m_to_w->ad_value_1, m_state.ad_value_1, AD_DERIV_CNT);
  copy_array_8bit(m_to_w->ad_value_2, m_state.ad_value_2, AD_DERIV_CNT);
  copy_array_8bit(m_to_w->ad_value_3, m_state.ad_value_3, AD_DERIV_CNT);
  m_to_w->a01                 = a01;
  m_to_w->a1                  = a1;
  m_to_w->msize               = msize;
  m_to_w->rd                  = m_state.rd;
  m_to_w->has_no_dest         = m_state.has_no_dest;
  m_to_w->is_ad_op         	  = m_state.is_ad_op;
  m_to_w->is_ad_store      	  = m_state.is_ad_store;
  #ifndef __SYNTHESIS__
  m_to_w->pc                  = m_state.pc;
  m_to_w->instruction         = m_state.instruction;
  m_to_w->decoded_instruction = m_state.decoded_instruction;
#endif
}
void mem_access(
  from_e_1_to_m_t m_from_e_1,
  data_mem_t     *data_mem,
  m_state_t      *m_state,
  from_m_to_w_t  *m_to_w){
#pragma HLS INLINE off
  bit_t          save_input;
  bit_t          a1;
  ap_uint<2>     a01;
  data_address_t a2;
  func3_t        msize;
  save_input = m_from_e_1.is_valid;
  if (save_input)
    get_input(m_from_e_1, m_state);
  if (m_from_e_1.is_valid){
    a01   =  m_state->address;
    a1    = (m_state->address >> 1);
    //TODO: old was => a2    = (m_state->address >> 2)&(DATA_MEM_SIZE/4 - 1);
    a2    = (m_state->address >> 2)&(DATA_MEM_SIZE/sizeof(int) - 1);
    msize =  m_state->func3;
    stage_job(a01,
              a1,
              a2,
              msize,
              m_state,
              data_mem);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("mem      ");
    printf("%04d\n", (int)m_state->pc);
#endif
#endif
    set_output(a01, a1, msize, *m_state, m_to_w);
  }
  m_to_w->is_valid =
#if defined(__SYNTHESIS__) || defined(DEBUG_PIPELINE)
   !m_state->is_store &&
   //TOD=: CHECK AND VERIFY -> if is_ad_store AND fnc3 == 0b111, is still valid, has to be written back
   //!m_state->is_ad_store &&
#endif
    m_from_e_1.is_valid;
}
