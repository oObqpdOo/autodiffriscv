#include "debug_multicycle_pipeline_ip.h"
#include "ad_multicycle_pipeline_ip.h"
#include "compute.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include "stdio.h"
#endif
#endif

void init_e_2_state(e_2_state_t *e_2_state){
  e_2_state->is_full = 0;
}
static void get_input(
  from_e_1_to_e_2_t e_2_from_e_1,
  e_2_state_t      *e_2_state){
#pragma HLS INLINE
  e_2_state->result1             = e_2_from_e_1.result1;
  e_2_state->result2             = e_2_from_e_1.result2;
  //TODO: copy or is pointer passing sufficient??? UNUSED IN EXECUTE 2 SO FAR!
  copy_array_32bit(e_2_state->ad_result1, e_2_from_e_1.ad_result1, AD_DERIV_CNT);
  copy_array_32bit(e_2_state->ad_result2, e_2_from_e_1.ad_result2, AD_DERIV_CNT);
  e_2_state->rd                  = e_2_from_e_1.rd;
  e_2_state->has_no_dest         = e_2_from_e_1.has_no_dest;
  e_2_state->is_r_type           = e_2_from_e_1.is_r_type;
  e_2_state->is_op_imm           = e_2_from_e_1.is_op_imm;
  e_2_state->is_ad_op            = e_2_from_e_1.is_ad_op;
  e_2_state->is_ret              = e_2_from_e_1.is_ret;
  e_2_state->is_exit              = e_2_from_e_1.is_exit;
#ifndef __SYNTHESIS__
  e_2_state->pc                  = e_2_from_e_1.pc;
  e_2_state->instruction         = e_2_from_e_1.instruction;
  e_2_state->decoded_instruction = e_2_from_e_1.decoded_instruction;
  e_2_state->target_pc           = e_2_from_e_1.target_pc;
#endif
}
static void stage_job(
  e_2_state_t e_2_state,
  int        *result,
  int        ad_result[AD_DERIV_CNT]){
#pragma HLS INLINE
  *result =
   (e_2_state.is_r_type ||
    e_2_state.is_op_imm)?
    e_2_state.result1   :
    e_2_state.result2;

   (e_2_state.is_r_type ||
    e_2_state.is_op_imm)?
    copy_array_32bit(ad_result, e_2_state.ad_result1, AD_DERIV_CNT)   :
	copy_array_32bit(ad_result, e_2_state.ad_result2, AD_DERIV_CNT) ;

}
static void set_output(
  int              result,
  int        	   ad_result[AD_DERIV_CNT],
  e_2_state_t      e_2_state,
  from_e_2_to_w_t *e_2_to_w){
#pragma HLS INLINE
  e_2_to_w->result               =
    (e_2_state.is_ret) ?
     e_2_state.result2 :
     result;
  (e_2_state.is_ret) ?
    copy_array_32bit(e_2_to_w->ad_result, e_2_state.ad_result2, AD_DERIV_CNT)   :
	copy_array_32bit(e_2_to_w->ad_result, ad_result, AD_DERIV_CNT) ;
  e_2_to_w->rd                   = e_2_state.rd;
  e_2_to_w->has_no_dest          = e_2_state.has_no_dest;
  e_2_to_w->is_ad_op          	 = e_2_state.is_ad_op;
  e_2_to_w->is_ret               = e_2_state.is_ret;
  e_2_to_w->is_exit               = e_2_state.is_exit;
#ifndef __SYNTHESIS__
  e_2_to_w->pc                   = e_2_state.pc;
  e_2_to_w->instruction          = e_2_state.instruction;
  e_2_to_w->decoded_instruction  = e_2_state.decoded_instruction;
  e_2_to_w->target_pc            = e_2_state.target_pc;
#endif
}
void execute_2(
  bit_t             m_from_e_1_is_valid,
  from_e_1_to_e_2_t e_2_from_e_1,
  e_2_state_t      *e_2_state,
  from_e_2_to_w_t  *e_2_to_w,
  bit_t            *e_2_state_is_full){
#pragma HLS INLINE off
  bit_t save_input, e_2_to_m_is_valid;
  int   result;
  int 	ad_result[AD_DERIV_CNT];
//TODO: understand why the following creates extreme neg. slack due to dependencies?
#pragma HLS ARRAY_PARTITION variable=ad_result dim=1 complete
  save_input = e_2_from_e_1.is_valid && (!e_2_state->is_full);
  if (save_input)
    get_input(e_2_from_e_1, e_2_state);
  e_2_state->is_full =
     m_from_e_1_is_valid   &&
    (e_2_from_e_1.is_valid || e_2_state->is_full);
  *e_2_state_is_full = e_2_state->is_full;
  e_2_to_w->is_valid    =
   (e_2_state->is_full || e_2_from_e_1.is_valid);
  if (e_2_to_w->is_valid){
    stage_job(*e_2_state, &result, ad_result);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("exe_ed_2 ");
    printf("%04d\n", (int)e_2_state->pc);
#endif
#endif
    set_output(result, ad_result, *e_2_state, e_2_to_w);
  }
}
