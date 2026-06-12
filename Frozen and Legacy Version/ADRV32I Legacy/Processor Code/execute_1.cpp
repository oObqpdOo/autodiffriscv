#include "debug_multicycle_pipeline_ip.h"
#include "ad_multicycle_pipeline_ip.h"
#include "compute.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include "stdio.h"
#include "emulate.h"
#endif
#endif

void init_e_1_state(e_1_state_t *e_1_state){
  e_1_state->is_full = 0;
}
static void get_input(
  from_i_to_e_1_t e_1_from_i,
  e_1_state_t       *e_1_state){
#pragma HLS INLINE
  e_1_state->rv1                 = e_1_from_i.rv1;
  e_1_state->rv2                 = e_1_from_i.rv2;
  //TODO: copy or is pointer passing sufficient??? double check tomorrow ;)
  copy_array_32bit(e_1_state->ad_rv1, e_1_from_i.ad_rv1, AD_DERIV_CNT);
  copy_array_32bit(e_1_state->ad_rv2, e_1_from_i.ad_rv2, AD_DERIV_CNT);
  e_1_state->pc                  = e_1_from_i.pc;
  e_1_state->decoded_instruction = e_1_from_i.decoded_instruction;
#ifndef __SYNTHESIS__
  e_1_state->target_pc           = e_1_from_i.target_pc;
  e_1_state->instruction         = e_1_from_i.instruction;
#endif
}
static void compute(
  e_1_state_t    *e_1_state,
  bit_t          *bcond,
  int            *result1,
  int            *result2,
  int			 ad_result1[AD_DERIV_CNT],
  //int			 ad_result2[AD_DERIV_CNT],
  code_address_t *target_pc){
#pragma HLS INLINE
  *bcond = compute_branch_result(
    e_1_state->rv1,
    e_1_state->rv2,
    e_1_state->decoded_instruction.func3);
  /* *result1 = compute_op_result(
    e_1_state->rv1,
    e_1_state->rv2,
    e_1_state->decoded_instruction); */
  //pass pointers for results, computation has to change values directly
  //TODO: but d_i is NOT a pointer, but copy;
  //so e_1_state->decoded_instruction.is_ad_op is not modified IN function
  //TODO: clean this up?
  e_1_state->decoded_instruction.is_ad_op = compute_ad_op_result(
    e_1_state->rv1,
    e_1_state->rv2,
	e_1_state->ad_rv1,
	e_1_state->ad_rv2,
	result1,
	ad_result1,
    e_1_state->decoded_instruction);
  *result2 = compute_result(
    e_1_state->rv1,
	//e_1_state->ad_rv1,
    e_1_state->pc,
	//ad_result2,
    e_1_state->decoded_instruction);
  *target_pc = compute_next_pc(
    e_1_state->pc,
    e_1_state->decoded_instruction,
    e_1_state->rv1);
}
static void stage_job(
  e_1_state_t     e_1_state,
  bit_t           bcond,
  code_address_t  tpc,
  code_address_t *target_pc){
#pragma HLS INLINE
  *target_pc =
   (bcond ||
    e_1_state.decoded_instruction.is_jalr) ?
    tpc                                    :
   (code_address_t)(e_1_state.pc + 4);
}

static void shift_results(
	int in_value[],
	char out_s_value0[],
	char out_s_value1[],
	char out_s_value2[],
	char out_s_value3[],
	unsigned long int length){
		unsigned long int i;
		//TODO: PRAGMAS CHECK
#pragma HLS PIPELINE II=1
#pragma HLS LATENCY max=1
		for(i = 0; i < length; i++){
#pragma HLS unroll
			out_s_value0[i] = in_value[i];
			out_s_value1[i] = in_value[i] >> 8;
			out_s_value2[i] = in_value[i] >> 16;
			out_s_value3[i] = in_value[i] >> 24;
	}
}

static void set_output(
  int                result1,
  int                result2,
  int			 	 ad_result1[AD_DERIV_CNT],
  int			 	 ad_result2[AD_DERIV_CNT],
  code_address_t     target_pc,
  e_1_state_t        e_1_state,
  from_e_1_to_f_t   *e_1_to_f,
  from_e_1_to_m_t   *e_1_to_m,
  from_e_1_to_e_2_t *e_1_to_e_2){
#pragma HLS INLINE
  e_1_to_f->target_pc         = target_pc;
  e_1_to_m->address           = result2;
  e_1_to_m->ad_channel		  = e_1_state.decoded_instruction.imm;
  e_1_to_m->value_0           = e_1_state.rv2;
  e_1_to_m->value_1           = e_1_state.rv2 >> 8;
  e_1_to_m->value_2           = e_1_state.rv2 >> 16;
  e_1_to_m->value_3           = e_1_state.rv2 >> 24;
  //TODO: create shifts
  shift_results(
		  e_1_state.ad_rv2,
		  e_1_to_m->ad_value_0,
		  e_1_to_m->ad_value_1,
		  e_1_to_m->ad_value_2,
		  e_1_to_m->ad_value_3,
		  AD_DERIV_CNT);
  e_1_to_m->rd                =
    e_1_state.decoded_instruction.rd;
  e_1_to_m->rs2               =
    e_1_state.decoded_instruction.rs2;
  e_1_to_m->rv1               =
    e_1_state.rv1;
  e_1_to_m->has_no_dest       =
    e_1_state.decoded_instruction.has_no_dest;
  e_1_to_m->is_load             =
    e_1_state.decoded_instruction.is_load;
  e_1_to_m->is_store            =
    e_1_state.decoded_instruction.is_store;
  e_1_to_m->is_ad_store         =
    e_1_state.decoded_instruction.is_ad_store;
  e_1_to_m->is_ad_op            =
    e_1_state.decoded_instruction.is_ad_op;
  e_1_to_m->func3               =
    e_1_state.decoded_instruction.func3;
#ifndef __SYNTHESIS__
  e_1_to_m->pc                  = e_1_state.pc;
  e_1_to_m->instruction         = e_1_state.instruction;
  e_1_to_m->decoded_instruction = e_1_state.decoded_instruction;
#endif
  e_1_to_e_2->result1             = result1;
  e_1_to_e_2->result2             =
    (e_1_state.decoded_instruction.is_ret)?
    (int)target_pc                        :
         result2;
  copy_array_32bit(e_1_to_e_2->ad_result1, ad_result1, AD_DERIV_CNT);
  copy_array_32bit(e_1_to_e_2->ad_result2, ad_result2, AD_DERIV_CNT);
  e_1_to_e_2->rd                  =
    e_1_state.decoded_instruction.rd;
  e_1_to_e_2->has_no_dest         =
    e_1_state.decoded_instruction.has_no_dest;
  e_1_to_e_2->is_r_type           =
    e_1_state.decoded_instruction.is_r_type;
  e_1_to_e_2->is_op_imm           =
    e_1_state.decoded_instruction.is_op_imm;
  e_1_to_e_2->is_ad_op           =
    e_1_state.decoded_instruction.is_ad_op;
  e_1_to_e_2->is_ret              =
    e_1_state.decoded_instruction.is_ret;
  e_1_to_e_2->is_exit              =
    e_1_state.decoded_instruction.is_exit;
#ifndef __SYNTHESIS__
  e_1_to_e_2->pc                  = e_1_state.pc;
  e_1_to_e_2->instruction         = e_1_state.instruction;
  e_1_to_e_2->decoded_instruction = e_1_state.decoded_instruction;
  e_1_to_e_2->target_pc           = target_pc;
#endif
}
void execute_1(
  from_i_to_e_1_t    e_1_from_i,
  bit_t              e_2_state_is_full,
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  reg_t             *reg_file,
#endif
#endif
  e_1_state_t       *e_1_state,
  from_e_1_to_f_t   *e_1_to_f,
  from_e_1_to_m_t   *e_1_to_m,
  from_e_1_to_e_2_t *e_1_to_e_2,
  bit_t             *e_1_state_is_full){
#pragma HLS INLINE off
  bit_t          save_input, bcond;
  int            result1, result2;
  int   		 ad_result1[AD_DERIV_CNT] = {0};
#pragma HLS ARRAY_PARTITION variable=ad_result1 dim=1 complete
  int			 ad_result2[AD_DERIV_CNT] = {0};
#pragma HLS ARRAY_PARTITION variable=ad_result2 dim=1 complete
  code_address_t tpc, target_pc;
  save_input = e_1_from_i.is_valid && (!e_1_state->is_full);
  if (save_input)
  get_input(e_1_from_i, e_1_state);
  compute(
    e_1_state,
   &bcond,
   &result1,
   &result2,
   ad_result1,
   //ad_result2,
   &tpc);
  e_1_to_f->is_valid =
   (e_1_state->is_full                       ||
    e_1_from_i.is_valid)                     &&
   (e_1_state->decoded_instruction.is_branch ||
    e_1_state->decoded_instruction.is_jalr);
  e_1_to_e_2->is_valid =
   !e_2_state_is_full                        &&
   (e_1_state->is_full                       ||
    e_1_from_i.is_valid)                     &&
#if defined(__SYNTHESIS__) || defined(DEBUG_PIPELINE)
   !e_1_state->decoded_instruction.is_branch &&
#endif
   !e_1_state->decoded_instruction.is_load   &&
   !e_1_state->decoded_instruction.is_store  &&
   !e_1_state->decoded_instruction.is_ad_store;
  e_1_to_m->is_valid   =
   (e_1_state->is_full                     ||
    e_1_from_i.is_valid)                   &&
   (e_1_state->decoded_instruction.is_load ||
    e_1_state->decoded_instruction.is_store||
    e_1_state->decoded_instruction.is_ad_store);
  e_1_state->is_full =
    e_2_state_is_full   &&
   (e_1_from_i.is_valid ||
    e_1_state->is_full) &&
   !e_1_state->decoded_instruction.is_branch;
  *e_1_state_is_full = e_1_state->is_full;
  if (e_1_to_e_2->is_valid ||
      e_1_to_m->is_valid   ||
      e_1_to_f->is_valid){
    stage_job(*e_1_state, bcond, tpc, &target_pc);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("exe_ed_1 ");
    printf("%04d\n", (int)e_1_state->pc);
    if (e_1_state->decoded_instruction.is_branch ||
        e_1_state->decoded_instruction.is_jalr)
      emulate(reg_file,
              e_1_state->decoded_instruction,
              target_pc);
	#ifdef DEBUG_AD
	#ifndef __SYNTHESIS__
	printf("AD_DEBUG: EXECUTE-STAGE IS_AD_OP = %d\n", (unsigned int)e_1_state->decoded_instruction.is_ad_op);
	#endif
	#endif
#endif
#endif
    set_output(result1,
               result2,
			   ad_result1,
			   ad_result2,
               target_pc,
              *e_1_state,
               e_1_to_f,
               e_1_to_m,
               e_1_to_e_2);
  }
}
