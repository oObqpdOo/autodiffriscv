#ifndef __FETCH
#define __FETCH

#include "adrv32imf_mp_ip.h"

void init_f_state(f_state_t *f_state);
void fetch(
#ifdef FETCH_EVERY_CYCLE
  from_f_to_f_t   f_from_f,
#endif
  from_d_to_f_t   f_from_d,
  from_e_1_to_f_t f_from_e_1,
  bit_t           d_state_is_full,
  unsigned int   *code_mem,
  f_state_t      *f_state,
#ifdef FETCH_EVERY_CYCLE
  from_f_to_f_t  *f_to_f,
#endif
  from_f_to_d_t  *f_to_d,
  bit_t          *f_state_is_full);

#endif
