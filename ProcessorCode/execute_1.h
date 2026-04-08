#ifndef __EXECUTE_1
#define __EXECUTE_1

#include "adrv32imf_mp_ip.h"

void init_e_1_state(e_1_state_t *e_1_state);
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
  int				*fcsr,
#endif
  e_1_state_t       *e_1_state,
  from_e_1_to_f_t   *e_1_to_f,
  from_e_1_to_m_t   *e_1_to_m,
  from_e_1_to_e_2_t *e_1_to_e_2,
  bit_t             *e_1_state_is_full);

void mul_execute_1(
  e_1_state_t e_1_state,
  int        *mul_result_out
#ifdef AD
  ,float_int_t ad_mul_result_out[AD_DERIV_CNT]
#endif
	);

void div_execute_1(
  e_1_state_t e_1_state,
  int        *div_result_out
#ifdef AD
  ,float_int_t ad_div_result_out[AD_DERIV_CNT]
#endif
	);
#ifdef FPU

void fpu_execute_1(
  e_1_state_t e_1_state,
  float_int_t *fpu_result_out
#ifdef AD
  ,float_int_t ad_fpu_result_out[AD_DERIV_CNT]
#endif
	);
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
  bit_t             *e_1_state_is_full);
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
  bit_t             *e_1_state_is_full);
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
  bit_t             *e_1_state_is_full);
#endif


#endif
