#ifndef __COMPUTE
#define __COMPUTE

#include "adrv32imf_mp_ip.h"

bit_t compute_branch_result(
  int rv1,
  int rv2,
  func3_t func3);

#ifdef AD

int compute_ad_mul_result(
  int 					rv1,
  int 					rv2,
  float_int_t			ad_rv1[AD_DERIV_CNT],
  float_int_t		 	ad_rv2[AD_DERIV_CNT],
  //int					*result,
  float_int_t			ad_result[AD_DERIV_CNT],
  decoded_instruction_t d_i,
  bit_t					*is_ad_op
  );

int compute_ad_div_result(
  int 					rv1,
  int 					rv2,
  float_int_t 			ad_rv1[AD_DERIV_CNT],
  float_int_t 			ad_rv2[AD_DERIV_CNT],
  //int					*result,
  float_int_t			ad_result[AD_DERIV_CNT],
  decoded_instruction_t d_i,
  bit_t					*is_ad_op);

#else

int compute_mul_result(
  int rv1,
  int rv2,
  decoded_instruction_t d_i);
int compute_div_result(
  int rv1,
  int rv2,
  decoded_instruction_t d_i);
  
#endif


int compute_result(
  int rv1,
  code_address_t pc,
  decoded_instruction_t decoded_instruction);
code_address_t compute_next_pc(
  code_address_t pc,
  decoded_instruction_t decoded_instruction,
  int rv1);

#ifndef AD
#ifdef FPU
int compute_op_result(
  int rv1,
  int rv2,
  int fcsr,
  decoded_instruction_t d_i);
#else
int compute_op_result(
  int rv1,
  int right,
  decoded_instruction_t d_i);
#endif
#endif

#ifdef FPU
#ifdef AD
float_int_t compute_fp_ad_op_result(
  float_int_t                  rv1,
  float_int_t                  rv2,
  float_int_t                  rv3,
  float_int_t 				   ad_rv1[AD_DERIV_CNT],
  float_int_t 				   ad_rv2[AD_DERIV_CNT],
  float_int_t 				   ad_rv3[AD_DERIV_CNT],
  //float_int_t				   *result,
  float_int_t				   ad_result[AD_DERIV_CNT],
  decoded_instruction_t        d_i,
  bit_t                        *is_ad_op
  );
#else
float_int_t compute_fp_op_result(
  float_int_t                  rv1,
  float_int_t                  rv2,
  float_int_t                  rv3,
  decoded_instruction_t d_i);
#endif

#ifdef FLOAT_INT_T_IN_COMPUTE
bit_t compute_ad_op_result(
  float_int_t rv1,
  float_int_t rv2,
  int fcsr,
  float_int_t ad_rv1[AD_DERIV_CNT],
  float_int_t ad_rv2[AD_DERIV_CNT],
  float_int_t *result,
  float_int_t ad_result1[AD_DERIV_CNT],
  decoded_instruction_t d_i);
#endif

#ifdef AD
bit_t compute_ad_op_result(
  int rv1,
  int rv2,
  int fcsr,
  int ad_rv1[AD_DERIV_CNT],
  int ad_rv2[AD_DERIV_CNT],
  int *result,
  int ad_result1[AD_DERIV_CNT],
  decoded_instruction_t d_i);
#endif
#endif

#endif
