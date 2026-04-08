#ifndef __EMULATE
#define __EMULATE

#include "adrv32imf_mp_ip.h"

void emulate(
  int *reg_file,
#ifdef FPU
  float *freg_file,
  int   fcsr,
#endif
  decoded_instruction_t d_i,
  code_address_t next_pc);

#endif
