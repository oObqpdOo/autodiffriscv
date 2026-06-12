#ifndef __UPDATE
#define __UPDATE
#include "ad_multicycle_pipeline_ip.h"

void lock_unlock_reg(
  reg_num_t i_destination,
  reg_num_t w_destination,
  reg_t    *reg_file);

#endif
