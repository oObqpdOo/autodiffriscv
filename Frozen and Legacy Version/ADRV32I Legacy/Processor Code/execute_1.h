#ifndef __EXECUTE_1
#define __EXECUTE_1

#include "ad_multicycle_pipeline_ip.h"

void init_e_1_state(e_1_state_t *e_1_state);
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
  bit_t             *e_1_state_is_full);

#endif
