#include "debug_adrv32imf_mp_ip.h"
#include "adrv32imf_mp_ip.h"
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
  e_1_state_t    *e_1_state){
#pragma HLS INLINE
  e_1_state->rv1                 = e_1_from_i.rv1;
  e_1_state->rv2                 = e_1_from_i.rv2;
  e_1_state->pc                  = e_1_from_i.pc;
  e_1_state->decoded_instruction = e_1_from_i.decoded_instruction;
#ifdef FPU
  e_1_state->rv3				 = e_1_from_i.rv3;
#ifdef AD
  copy_array_float_int_t(e_1_state->ad_rv1, e_1_from_i.ad_rv1, AD_DERIV_CNT);
  copy_array_float_int_t(e_1_state->ad_rv2, e_1_from_i.ad_rv2, AD_DERIV_CNT);
  copy_array_float_int_t(e_1_state->ad_rv3, e_1_from_i.ad_rv3, AD_DERIV_CNT);
#endif
  e_1_state->fcsr				 = e_1_from_i.fcsr;
#endif
#ifndef __SYNTHESIS__
  e_1_state->target_pc           = e_1_from_i.target_pc;
  e_1_state->instruction         = e_1_from_i.instruction;
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_ON_THE_FLY
#ifdef FPU
	/*
    printf("=======================\n");
    printf("E1-FROM-I-Stage: INPUT\n");
    printf("E1-FROM-I-Stage: is_float: %d\n", 		(int)e_1_from_i.decoded_instruction.is_float);
    printf("E1-FROM-I-Stage: is_op_fp: %d\n", 		(int)e_1_from_i.decoded_instruction.is_op_fp);
    printf("E1-FROM-I-Stage: is_fused: %d\n", 		(int)e_1_from_i.decoded_instruction.is_fused);
    printf("E1-FROM-I-Stage: is_load_fp: %d\n", 	(int)e_1_from_i.decoded_instruction.is_load_fp);
    printf("E1-FROM-I-Stage: is_store_fp: %d\n", 	(int)e_1_from_i.decoded_instruction.is_store_fp);
    printf("=======================\n");
	printf("=======================\n");
    printf("E1-Stage: GET_INPUT\n");
    printf("E1-Stage: is_float: %d\n", 		(int)e_1_state->decoded_instruction.is_float);
    printf("E1-Stage: is_op_fp: %d\n", 		(int)e_1_state->decoded_instruction.is_op_fp);
    printf("E1-Stage: is_fused: %d\n", 		(int)e_1_state->decoded_instruction.is_fused);
    printf("E1-Stage: is_load_fp: %d\n", 	(int)e_1_state->decoded_instruction.is_load_fp);
    printf("E1-Stage: is_store_fp: %d\n", 	(int)e_1_state->decoded_instruction.is_store_fp);
    printf("=======================\n");
    */
#endif
#endif
#endif
}
//===================================================
//TODO:
//I have to add the computation: will be last step!
//===================================================
static void compute(
  e_1_state_t    *e_1_state,
  bit_t          *bcond,
  float_int_t    *result1,
  float_int_t    *result2,
#ifdef AD
  float_int_t	 ad_result1[AD_DERIV_CNT],
#endif
  code_address_t *target_pc){
#pragma HLS INLINE
  *bcond = compute_branch_result(
    e_1_state->rv1.i,
    e_1_state->rv2.i,
    e_1_state->decoded_instruction.func3);
  //TODO: this is either float or int?

  //TODO: THERE IS AN ERROR WHERE OUTPUTS VARY DEPENDING ON RUN: WHY???
  //TODO: this shows an FP op, although there IS no FP op...
#ifdef FPU
  //if(e_1_state->decoded_instruction.is_float){
  //TODO: is this the correct condition?
#ifndef EXTERNAL_FPU_COMP
  if(e_1_state->decoded_instruction.is_op_fp || e_1_state->decoded_instruction.is_fused ){
	  *result1 = compute_fp_op_result(
	  		e_1_state->rv1,
	  		e_1_state->rv2,
	  		e_1_state->rv3,
	  		e_1_state->decoded_instruction);
	  //TODO: Why is is_float 1??? although it is NO float operation
#ifndef __SYNTHESIS__
#ifdef DEBUG_ON_THE_FLY
#ifdef FPU
	printf("=======================\n");
    printf("E1-Stage: IS FP OP\n");
    printf("E1-Stage: is_float: %d\n", (int)e_1_state->decoded_instruction.is_float);
    printf("E1-Stage: is_op_fp: %d\n", (int)e_1_state->decoded_instruction.is_op_fp);
    printf("E1-Stage: is_fused: %d\n", (int)e_1_state->decoded_instruction.is_fused);
    printf("E1-Stage: is_load_fp: %d\n", (int)e_1_state->decoded_instruction.is_load_fp);
    printf("E1-Stage: is_store_fp: %d\n", (int)e_1_state->decoded_instruction.is_store_fp);
    printf("=======================\n");
#endif
#endif
#endif
  }
  else{
#endif
#endif
#ifdef AD
	//pass pointers for results, computation has to change values directly
	//TODO: but d_i is NOT a pointer, but copy;
	//so e_1_state->decoded_instruction.is_ad_op is not modified IN function
	//TODO: clean this up? This pointer passing could massively decrease performance right?
	int tmp_ad_rv1[AD_DERIV_CNT];
	int tmp_ad_rv2[AD_DERIV_CNT];
	int tmp_ad_result1[AD_DERIV_CNT];
	//TODO: understand why the following creates extreme neg. slack due to dependencies?
	#pragma HLS ARRAY_PARTITION variable=tmp_ad_rv1 dim=1 complete
	#pragma HLS ARRAY_PARTITION variable=tmp_ad_rv2 dim=1 complete
	#pragma HLS ARRAY_PARTITION variable=tmp_ad_result1 dim=1 complete
	int tmp_result;
	copy_float_int_t_array_to_int(tmp_ad_rv1, e_1_state->ad_rv1, AD_DERIV_CNT);
	copy_float_int_t_array_to_int(tmp_ad_rv2, e_1_state->ad_rv2, AD_DERIV_CNT);
	//copy_float_int_t_array_to_int(tmp_ad_result1, ad_result1, AD_DERIV_CNT);
	tmp_result = (*result1).i;
	e_1_state->decoded_instruction.is_ad_op = compute_ad_op_result(
											e_1_state->rv1.i,
											e_1_state->rv2.i,
											e_1_state->fcsr,
											tmp_ad_rv1,
											tmp_ad_rv2,
											&tmp_result,
											tmp_ad_result1,
											e_1_state->decoded_instruction);
	copy_int_array_to_float_int_t(e_1_state->ad_rv1, tmp_ad_rv1, AD_DERIV_CNT);
	copy_int_array_to_float_int_t(e_1_state->ad_rv2, tmp_ad_rv2, AD_DERIV_CNT);
	copy_int_array_to_float_int_t(ad_result1, tmp_ad_result1, AD_DERIV_CNT);
	(*result1).i = tmp_result;
#else //if we don't use AD we use old compute_op_result
    int tempresult1 = compute_op_result(
      e_1_state->rv1.i,
      e_1_state->rv2.i,
#ifdef FPU
	  e_1_state->fcsr,
#endif
      e_1_state->decoded_instruction);
    (*result1).i = tempresult1;
#endif

#ifndef __SYNTHESIS__
#ifdef DEBUG_ON_THE_FLY
#ifdef FPU
    printf("=======================\n");
    printf("E1-Stage: IS NORMAL OP\n");
    printf("E1-Stage: is_float: %d\n", (int)e_1_state->decoded_instruction.is_float);
    printf("E1-Stage: is_op_fp: %d\n", (int)e_1_state->decoded_instruction.is_op_fp);
    printf("E1-Stage: is_fused: %d\n", (int)e_1_state->decoded_instruction.is_fused);
    printf("E1-Stage: is_load_fp: %d\n", (int)e_1_state->decoded_instruction.is_load_fp);
    printf("E1-Stage: is_store_fp: %d\n", (int)e_1_state->decoded_instruction.is_store_fp);
    printf("=======================\n");
#endif
#endif
#endif
#ifdef FPU
#ifndef EXTERNAL_FPU_COMP
  }
#endif
#endif
  //TODO: is this temp variable slowing me down?
  int tempresult2 = compute_result(
    e_1_state->rv1.i,
    e_1_state->pc,
    e_1_state->decoded_instruction);
  (*result2).i = tempresult2;
  *target_pc = compute_next_pc(
    e_1_state->pc,
    e_1_state->decoded_instruction,
    e_1_state->rv1.i);
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

#ifdef AD
static void shift_results(
	float_int_t in_value[],
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
			out_s_value0[i] = in_value[i].i;
			out_s_value1[i] = in_value[i].i >> 8;
			out_s_value2[i] = in_value[i].i >> 16;
			out_s_value3[i] = in_value[i].i >> 24;
	}
}
#endif

static void set_output_to_m(
  float_int_t      result2,
#ifdef AD
  float_int_t 	   ad_result2[AD_DERIV_CNT],
#endif
  e_1_state_t      e_1_state,
  from_e_1_to_m_t *e_1_to_m){
#pragma HLS INLINE
  e_1_to_m->address           = result2.i;
#ifdef AD
  e_1_to_m->ad_channel		  = e_1_state.decoded_instruction.imm;
#endif
  e_1_to_m->value_0           = e_1_state.rv2.i;
  e_1_to_m->value_1           = e_1_state.rv2.i >> 8;
  e_1_to_m->value_2           = e_1_state.rv2.i >> 16;
  e_1_to_m->value_3           = e_1_state.rv2.i >> 24;
#ifdef AD
  //TODO: These are duplicates that should be already in e_1_to_m->wb_info_fpu
  e_1_to_m->rs2               = e_1_state.decoded_instruction.rs2;
  e_1_to_m->rv1               = e_1_state.rv1.i;
  //TODO: create shifts
  shift_results(
		  e_1_state.ad_rv2,
		  e_1_to_m->ad_value_0,
		  e_1_to_m->ad_value_1,
		  e_1_to_m->ad_value_2,
		  e_1_to_m->ad_value_3,
		  AD_DERIV_CNT);
#endif
  e_1_to_m->rd                =
    e_1_state.decoded_instruction.rd;
  e_1_to_m->has_no_dest       =
    e_1_state.decoded_instruction.has_no_dest;
  e_1_to_m->is_load             =
    e_1_state.decoded_instruction.is_load
#ifdef FPU
	|| e_1_state.decoded_instruction.is_load_fp
#endif
	;
  e_1_to_m->is_store            =
    e_1_state.decoded_instruction.is_store
#ifdef FPU
	|| e_1_state.decoded_instruction.is_store_fp
#endif
	;
#ifdef AD
  e_1_to_m->is_ad_store         =
    e_1_state.decoded_instruction.is_ad_store;
  e_1_to_m->is_ad_op            =
    e_1_state.decoded_instruction.is_ad_op;
#endif
  e_1_to_m->func3               =
    e_1_state.decoded_instruction.func3;
#ifdef FPU
  e_1_to_m->wb_info_fpu.func3 				= e_1_state.decoded_instruction.func3;
  e_1_to_m->wb_info_fpu.func5 				= e_1_state.decoded_instruction.func5;
  e_1_to_m->wb_info_fpu.has_no_dest 		= e_1_state.decoded_instruction.has_no_dest;
  e_1_to_m->wb_info_fpu.is_execpt_inst 		= e_1_state.decoded_instruction.is_execpt_inst;
  e_1_to_m->wb_info_fpu.is_fcmp 			= e_1_state.decoded_instruction.is_fcmp;
  e_1_to_m->wb_info_fpu.is_fcvt_w_s 		= e_1_state.decoded_instruction.is_fcvt_w_s;
  e_1_to_m->wb_info_fpu.is_float 			= e_1_state.decoded_instruction.is_float;
  e_1_to_m->wb_info_fpu.is_fmv_x_w 			= e_1_state.decoded_instruction.is_fmv_x_w;
  e_1_to_m->wb_info_fpu.is_load_fp 			= e_1_state.decoded_instruction.is_load_fp;
  e_1_to_m->wb_info_fpu.is_op_fp 			= e_1_state.decoded_instruction.is_op_fp;
  e_1_to_m->wb_info_fpu.is_store_fp 		= e_1_state.decoded_instruction.is_store_fp;
  //e_1_to_m->wb_info_fpu.rd 					= e_1_state.decoded_instruction.rd;
  e_1_to_m->wb_info_fpu.type 				= e_1_state.decoded_instruction.type;
  e_1_to_m->wb_info_fpu.rv1 				= e_1_state.rv1;
  e_1_to_m->wb_info_fpu.rv2					= e_1_state.rv2;
  e_1_to_m->wb_info_fpu.is_store 			= e_1_state.decoded_instruction.is_store;
  e_1_to_m->wb_info_fpu.is_branch 			= e_1_state.decoded_instruction.is_branch;
  e_1_to_m->wb_info_fpu.is_system 			= e_1_state.decoded_instruction.is_system;
  e_1_to_m->wb_info_fpu.fcsr				= e_1_state.fcsr;
#endif
#ifndef __SYNTHESIS__
  e_1_to_m->pc                  = e_1_state.pc;
  e_1_to_m->instruction         = e_1_state.instruction;
  e_1_to_m->decoded_instruction = e_1_state.decoded_instruction;
#endif
}
static void set_output_to_e_2(
  float_int_t        result1,
  float_int_t        result2,
#ifdef AD
  float_int_t        ad_result1[AD_DERIV_CNT],
  float_int_t        ad_result2[AD_DERIV_CNT],
#endif
  code_address_t     target_pc,
  e_1_state_t        e_1_state,
  from_e_1_to_e_2_t *e_1_to_e_2){
#pragma HLS INLINE
  e_1_to_e_2->result1             = result1;
  //TODO: I dislike this typecast solution very much
  float_int_t target_pc_as_float_int_t;
  target_pc_as_float_int_t.i = (int)target_pc ;
  e_1_to_e_2->result2             =
    (e_1_state.decoded_instruction.is_ret)?
    	 target_pc_as_float_int_t :
         result2;
#ifdef AD
  copy_array_float_int_t(e_1_to_e_2->ad_result1, ad_result1, AD_DERIV_CNT);
  copy_array_float_int_t(e_1_to_e_2->ad_result2, ad_result2, AD_DERIV_CNT);
#endif
  e_1_to_e_2->rd                  =
    e_1_state.decoded_instruction.rd;
  e_1_to_e_2->has_no_dest         =
    e_1_state.decoded_instruction.has_no_dest;
  e_1_to_e_2->is_r_type           =
    e_1_state.decoded_instruction.is_r_type;
  e_1_to_e_2->is_op_imm           =
    e_1_state.decoded_instruction.is_op_imm;
#ifdef AD
  e_1_to_e_2->is_ad_op           =
    e_1_state.decoded_instruction.is_ad_op;
#endif
  e_1_to_e_2->is_ret              =
    e_1_state.decoded_instruction.is_ret;
  e_1_to_e_2->is_exit              =
    e_1_state.decoded_instruction.is_exit;
#ifndef __SYNTHESIS__
#ifdef DEBUG_EXIT
       printf("E_1: IS_EXIT: %d\n", e_1_state.decoded_instruction.is_exit);
#endif
#endif
#ifdef FPU
  e_1_to_e_2->wb_info_fpu.func3 			= e_1_state.decoded_instruction.func3;
  e_1_to_e_2->wb_info_fpu.func5 			= e_1_state.decoded_instruction.func5;
  e_1_to_e_2->wb_info_fpu.has_no_dest 		= e_1_state.decoded_instruction.has_no_dest;
  e_1_to_e_2->wb_info_fpu.is_execpt_inst 	= e_1_state.decoded_instruction.is_execpt_inst;
  e_1_to_e_2->wb_info_fpu.is_fcmp 			= e_1_state.decoded_instruction.is_fcmp;
  e_1_to_e_2->wb_info_fpu.is_fcvt_w_s 		= e_1_state.decoded_instruction.is_fcvt_w_s;
  e_1_to_e_2->wb_info_fpu.is_float 			= e_1_state.decoded_instruction.is_float;
  e_1_to_e_2->wb_info_fpu.is_fmv_x_w 		= e_1_state.decoded_instruction.is_fmv_x_w;
  e_1_to_e_2->wb_info_fpu.is_load_fp 		= e_1_state.decoded_instruction.is_load_fp;
  e_1_to_e_2->wb_info_fpu.is_op_fp 			= e_1_state.decoded_instruction.is_op_fp;
  e_1_to_e_2->wb_info_fpu.is_store_fp 		= e_1_state.decoded_instruction.is_store_fp;
  //e_1_to_e_2->wb_info_fpu.rd 				= e_1_state.decoded_instruction.rd;
  e_1_to_e_2->wb_info_fpu.type 				= e_1_state.decoded_instruction.type;
  e_1_to_e_2->wb_info_fpu.rv1 				= e_1_state.rv1;
  e_1_to_e_2->wb_info_fpu.rv2				= e_1_state.rv2;
  e_1_to_e_2->wb_info_fpu.is_store 			= e_1_state.decoded_instruction.is_store;
  e_1_to_e_2->wb_info_fpu.is_branch 		= e_1_state.decoded_instruction.is_branch;
  e_1_to_e_2->wb_info_fpu.is_system 		= e_1_state.decoded_instruction.is_system;
  e_1_to_e_2->wb_info_fpu.fcsr				= e_1_state.fcsr;
#endif
#ifndef __SYNTHESIS__
  e_1_to_e_2->pc                  = e_1_state.pc;
  e_1_to_e_2->instruction         = e_1_state.instruction;
  e_1_to_e_2->decoded_instruction = e_1_state.decoded_instruction;
  e_1_to_e_2->target_pc           = target_pc;
#endif
}

static void set_output(
  float_int_t        result1,
  float_int_t        result2,
#ifdef AD
  float_int_t 		 ad_result1[AD_DERIV_CNT],
  float_int_t 		 ad_result2[AD_DERIV_CNT],
#endif
  code_address_t     target_pc,
  e_1_state_t        e_1_state,
  from_e_1_to_f_t   *e_1_to_f,
  from_e_1_to_m_t   *e_1_to_m,
  from_e_1_to_e_2_t *e_1_to_e_2){
#pragma HLS INLINE
  e_1_to_f->target_pc = target_pc;
  set_output_to_m(	result2,
#ifdef AD
		  	  	  	ad_result2,
#endif
					e_1_state,
					e_1_to_m);
  set_output_to_e_2(result1,
		  	  	  	result2,
#ifdef AD
					ad_result1,
					ad_result2,
#endif
					target_pc,
					e_1_state,
					e_1_to_e_2);
}
void prepare_execute_1(
  from_i_to_e_1_t    e_1_from_i,
  bit_t              e_2_state_is_full,
  bit_t              m_state_is_full,
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  int               *reg_file,
#ifdef FPU
  float				*freg_file,
#endif
#endif
#endif
#ifdef FPU
  int				*fcsr,				//TODO: only used in emulate? double check if rest uses e_1_state->fcsr as intended
#endif
  e_1_state_t       *e_1_state,
  from_e_1_to_f_t   *e_1_to_f,
  from_e_1_to_m_t   *e_1_to_m,
  from_e_1_to_e_2_t *e_1_to_e_2,
  bit_t             *e_1_state_is_full){
#ifdef PRAGMA_USE_INLINE_IN_MPP_EXECUTE
#pragma HLS INLINE
#else 
#pragma HLS INLINE off
#endif

#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("\n=====EXECUTE======\n");
#endif
#endif

  bit_t          save_input, bcond;
  float_int_t    result1;
  float_int_t    result2;
#ifdef AD
  float_int_t 	 ad_result1[AD_DERIV_CNT] = {0};
#pragma HLS ARRAY_PARTITION variable=ad_result1 dim=1 complete
  float_int_t	 ad_result2[AD_DERIV_CNT] = {0};
#pragma HLS ARRAY_PARTITION variable=ad_result2 dim=1 complete
#endif
  code_address_t tpc, target_pc;
  bit_t          valid_or_full;
  save_input    = e_1_from_i.is_valid && (!e_1_state->is_full);
  valid_or_full = e_1_from_i.is_valid ||   e_1_state->is_full;
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("e_1_from_i.is_valid: %d\n",(int)e_1_from_i.is_valid );
	printf("e_1_state->is_full: %d\n",(int)e_1_state->is_full);
#endif
#endif

  if (save_input){
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("EXECUTE: get_input()\n");
#endif
#endif
    get_input(e_1_from_i, e_1_state);
  }
  compute(e_1_state,
         &bcond,
         &result1,
         &result2,
#ifdef AD
		 ad_result1,
#endif
         &tpc);
  e_1_to_m->is_valid   =
   !e_2_state_is_full                        &&
   !m_state_is_full                          &&
   valid_or_full                             &&
   e_1_state->decoded_instruction.is_mem;    //here also FPU OPs are getting scheduled and contains .is_ad_store
  //Only valid if not div, mul or fpu-op
  e_1_to_e_2->is_valid =
   !e_2_state_is_full                        &&
   !m_state_is_full                          &&
   valid_or_full                             &&
#if defined(__SYNTHESIS__) || defined(DEBUG_PIPELINE)
   !e_1_state->decoded_instruction.is_branch &&
#endif
   !e_1_state->decoded_instruction.is_mem    && 	//contains .is_ad_store
#ifdef FPU
   !(e_1_state->decoded_instruction.is_op_fp  ||
     e_1_state->decoded_instruction.is_fused) &&
#endif
   !e_1_state->decoded_instruction.is_mul    &&
   !e_1_state->decoded_instruction.is_div;
  e_1_to_f->is_valid =
   !e_2_state_is_full                        &&
   !m_state_is_full                          &&
    valid_or_full                            &&
    e_1_state->decoded_instruction.is_bjalr  &&
   (!e_1_state->decoded_instruction.is_ret || tpc != 0);
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
#ifdef FPU
	  emulate(reg_file,
			  freg_file,
			  *fcsr,
			  e_1_state->decoded_instruction,
              target_pc);
#ifdef AD
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
      printf("AD_DEBUG: EXECUTE-STAGE IS_AD_OP = %d\n", (unsigned int)e_1_state->decoded_instruction.is_ad_op);
#endif
#endif
#endif
#else
      emulate(reg_file,
              e_1_state->decoded_instruction,
              target_pc);
#endif
#endif
#endif
    set_output(result1,
               result2,
#ifdef AD
			   ad_result1,
			   ad_result2,
#endif
               target_pc,
              *e_1_state,
               e_1_to_f,
               e_1_to_m,
               e_1_to_e_2);
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("EXECUTE: set_output()\n");
#endif
#endif
  }
  *e_1_state_is_full =
    valid_or_full        &&
   !e_1_to_f->is_valid   &&
   !e_1_to_e_2->is_valid &&
   !e_1_to_m->is_valid;
  e_1_state->is_full = *e_1_state_is_full;
}
void mul_execute_1(
  e_1_state_t e_1_state,
  int        *mul_result_out
#ifdef AD
  ,float_int_t ad_mul_result_out[AD_DERIV_CNT]
#endif
	){
#ifdef PRAGMA_USE_INLINE_IN_MPP_EXECUTE
#pragma HLS INLINE
#else
#pragma HLS INLINE off
#endif
#ifdef AD
  bit_t is_ad_op;
  *mul_result_out = compute_ad_mul_result(
	e_1_state.rv1.i,
	e_1_state.rv2.i,
	e_1_state.ad_rv1,
	e_1_state.ad_rv2,
	//mul_result_out,
	ad_mul_result_out,
	e_1_state.decoded_instruction,
	&is_ad_op);
  e_1_state.decoded_instruction.is_ad_op = is_ad_op;
#else
  *mul_result_out = compute_mul_result(
    e_1_state.rv1.i,
    e_1_state.rv2.i,
    e_1_state.decoded_instruction);
#endif
}
void div_execute_1(
  e_1_state_t e_1_state,
  int        *div_result_out
#ifdef AD
  ,float_int_t ad_div_result_out[AD_DERIV_CNT]
#endif
	){
#ifdef PRAGMA_USE_INLINE_IN_MPP_EXECUTE
#pragma HLS INLINE
#else
#pragma HLS INLINE off
#endif
#ifdef AD
    bit_t is_ad_op;
    *div_result_out = compute_ad_div_result(
        e_1_state.rv1.i,
        e_1_state.rv2.i,
        e_1_state.ad_rv1,
        e_1_state.ad_rv2,
        //div_result_out,
        ad_div_result_out,
        e_1_state.decoded_instruction,
        &is_ad_op);
    e_1_state.decoded_instruction.is_ad_op = is_ad_op;
#else
  *div_result_out = compute_div_result(
    e_1_state.rv1.i,
    e_1_state.rv2.i,
    e_1_state.decoded_instruction);
#endif
}
#ifdef FPU
void fpu_execute_1(
  e_1_state_t e_1_state,
  float_int_t *fpu_result_out
#ifdef AD
  ,float_int_t ad_fpu_result_out[AD_DERIV_CNT]
#endif
	){
#ifdef PRAGMA_USE_INLINE_IN_MPP_EXECUTE
#pragma HLS INLINE
#else
#pragma HLS INLINE off
#endif
#ifdef AD
    bit_t is_ad_op;
	*fpu_result_out = compute_fp_ad_op_result(
	e_1_state.rv1,
	e_1_state.rv2,
	e_1_state.rv3,
	e_1_state.ad_rv1,
	e_1_state.ad_rv2,
	e_1_state.ad_rv3,
	//fpu_result_out,
    ad_fpu_result_out,
	e_1_state.decoded_instruction,
    &is_ad_op);
    e_1_state.decoded_instruction.is_ad_op = is_ad_op;
#else
  *fpu_result_out = compute_fp_op_result(
    e_1_state.rv1,
    e_1_state.rv2,
	e_1_state.rv3,
    e_1_state.decoded_instruction);
#endif

}
#endif
void end_mul_execute_1(
  int                mul_result_in,
#ifdef AD
  float_int_t		 ad_mul_result_in[AD_DERIV_CNT],
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  int               *reg_file,
#endif
#endif
  e_1_state_t       *e_1_state,
  from_e_1_to_e_2_t *e_1_to_e_2,
  mul_latency_t     *mul_latency_out,
  bit_t             *e_1_state_is_full){
#ifdef PRAGMA_USE_INLINE_IN_MPP_EXECUTE
#pragma HLS INLINE
#else
#pragma HLS INLINE off
#endif
  *mul_latency_out     = 0;
  e_1_state->is_full   = 0;
  *e_1_state_is_full   = 0;
  //TODO: This is super ugly and temporarily to debug - better solution?
  float_int_t mul_result_in_float_int;
  mul_result_in_float_int.i = mul_result_in;
  float_int_t int_zero_as_float_int_t;
  int_zero_as_float_int_t.i = 0;
#ifdef AD
  float_int_t ad_mul_result_in_float_int[AD_DERIV_CNT];
  copy_array_float_int_t(ad_mul_result_in_float_int, ad_mul_result_in, AD_DERIV_CNT); //This has to be decoupled from previous mul_result_in
  float_int_t zero_as_float_int_t[AD_DERIV_CNT];
  copy_value_float_int_t(zero_as_float_int_t, &int_zero_as_float_int_t, AD_DERIV_CNT);
#endif
  set_output_to_e_2(mul_result_in_float_int,
		  	  	  	int_zero_as_float_int_t,
#ifdef AD
					ad_mul_result_in_float_int,
					zero_as_float_int_t,
#endif
					0,
					*e_1_state,
					e_1_to_e_2);
  e_1_to_e_2->is_valid = 1;
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  printf("exe_ed_1 ");
  printf("%04d\n", (int)e_1_state->pc);
#endif
#endif
}
void end_div_execute_1(
  int                div_result_in,
#ifdef AD
  float_int_t		 ad_div_result_in[AD_DERIV_CNT],
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  int               *reg_file,
#endif
#endif
  e_1_state_t       *e_1_state,
  from_e_1_to_e_2_t *e_1_to_e_2,
  div_latency_t     *div_latency_out,
  bit_t             *e_1_state_is_full){
#ifdef PRAGMA_USE_INLINE_IN_MPP_EXECUTE
#pragma HLS INLINE
#else
#pragma HLS INLINE off
#endif
  *div_latency_out     = 0;
  e_1_state->is_full   = 0;
  *e_1_state_is_full   = 0;
  //TODO: This is super ugly and temporarily to debug - better solution?
  float_int_t div_result_in_float_int;
  div_result_in_float_int.i = div_result_in;
  float_int_t int_zero_as_float_int_t;
  int_zero_as_float_int_t.i = 0;
#ifdef AD
  float_int_t ad_div_result_in_float_int[AD_DERIV_CNT];
  copy_array_float_int_t(ad_div_result_in_float_int, ad_div_result_in, AD_DERIV_CNT); //This has to be decoupled from previous div_result_in
  float_int_t zero_as_float_int_t[AD_DERIV_CNT];
  copy_value_float_int_t(zero_as_float_int_t, &int_zero_as_float_int_t, AD_DERIV_CNT);
#endif
  set_output_to_e_2(div_result_in_float_int,
		  	  	  	int_zero_as_float_int_t,
#ifdef AD
					ad_div_result_in_float_int,
					zero_as_float_int_t,
#endif
					0,
					*e_1_state,
					e_1_to_e_2);
  e_1_to_e_2->is_valid = 1;
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  printf("exe_ed_1 ");
  printf("%04d\n", (int)e_1_state->pc);
#endif
#endif
}
#ifdef FPU
void end_fpu_execute_1(
  float_int_t      fpu_result_in,
#ifdef AD
  float_int_t	   ad_fpu_result_in[AD_DERIV_CNT],
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  float             *freg_file,
#endif
#endif
  e_1_state_t       *e_1_state,
  from_e_1_to_e_2_t *e_1_to_e_2,
  fpu_latency_t     *fpu_latency_out,
  bit_t             *e_1_state_is_full){
#ifdef PRAGMA_USE_INLINE_IN_MPP_EXECUTE
#pragma HLS INLINE
#else
#pragma HLS INLINE off
#endif
  *fpu_latency_out     = 0;
  e_1_state->is_full   = 0;
  *e_1_state_is_full   = 0;
  //TODO: This is super ugly and temporarily to debug - better solution?
  float_int_t fpu_result_in_float_int;
  fpu_result_in_float_int = fpu_result_in;
  float_int_t int_zero_as_float_int_t;
  int_zero_as_float_int_t.i = 0;
#ifdef AD
  float_int_t ad_fpu_result_in_float_int[AD_DERIV_CNT];
  copy_array_float_int_t(ad_fpu_result_in_float_int, ad_fpu_result_in, AD_DERIV_CNT); //This has to be decoupled from previous fpu_result_in
  float_int_t zero_as_float_int_t[AD_DERIV_CNT];
  copy_value_float_int_t(zero_as_float_int_t, &int_zero_as_float_int_t, AD_DERIV_CNT);
#endif
  set_output_to_e_2(fpu_result_in_float_int,
		  	  	  	int_zero_as_float_int_t,
#ifdef AD
					ad_fpu_result_in_float_int,
					zero_as_float_int_t,
#endif
					0,
					*e_1_state,
					e_1_to_e_2);
  e_1_to_e_2->is_valid = 1;
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  printf("exe_ed_1 ");
  printf("%04d\n", (int)e_1_state->pc);
#endif
#endif
}
#endif
