#include "adrv32imf_mp_ip.h"

static void copy_d_i(
  decoded_instruction_t  d_i_in,
  decoded_instruction_t *d_i_out){
#pragma HLS INLINE
  d_i_out->inst_31     = d_i_in.inst_31;
  d_i_out->inst_30_25  = d_i_in.inst_30_25;
  d_i_out->inst_24_21  = d_i_in.inst_24_21;
  d_i_out->inst_20     = d_i_in.inst_20;
  d_i_out->inst_19_12  = d_i_in.inst_19_12;
  d_i_out->inst_11_8   = d_i_in.inst_11_8;
  d_i_out->inst_7      = d_i_in.inst_7;
  d_i_out->inst_6_2    = d_i_in.inst_6_2;
  d_i_out->opcode      = d_i_in.opcode;
  d_i_out->inst_1_0    = d_i_in.inst_1_0;
  d_i_out->rd          = d_i_in.rd;
  d_i_out->func3       = d_i_in.func3;
  d_i_out->rs1         = d_i_in.rs1;
  d_i_out->rs2         = d_i_in.rs2;
  d_i_out->func7       = d_i_in.func7;
  d_i_out->type        = d_i_in.type;
  d_i_out->imm         = d_i_in.imm;
  d_i_out->is_rs1_reg  = d_i_in.is_rs1_reg;
  d_i_out->is_rs2_reg  = d_i_in.is_rs2_reg;
  d_i_out->is_r_type   = d_i_in.is_r_type;
  d_i_out->is_mul      = d_i_in.is_mul;
  d_i_out->is_div      = d_i_in.is_div;
  d_i_out->is_load     = d_i_in.is_load;
  d_i_out->is_store    = d_i_in.is_store;
#ifdef AD
  d_i_out->is_ad_store = d_i_in.is_ad_store;
  d_i_out->is_ad_op	   = d_i_in.is_ad_op;		//needed for writeback - if false, no writeback to ad-registers
#endif
  d_i_out->is_branch   = d_i_in.is_branch;
  d_i_out->is_jal      = d_i_in.is_jal;
  d_i_out->is_jalr     = d_i_in.is_jalr;
  d_i_out->is_ret      = d_i_in.is_ret;
  d_i_out->is_lui      = d_i_in.is_lui;
  d_i_out->is_op_imm   = d_i_in.is_op_imm;
  d_i_out->is_system   = d_i_in.is_system;
  d_i_out->is_mem      = d_i_in.is_mem;
  d_i_out->is_bjalr    = d_i_in.is_bjalr;
  d_i_out->is_jump     = d_i_in.is_jump;
  d_i_out->is_ctrl     = d_i_in.is_ctrl;
  d_i_out->has_no_dest = d_i_in.has_no_dest;
  d_i_out->opch        = d_i_in.opch;
  d_i_out->opcl        = d_i_in.opcl;
  d_i_out->is_null_rv2           = d_i_in.is_null_rv2;
  d_i_out->is_first_negative_rv2 = d_i_in.is_first_negative_rv2;
  d_i_out->is_last_negative_rv1  = d_i_in.is_last_negative_rv1;
#ifdef FPU
  d_i_out->rs3					= d_i_in.rs3;
  d_i_out->func5				= d_i_in.func5;
  d_i_out->func5l				= d_i_in.func5l;
  d_i_out->func5h				= d_i_in.func5h;
  d_i_out->is_rs3_reg			= d_i_in.is_rs3_reg;
  d_i_out->is_load_fp			= d_i_in.is_load_fp;
  d_i_out->is_store_fp			= d_i_in.is_store_fp;
  d_i_out->is_madd				= d_i_in.is_madd;
  d_i_out->is_msub				= d_i_in.is_msub;
  d_i_out->is_nmsub				= d_i_in.is_nmsub;
  d_i_out->is_nmadd				= d_i_in.is_nmadd;
  d_i_out->is_fused				= d_i_in.is_fused;
  d_i_out->is_op_fp				= d_i_in.is_op_fp;
  d_i_out->is_fsgnj				= d_i_in.is_fsgnj;
  d_i_out->is_fmin_max			= d_i_in.is_fmin_max;
  d_i_out->is_fsqrt				= d_i_in.is_fsqrt;
  d_i_out->is_fcmp				= d_i_in.is_fcmp;
  d_i_out->is_fcvt_w_s			= d_i_in.is_fcvt_w_s;
  d_i_out->is_fcvt_s_w			= d_i_in.is_fcvt_s_w;
  d_i_out->is_fmv_x_w			= d_i_in.is_fmv_x_w;
  d_i_out->is_fmv_w_x			= d_i_in.is_fmv_w_x;
  d_i_out->is_float				= d_i_in.is_float;
  d_i_out->is_execpt_inst		= d_i_in.is_execpt_inst;
#endif
  d_i_out->is_exit				= d_i_in.is_exit;
}
#ifdef FPU
static void copy_fpu_inst_for_wb(
  fpu_inst_for_wb_t  wb_i_in,
  fpu_inst_for_wb_t *wb_i_out){
	  //reg_num_t   rd;
	  wb_i_out->func3			 = wb_i_in.func3;
	  wb_i_out->func5			 = wb_i_in.func5;
	  wb_i_out->type			 = wb_i_in.type;
	  wb_i_out->is_load_fp		 = wb_i_in.is_load_fp;
	  wb_i_out->is_store_fp		 = wb_i_in.is_store_fp;
	  wb_i_out->is_store		 = wb_i_in.is_store;
	  wb_i_out->is_branch		 = wb_i_in.is_branch;
	  wb_i_out->is_system		 = wb_i_in.is_system;
	  wb_i_out->is_op_fp		 = wb_i_in.is_op_fp;
	  wb_i_out->is_fcmp			 = wb_i_in.is_fcmp;
	  wb_i_out->is_fcvt_w_s		 = wb_i_in.is_fcvt_w_s;
	  wb_i_out->is_fmv_x_w		 = wb_i_in.is_fmv_x_w;
	  wb_i_out->is_float		 = wb_i_in.is_float;
	  wb_i_out->is_execpt_inst	 = wb_i_in.is_execpt_inst;
	  wb_i_out->has_no_dest		 = wb_i_in.has_no_dest;
	  wb_i_out->fcsr			 = wb_i_in.fcsr;
	  wb_i_out->rv1			 	 = wb_i_in.rv1;
	  wb_i_out->rv2			 	 = wb_i_in.rv2;
}
#endif

#ifdef FETCH_EVERY_CYCLE
static void copy_f_to_f(
  from_f_to_f_t  f_to_f,
  from_f_to_f_t *f_from_f){
#pragma HLS INLINE
  f_from_f->is_valid  = f_to_f.is_valid;
  f_from_f->next_pc   = f_to_f.next_pc;
}
#endif

static void copy_d_to_f(
  from_d_to_f_t  d_to_f,
  from_d_to_f_t *f_from_d){
#pragma HLS INLINE
  f_from_d->is_valid  = d_to_f.is_valid;
  f_from_d->target_pc = d_to_f.target_pc;
}
static void copy_e_1_to_f(
  from_e_1_to_f_t  e_1_to_f,
  from_e_1_to_f_t *f_from_e_1){
#pragma HLS INLINE
  f_from_e_1->is_valid  = e_1_to_f.is_valid;
  f_from_e_1->target_pc = e_1_to_f.target_pc;
}
static void copy_f_to_d(
  from_f_to_d_t  f_to_d,
  from_f_to_d_t *d_from_f){
#pragma HLS INLINE
  d_from_f->is_valid    = f_to_d.is_valid;
  d_from_f->pc          = f_to_d.pc;
  d_from_f->instruction = f_to_d.instruction;
}
static void copy_d_to_i(
  from_d_to_i_t  d_to_i,
  from_d_to_i_t *i_from_d){
#pragma HLS INLINE
  i_from_d->is_valid    = d_to_i.is_valid;
  i_from_d->pc          = d_to_i.pc;
#ifndef __SYNTHESIS__
  i_from_d->instruction = d_to_i.instruction;
  i_from_d->target_pc   = d_to_i.target_pc;
#endif
  copy_d_i(d_to_i.decoded_instruction,
       &i_from_d->decoded_instruction);
}
static void copy_i_to_e_1(
  from_i_to_e_1_t  i_to_e_1,
  from_i_to_e_1_t *e_1_from_i){
#pragma HLS INLINE
  e_1_from_i->is_valid    = i_to_e_1.is_valid;
  e_1_from_i->rv1         = i_to_e_1.rv1;
  e_1_from_i->rv2         = i_to_e_1.rv2;
#ifdef AD
  copy_array_float_int_t(e_1_from_i->ad_rv1, i_to_e_1.ad_rv1, AD_DERIV_CNT);
  copy_array_float_int_t(e_1_from_i->ad_rv2, i_to_e_1.ad_rv2, AD_DERIV_CNT);
#endif
  e_1_from_i->pc          = i_to_e_1.pc;
#ifdef FPU
  e_1_from_i->rv3         = i_to_e_1.rv3;
#ifdef AD
  copy_array_float_int_t(e_1_from_i->ad_rv3, i_to_e_1.ad_rv3, AD_DERIV_CNT);
#endif
  e_1_from_i->fcsr        = i_to_e_1.fcsr;
#endif
#ifndef __SYNTHESIS__
  e_1_from_i->instruction = i_to_e_1.instruction;
  e_1_from_i->target_pc   = i_to_e_1.target_pc;
#endif
  copy_d_i(i_to_e_1.decoded_instruction,
       &e_1_from_i->decoded_instruction);
}
static void copy_e_1_to_e_2(
  from_e_1_to_e_2_t  e_1_to_e_2,
  from_e_1_to_e_2_t *e_2_from_e_1){
#pragma HLS INLINE
  e_2_from_e_1->is_valid    = e_1_to_e_2.is_valid;
  e_2_from_e_1->rd          = e_1_to_e_2.rd;
  e_2_from_e_1->has_no_dest = e_1_to_e_2.has_no_dest;
  e_2_from_e_1->is_r_type   = e_1_to_e_2.is_r_type;
  e_2_from_e_1->is_op_imm   = e_1_to_e_2.is_op_imm;
#ifdef AD
  e_2_from_e_1->is_ad_op   = e_1_to_e_2.is_ad_op;
#endif
  e_2_from_e_1->is_ret      = e_1_to_e_2.is_ret;
  e_2_from_e_1->result1     = e_1_to_e_2.result1;
  e_2_from_e_1->result2     = e_1_to_e_2.result2;
#ifdef AD
  copy_array_float_int_t(e_2_from_e_1->ad_result1, e_1_to_e_2.ad_result1, AD_DERIV_CNT);
  copy_array_float_int_t(e_2_from_e_1->ad_result2, e_1_to_e_2.ad_result2, AD_DERIV_CNT);
#endif
  e_2_from_e_1->is_exit     = e_1_to_e_2.is_exit;
#ifndef __SYNTHESIS__
  e_2_from_e_1->pc          = e_1_to_e_2.pc;
  e_2_from_e_1->instruction = e_1_to_e_2.instruction;
  copy_d_i(e_1_to_e_2.decoded_instruction,
       &e_2_from_e_1->decoded_instruction);
  e_2_from_e_1->target_pc   = e_1_to_e_2.target_pc;
#endif
#ifdef FPU
  copy_fpu_inst_for_wb(e_1_to_e_2.wb_info_fpu, &e_2_from_e_1->wb_info_fpu);
#endif
}
static void copy_e_1_to_m(
  from_e_1_to_m_t  e_1_to_m,
  from_e_1_to_m_t *m_from_e_1){
#pragma HLS INLINE
  m_from_e_1->is_valid    = e_1_to_m.is_valid;
  m_from_e_1->address     = e_1_to_m.address;
  m_from_e_1->value_0     = e_1_to_m.value_0;
  m_from_e_1->value_1     = e_1_to_m.value_1;
  m_from_e_1->value_2     = e_1_to_m.value_2;
  m_from_e_1->value_3     = e_1_to_m.value_3;
#ifdef AD
  copy_array_8bit(m_from_e_1->ad_value_0, e_1_to_m.ad_value_0, AD_DERIV_CNT);
  copy_array_8bit(m_from_e_1->ad_value_1, e_1_to_m.ad_value_1, AD_DERIV_CNT);
  copy_array_8bit(m_from_e_1->ad_value_2, e_1_to_m.ad_value_2, AD_DERIV_CNT);
  copy_array_8bit(m_from_e_1->ad_value_3, e_1_to_m.ad_value_3, AD_DERIV_CNT);
#endif
  m_from_e_1->rd          = e_1_to_m.rd;
#ifdef AD
  m_from_e_1->rs2         = e_1_to_m.rs2;
  m_from_e_1->rv1         = e_1_to_m.rv1;
#endif
  m_from_e_1->has_no_dest = e_1_to_m.has_no_dest;
  m_from_e_1->is_load     = e_1_to_m.is_load;
  m_from_e_1->is_store    = e_1_to_m.is_store;
#ifdef AD
  m_from_e_1->is_ad_op    = e_1_to_m.is_ad_op;
  m_from_e_1->is_ad_store = e_1_to_m.is_ad_store;
  m_from_e_1->ad_channel  = e_1_to_m.ad_channel;
#endif
  m_from_e_1->func3       = e_1_to_m.func3;
#ifndef __SYNTHESIS__
  m_from_e_1->pc          = e_1_to_m.pc;
  m_from_e_1->instruction = e_1_to_m.instruction;
  copy_d_i(e_1_to_m.decoded_instruction,
       &m_from_e_1->decoded_instruction);
#endif
#ifdef FPU
  copy_fpu_inst_for_wb(e_1_to_m.wb_info_fpu, &m_from_e_1->wb_info_fpu);
#endif
}
static void copy_e_2_to_w(
  from_e_2_to_w_t  e_2_to_w,
  from_e_2_to_w_t *w_from_e_2){
#pragma HLS INLINE
  w_from_e_2->is_valid    = e_2_to_w.is_valid;
  w_from_e_2->result      = e_2_to_w.result;
#ifdef AD
  copy_array_float_int_t(w_from_e_2->ad_result, e_2_to_w.ad_result, AD_DERIV_CNT);
#endif
  w_from_e_2->rd          = e_2_to_w.rd;
#ifdef AD
  w_from_e_2->is_ad_op    = e_2_to_w.is_ad_op;
#endif
  w_from_e_2->has_no_dest = e_2_to_w.has_no_dest;
  w_from_e_2->is_ret      = e_2_to_w.is_ret;
  w_from_e_2->is_exit      = e_2_to_w.is_exit;
#ifndef __SYNTHESIS__
  w_from_e_2->pc          = e_2_to_w.pc;
  w_from_e_2->instruction = e_2_to_w.instruction;
  copy_d_i(e_2_to_w.decoded_instruction,
       &w_from_e_2->decoded_instruction);
  w_from_e_2->target_pc   = e_2_to_w.target_pc;
#endif
#ifdef FPU
  copy_fpu_inst_for_wb(e_2_to_w.wb_info_fpu, &w_from_e_2->wb_info_fpu);
#endif
}
static void copy_m_to_w(
  from_m_to_w_t  m_to_w,
  from_m_to_w_t *w_from_m){
#pragma HLS INLINE
  w_from_m->is_valid      = m_to_w.is_valid;
  w_from_m->result        = m_to_w.result;
  w_from_m->value_0       = m_to_w.value_0;
  w_from_m->value_1       = m_to_w.value_1;
  w_from_m->value_2       = m_to_w.value_2;
  w_from_m->value_3       = m_to_w.value_3;
#ifdef AD
  copy_array_8bit(w_from_m->ad_value_0, m_to_w.ad_value_0, AD_DERIV_CNT);
  copy_array_8bit(w_from_m->ad_value_1, m_to_w.ad_value_1, AD_DERIV_CNT);
  copy_array_8bit(w_from_m->ad_value_2, m_to_w.ad_value_2, AD_DERIV_CNT);
  copy_array_8bit(w_from_m->ad_value_3, m_to_w.ad_value_3, AD_DERIV_CNT);
#endif
  w_from_m->a01           = m_to_w.a01;
  w_from_m->a1            = m_to_w.a1;
  w_from_m->msize         = m_to_w.msize;
  w_from_m->rd            = m_to_w.rd;
  w_from_m->has_no_dest   = m_to_w.has_no_dest;
#ifdef AD
  w_from_m->is_ad_op      = m_to_w.is_ad_op;
  w_from_m->is_ad_store   = m_to_w.is_ad_store;
#endif
#ifndef __SYNTHESIS__
  w_from_m->pc            = m_to_w.pc;
  w_from_m->instruction   = m_to_w.instruction;
  copy_d_i(m_to_w.decoded_instruction,
		&w_from_m->decoded_instruction);
#endif
#ifdef FPU
  copy_fpu_inst_for_wb(m_to_w.wb_info_fpu, &w_from_m->wb_info_fpu);
#endif
}
void clear_cycle(
#ifdef FETCH_EVERY_CYCLE
  bit_t *f_to_f_is_valid,
#endif
  bit_t *f_to_d_is_valid,
  bit_t *d_to_f_is_valid,
  bit_t *d_to_i_is_valid,
  bit_t *i_to_e_1_is_valid,
  bit_t *e_1_to_f_is_valid,
  bit_t *e_1_to_e_2_is_valid,
  bit_t *e_1_to_m_is_valid,
  bit_t *e_2_to_w_is_valid,
  bit_t *m_to_w_is_valid
#ifdef RUNNING_CONDITION_EXIT
  ,bit_t *w_from_e_2_is_exit
#endif
  ){
#pragma HLS INLINE
#ifdef FETCH_EVERY_CYCLE
  *f_to_f_is_valid     = 0;
#endif
  *f_to_d_is_valid     = 0;
  *d_to_f_is_valid     = 0;
  *d_to_i_is_valid     = 0;
  *i_to_e_1_is_valid   = 0;
  *e_1_to_f_is_valid   = 0;
  *e_1_to_e_2_is_valid = 0;
  *e_1_to_m_is_valid   = 0;
  *e_2_to_w_is_valid   = 0;
  *m_to_w_is_valid     = 0;
#ifdef RUNNING_CONDITION_EXIT
  *w_from_e_2_is_exit  = 0;
#endif
}
void new_cycle(
#ifdef FETCH_EVERY_CYCLE
  from_f_to_f_t      f_to_f,
#endif
  from_f_to_d_t      f_to_d,
  from_d_to_f_t      d_to_f,
  from_d_to_i_t      d_to_i,
  from_i_to_e_1_t    i_to_e_1,
  from_e_1_to_f_t    e_1_to_f,
  from_e_1_to_e_2_t  e_1_to_e_2,
  from_e_1_to_m_t    e_1_to_m,
  from_e_2_to_w_t    e_2_to_w,
  from_m_to_w_t      m_to_w,
  mul_latency_t      mul_latency_out,
  div_latency_t      div_latency_out,
#ifdef FPU
  fpu_latency_t		 fpu_latency_out,
#endif
  int                mul_result_out,
  int                div_result_out,
#ifdef FPU
  float_int_t		 fpu_result_out,
#ifdef AD
  float_int_t		 ad_mul_result_out[AD_DERIV_CNT],
  float_int_t		 ad_div_result_out[AD_DERIV_CNT],
  float_int_t		 ad_fpu_result_out[AD_DERIV_CNT],
#endif
#endif
  bit_t              is_mul_in_flight_out,
  bit_t              is_div_in_flight_out,
#ifdef FPU
  bit_t 			 is_fpu_in_flight_out,
#endif
  bit_t              f_state_is_full_out,
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
  bit_t              d_state_is_full_out,
  bit_t              i_state_is_full_out,
  bit_t              e_1_state_is_full_out,
  bit_t              e_2_state_is_full_out,
  bit_t              m_state_is_full_out,
#endif  
#ifdef FETCH_EVERY_CYCLE
  from_f_to_f_t     *f_from_f,
#endif
  from_d_to_f_t     *f_from_d,
  from_e_1_to_f_t   *f_from_e_1,
  from_f_to_d_t     *d_from_f,
  from_d_to_i_t     *i_from_d,
  from_i_to_e_1_t   *e_1_from_i,
  from_e_1_to_e_2_t *e_2_from_e_1,
  from_e_1_to_m_t   *m_from_e_1,
  from_e_2_to_w_t   *w_from_e_2,
  from_m_to_w_t     *w_from_m,
  mul_latency_t     *mul_latency_in,
  div_latency_t     *div_latency_in,
#ifdef FPU
  fpu_latency_t		*fpu_latency_in,
#endif
  int               *mul_result_in,
  int               *div_result_in,
#ifdef FPU
  float_int_t		*fpu_result_in,
#ifdef AD
  float_int_t		 ad_mul_result_in[AD_DERIV_CNT],
  float_int_t		 ad_div_result_in[AD_DERIV_CNT],
  float_int_t		 ad_fpu_result_in[AD_DERIV_CNT],
#endif
#endif
  bit_t             *is_mul_in_flight_in,
  bit_t             *is_div_in_flight_in,
#ifdef FPU
  bit_t				*is_fpu_in_flight_in,
#endif
  bit_t             *f_state_is_full_in
#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
  ,bit_t             *d_state_is_full_in,
  bit_t             *i_state_is_full_in,
  bit_t             *e_1_state_is_full_in,
  bit_t             *e_2_state_is_full_in,
  bit_t             *m_state_is_full_in
#endif
  ){
#pragma HLS INLINE off
#ifdef FETCH_EVERY_CYCLE
  copy_f_to_f    (f_to_f,     f_from_f);
#endif									/*
  *f_from_d     = d_to_f;*/
  copy_d_to_f    (d_to_f,     f_from_d);/*
  *f_from_d     = d_to_f;*/
  copy_e_1_to_f  (e_1_to_f,   f_from_e_1);/*
  *f_from_e_1   = e_1_to_f;*/
  copy_f_to_d    (f_to_d,     d_from_f);/*
  *d_from_f     = f_to_d;*/
  copy_d_to_i    (d_to_i,     i_from_d);/*
  *i_from_d     = d_to_i;*/
  copy_i_to_e_1  (i_to_e_1,   e_1_from_i);/*
  *e_1_from_i   = i_to_e_1;*/
  copy_e_1_to_e_2(e_1_to_e_2, e_2_from_e_1);/*
  *e_2_from_e_1 = e_1_to_e_2;*/
  copy_e_1_to_m  (e_1_to_m,   m_from_e_1);/*
  *m_from_e_1   = e_1_to_m;*/
  copy_e_2_to_w  (e_2_to_w,   w_from_e_2);/*
  *w_from_e_2   = e_2_to_w;*/
  copy_m_to_w    (m_to_w,     w_from_m);/*
  *w_from_m     = m_to_w;*/
  *mul_latency_in      = mul_latency_out;
  *div_latency_in      = div_latency_out;
#ifdef FPU
  *fpu_latency_in	   = fpu_latency_out;
#endif

  if (mul_latency_out == MUL_LATENCY){
    *mul_result_in     = mul_result_out;
#ifdef AD
    copy_array_float_int_t(ad_mul_result_in, ad_mul_result_out, AD_DERIV_CNT);
#endif
  }

  if (div_latency_out == DIV_LATENCY){
    *div_result_in     = div_result_out;
#ifdef AD
    copy_array_float_int_t(ad_div_result_in, ad_div_result_out, AD_DERIV_CNT);
#endif
  }


#ifdef FPU
  if (fpu_latency_out == FPU_LATENCY){
	  *fpu_result_in = fpu_result_out;
#ifdef AD
    copy_array_float_int_t(ad_fpu_result_in, ad_fpu_result_out, AD_DERIV_CNT);
#endif
  }
#endif

  *is_mul_in_flight_in = is_mul_in_flight_out;
  *is_div_in_flight_in = is_div_in_flight_out;
#ifdef FPU
  *is_fpu_in_flight_in = is_fpu_in_flight_out;
#endif
  *f_state_is_full_in  = f_state_is_full_out;

#ifdef USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS
  *d_state_is_full_in  = d_state_is_full_out;
  *i_state_is_full_in  = i_state_is_full_out;
  *e_1_state_is_full_in  = e_1_state_is_full_out;
  *e_2_state_is_full_in  = e_2_state_is_full_out;
  *m_state_is_full_in  = m_state_is_full_out;
#endif

#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifdef DEBUG_ADHOC
    //printf("new_cycle(): \n");
#endif
#endif
#endif
}
