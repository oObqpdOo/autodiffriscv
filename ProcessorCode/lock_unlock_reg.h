#ifndef __UPDATE
#define __UPDATE
#include "adrv32imf_mp_ip.h"

void lock_unlock_reg(
  reg_num_t i_destination,
#ifdef FPU
  bit_t i_type,
#endif
  reg_num_t w_destination,
#ifdef FPU
  bit_t wb_type,
#endif
#ifdef AD
  reg_t    *reg_file_s
#else
  bit_t    *is_reg_computed
#endif
  );


#ifdef FPU
void lock_unlock_freg(
  reg_num_t i_destination,
  bit_t i_type,
  reg_num_t w_destination,
  bit_t wb_type,
#ifdef AD
  reg_t    *reg_file_s
#else
  bit_t    *is_freg_computed,
  bit_t	   *is_fcsr_computed
#endif
  );
#endif

#endif
