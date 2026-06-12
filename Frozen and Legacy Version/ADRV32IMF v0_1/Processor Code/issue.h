#ifndef __ISSUE
#define __ISSUE

#include "adrv32imf_mp_ip.h"

void init_i_state(i_state_t *i_state);
void issue(
  from_d_to_i_t    i_from_d,
  bit_t            e_1_state_is_full,
  reg_num_t       *i_destination,
#ifdef FPU
  bit_t			  *i_type,
#endif
#ifdef AD
  reg_t			  *reg_file_s,
#else
  int             *reg_file,
#ifdef FPU
  float 		  *freg_file,
  int			  *fcsr,
#endif
  bit_t           *is_reg_computed,
#ifdef FPU
  bit_t           *is_freg_computed,
  bit_t			  *is_fcsr_computed,
#endif
#endif
  i_state_t       *i_state,
  from_i_to_e_1_t *i_to_e_1,
  bit_t           *i_state_is_full);

#endif
