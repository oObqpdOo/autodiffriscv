#ifndef __ISSUE
#define __ISSUE

#include "ad_multicycle_pipeline_ip.h"

void init_i_state(i_state_t *i_state);
void issue(
  from_d_to_i_t    i_from_d,
  bit_t            e_1_state_is_full,
  reg_num_t       *i_destination,
  reg_t           *reg_file,
  i_state_t       *i_state,
  from_i_to_e_1_t *i_to_e_1,
  bit_t           *i_state_is_full);

#endif
