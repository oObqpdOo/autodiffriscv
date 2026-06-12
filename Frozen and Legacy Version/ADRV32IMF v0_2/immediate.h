#ifndef __IMMEDIATE
#define __IMMEDIATE

#include "adrv32imf_mp_ip.h"

i_immediate_t i_immediate(
  decoded_instruction_t *decoded_instruction);
s_immediate_t s_immediate(
  decoded_instruction_t *decoded_instruction);
b_immediate_t b_immediate(
  decoded_instruction_t *decoded_instruction);
u_immediate_t u_immediate(
  decoded_instruction_t *decoded_instruction);
j_immediate_t j_immediate(
  decoded_instruction_t *decoded_instruction);

#endif
