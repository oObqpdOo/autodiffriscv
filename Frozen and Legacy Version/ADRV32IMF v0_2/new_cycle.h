#ifndef __NEW_CYCLE
#define __NEW_CYCLE

#include "adrv32imf_mp_ip.h"

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
  );
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
);

#endif

