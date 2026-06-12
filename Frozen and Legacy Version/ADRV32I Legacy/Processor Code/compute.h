#ifndef __COMPUTE
#define __COMPUTE

#include "ad_multicycle_pipeline_ip.h"

bit_t compute_branch_result(
  int rv1,
  int rv2,
  func3_t func3);

int compute_op_result(
  int rv1,
  int rv2,
  decoded_instruction_t d_i);

bit_t compute_ad_op_result(
  int rv1,
  int rv2,
  int ad_rv1[AD_DERIV_CNT],
  int ad_rv2[AD_DERIV_CNT],
  int *result,
  int ad_result1[AD_DERIV_CNT],
  decoded_instruction_t d_i);

//int rv1 already gets the input value
int compute_result(
  int rv1,
  //int ad_rv1[AD_DERIV_CNT],
  code_address_t pc,
  //int ad_result2[AD_DERIV_CNT],
  decoded_instruction_t decoded_instruction);


code_address_t compute_next_pc(
  code_address_t pc,
  decoded_instruction_t decoded_instruction,
  int rv1);

#endif
