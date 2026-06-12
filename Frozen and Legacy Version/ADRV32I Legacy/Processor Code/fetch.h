#ifndef __FETCH
#define __FETCH

#include "ad_multicycle_pipeline_ip.h"

void init_f_state(f_state_t *f_state);
void fetch(
  from_d_to_f_t   f_from_d,
  from_e_1_to_f_t f_from_e_1,
  bit_t           d_state_is_full,
  unsigned int   *code_mem,
  f_state_t      *f_state,
  from_f_to_d_t  *f_to_d);

#endif
