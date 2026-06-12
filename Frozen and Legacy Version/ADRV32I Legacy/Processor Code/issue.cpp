#include "debug_multicycle_pipeline_ip.h"
#include "ad_multicycle_pipeline_ip.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include "stdio.h"
#endif
#endif

void init_i_state(i_state_t *i_state){
  i_state->is_full     = 0;
}
static void get_input(
  from_d_to_i_t i_from_d,
  i_state_t        *i_state){
#pragma HLS INLINE
  i_state->decoded_instruction = i_from_d.decoded_instruction;
  i_state->pc                  = i_from_d.pc;
#ifndef __SYNTHESIS__
  i_state->instruction         = i_from_d.instruction;
  i_state->target_pc           = i_from_d.target_pc;
#endif
}
static void stage_job(
  i_state_t i_state,
  reg_t     *reg_file,
  int       *rv1,
  int       *rv2,
  int		ad_rv1[AD_DERIV_CNT],
  int		ad_rv2[AD_DERIV_CNT]){
#pragma HLS INLINE
  *rv1 = reg_file->gp_reg_file[i_state.decoded_instruction.rs1]; //pointer to int
  *rv2 = reg_file->gp_reg_file[i_state.decoded_instruction.rs2];
  copy_array_32bit(ad_rv1, reg_file->ad_reg_file[i_state.decoded_instruction.rs1], AD_DERIV_CNT); //pointer to array
  copy_array_32bit(ad_rv2, reg_file->ad_reg_file[i_state.decoded_instruction.rs2], AD_DERIV_CNT);
}
static void set_output(
  int              rv1,
  int              rv2,
  int			   ad_rv1[AD_DERIV_CNT],
  int			   ad_rv2[AD_DERIV_CNT],
  i_state_t        i_state,
  from_i_to_e_1_t *i_to_e_1){
#pragma HLS INLINE
//TODO: understand why the following creates extreme neg. slack due to dependencies?
#pragma HLS ARRAY_PARTITION variable=ad_rv1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=ad_rv2 dim=1 complete
  i_to_e_1->rv1                 = rv1;
  i_to_e_1->rv2                 = rv2;
  //TODO: copy or is pointer passing sufficient??? It's getting really late in the office now ;)
  copy_array_32bit(i_to_e_1->ad_rv1, ad_rv1, AD_DERIV_CNT);
  copy_array_32bit(i_to_e_1->ad_rv2, ad_rv2, AD_DERIV_CNT);
  i_to_e_1->decoded_instruction = i_state.decoded_instruction;
  i_to_e_1->pc                  = i_state.pc;
#ifndef __SYNTHESIS__
  i_to_e_1->instruction         = i_state.instruction;
  i_to_e_1->target_pc           = i_state.target_pc;
#endif
}
void issue(
  from_d_to_i_t    i_from_d,
  bit_t            e_1_state_is_full,
  reg_num_t       *i_destination,
  reg_t           *reg_file,
  i_state_t       *i_state,
  from_i_to_e_1_t *i_to_e_1,
  bit_t           *i_state_is_full){
#pragma HLS INLINE off
  bit_t save_input, wait_for_source, is_locked_1, is_locked_2, is_locked_d;
  //TODO: LOCAL copies of register values for this particular stage! - can't just pass pointer here!!!
  int   rv1, rv2;
  int   ad_rv1[AD_DERIV_CNT];
  int	ad_rv2[AD_DERIV_CNT];
//TODO: understand why the following creates extreme neg. slack due to dependencies?
//#pragma HLS ARRAY_PARTITION variable=ad_rv1 dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=ad_rv2 dim=1 complete
  is_locked_1 =
    i_from_d.decoded_instruction.is_rs1_reg &&
    reg_file->is_reg_computed[i_from_d.decoded_instruction.rs1];
  is_locked_2 =
    i_from_d.decoded_instruction.is_rs2_reg &&
	reg_file->is_reg_computed[i_from_d.decoded_instruction.rs2];
  is_locked_d =
  (!i_from_d.decoded_instruction.has_no_dest) &&
  reg_file->is_reg_computed[i_from_d.decoded_instruction.rd];
  wait_for_source = is_locked_1 || is_locked_2 || is_locked_d;
  i_to_e_1->is_valid =
    !e_1_state_is_full  &&
    (i_from_d.is_valid ||
     i_state->is_full)  &&
    !wait_for_source;
  if (i_to_e_1->is_valid)
    *i_destination = i_from_d.decoded_instruction.rd;
  else
    *i_destination = 0;
  save_input = i_from_d.is_valid && (!i_state->is_full);
  if (save_input)
    get_input(i_from_d, i_state);
  i_state->is_full =
    (e_1_state_is_full ||
     wait_for_source)  &&
    (i_from_d.is_valid ||
     i_state->is_full);
  *i_state_is_full = i_state->is_full;
  if (i_to_e_1->is_valid){
    stage_job(*i_state, reg_file, &rv1, &rv2, ad_rv1, ad_rv2);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    if (!wait_for_source){
      printf("issued   ");
      printf("%04d\n", (int)i_state->pc);
    }
#endif
#endif
    set_output(rv1,
               rv2,
			   ad_rv1,
			   ad_rv2,
              *i_state,
               i_to_e_1);
  }
}
