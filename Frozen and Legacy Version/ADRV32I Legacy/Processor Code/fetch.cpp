#include "debug_multicycle_pipeline_ip.h"
#include "ad_multicycle_pipeline_ip.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include <stdio.h>
#endif
#endif

void init_f_state(f_state_t *f_state){
  f_state->is_full = 0;
}
static void get_input(
  from_d_to_f_t   f_from_d,
  from_e_1_to_f_t f_from_e_1,
  f_state_t      *f_state){
#pragma HLS INLINE
  if (f_from_e_1.is_valid)
    f_state->pc = f_from_e_1.target_pc;
  else //if (f_from_d.is_valid)
    f_state->pc = f_from_d.target_pc;
}
static void stage_job(
  f_state_t      f_state,
  unsigned int  *code_mem,
  instruction_t *instruction){
#pragma HLS INLINE
  *instruction = code_mem[f_state.pc >> 2];
}
static void set_output(
  instruction_t  instruction,
  f_state_t     *f_state,
  from_f_to_d_t *f_to_d){
#pragma HLS INLINE
  f_to_d->pc          = f_state->pc;
  f_to_d->instruction = instruction;
}
void fetch(
  from_d_to_f_t   f_from_d,
  from_e_1_to_f_t f_from_e_1,
  bit_t           d_state_is_full,
  unsigned int   *code_mem,
  f_state_t      *f_state,
  from_f_to_d_t  *f_to_d){
#pragma HLS INLINE off
  bit_t         save_input;
  instruction_t instruction;
  save_input = 
    (f_from_d.is_valid    ||
     f_from_e_1.is_valid) &&
    !f_state->is_full;
  if (save_input)
    get_input(f_from_d, f_from_e_1, f_state);
  f_to_d->is_valid =
   !d_state_is_full     &&
   (f_from_d.is_valid   ||
    f_from_e_1.is_valid ||
    f_state->is_full);
  f_state->is_full =
    d_state_is_full     &&
   (f_from_d.is_valid   ||
    f_from_e_1.is_valid ||
    f_state->is_full);
  if (f_to_d->is_valid){
    stage_job(*f_state, code_mem, &instruction);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("fetched  ");
    printf("%04d: %08x      \n",
      (int)f_state->pc, instruction);
#endif
#endif
    set_output(instruction, f_state, f_to_d);
  }
}
