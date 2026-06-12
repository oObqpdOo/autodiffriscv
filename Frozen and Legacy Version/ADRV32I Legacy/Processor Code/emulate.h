#ifndef __EMULATE
#define __EMULATE

#include "ad_multicycle_pipeline_ip.h"

void emulate(
  reg_t *reg_file,
  decoded_instruction_t d_i,
  code_address_t next_pc);

#endif
