#ifndef __WB
#define __WB

#include "adrv32imf_mp_ip.h"

void write_back(
  from_e_2_to_w_t w_from_e_2,
  from_m_to_w_t   w_from_m,
#ifdef AD
  reg_t			 *reg_file_s,
#else
  int            *reg_file,
#ifdef FPU
  float			 *freg_file,
  int			 *fcsr,
#endif
#endif
  reg_num_t      *w_destination,
#ifdef FPU
  bit_t			 *wb_type,
#endif
  w_state_t      *w_state);

#endif
