#ifndef __WB
#define __WB

#include "ad_multicycle_pipeline_ip.h"

void write_back(
  from_e_2_to_w_t w_from_e_2,
  from_m_to_w_t   w_from_m,
  reg_t          *reg_file,
  reg_num_t      *w_destination,
  w_state_t      *w_state);

#endif
