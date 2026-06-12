#ifndef __EXECUTE_2
#define __EXECUTE_2

#include "adrv32imf_mp_ip.h"

void init_e_2_state(e_2_state_t *e_2_state);
void execute_2(
  from_e_1_to_e_2_t e_2_from_e_1,
  e_2_state_t      *e_2_state,
  from_e_2_to_w_t  *e_2_to_w,
  bit_t            *e_2_state_is_full);

#endif
