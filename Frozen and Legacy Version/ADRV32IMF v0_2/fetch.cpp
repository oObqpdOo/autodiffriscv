#include "debug_adrv32imf_mp_ip.h"
#include "adrv32imf_mp_ip.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include <stdio.h>
#endif
#endif

void init_f_state(f_state_t *f_state){
  f_state->is_full = 0;
}

#ifdef FETCH_EVERY_CYCLE
//TODO: This is actually doubled up in decode
//I could remove this in decode and pass it on, 
//but i have more, by far more urgent problems atm
static void decode_control(
  instruction_t      instruction,
  decoded_control_t *d_ctrl){
  opcode_t opcode;
  opcode = (instruction >> 2);
  d_ctrl->is_branch = (opcode == BRANCH);
  d_ctrl->is_jalr   = (opcode == JALR);
  d_ctrl->is_jal    = (opcode == JAL);
  //d_ctrl->is_ret    = (opcode == RET);
}
#endif

static void get_input(
#ifdef FETCH_EVERY_CYCLE
  from_f_to_f_t   f_from_f,
#endif
  from_d_to_f_t   f_from_d,
  from_e_1_to_f_t f_from_e_1,
  f_state_t      *f_state){
#pragma HLS INLINE

#ifdef FETCH_EVERY_CYCLE
  if (f_from_f.is_valid)
    f_state->pc = f_from_f.next_pc;
  else if (f_from_e_1.is_valid)
      f_state->pc = f_from_e_1.target_pc;

#else
  if (f_from_e_1.is_valid)
    f_state->pc = f_from_e_1.target_pc;
#endif
  else if (f_from_d.is_valid)
    f_state->pc = f_from_d.target_pc;
}

static void stage_job(
  f_state_t      f_state,
  unsigned int  *code_mem,
  instruction_t *instruction
#ifdef FETCH_EVERY_CYCLE
  ,decoded_control_t *d_ctrl
#endif
){
#pragma HLS INLINE
  *instruction = code_mem[f_state.pc >> 2];
#ifdef FETCH_EVER_CYCLE
  decode_control(*instruction, d_ctrl);
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
  printf("PASSED: %04d: %08x      \n",
      (int)f_state.pc, *instruction);
#endif
#endif
}

static void set_output(
  instruction_t  instruction,
  f_state_t     *f_state,
#ifdef FETCH_EVERY_CYCLE
  from_f_to_f_t *f_to_f,
#endif
  from_f_to_d_t *f_to_d){
#pragma HLS INLINE
  f_to_d->pc          = f_state->pc;
  f_to_d->instruction = instruction;
#ifdef FETCH_EVERY_CYCLE
  f_to_f->next_pc     = f_state->pc + (code_address_t) 4;
#endif
}

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
  bit_t          *f_state_is_full){
#pragma HLS INLINE off
  bit_t         save_input;
  instruction_t instruction;
  bit_t         valid_or_full;
  bit_t         valid_input;
  decoded_control_t d_ctrl;
  bit_t             is_ctrl;
  bit_t             fetch_wait;
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("\n=====FETCH======\n");
#endif
#endif
  valid_input   =
#ifdef FETCH_EVERY_CYCLE
		  f_from_f.is_valid ||
#endif
		  f_from_d.is_valid || f_from_e_1.is_valid;
  valid_or_full = valid_input || f_state->is_full; //valid_input || 0
  save_input    = valid_input && !f_state->is_full; //valid_input & 1
  if (save_input){
    get_input(
#ifdef FETCH_EVERY_CYCLE
    		f_from_f,
#endif
			f_from_d, f_from_e_1, f_state);
#ifdef FETCH_EVERY_CYCLE
    instruction = code_mem[f_state->pc >> 2];
    decode_control(instruction, &d_ctrl);
    is_ctrl = d_ctrl.is_branch || d_ctrl.is_jalr || d_ctrl.is_jal; //|| d_ctrl.is_ret;
    fetch_wait = is_ctrl && !f_from_d.is_valid && !f_from_e_1.is_valid;
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("FETCH: get_input()\n");
#endif
#endif
  //f_to_f->is_valid = valid_or_full && !(*f_state_is_full) && !(is_ctrl) && !(fetch_wait);
  //f_to_f->is_valid = 0;
  }
  f_to_d->is_valid = valid_or_full && !d_state_is_full; //&& is_ctrl; //&& !fetch_wait; //valid_or_full & 1 
  //Set f_to_f->is_valid to 0 to enable old behaviour
  //f_to_f->is_valid = valid_or_full && !(*f_state_is_full) && !(is_ctrl) && !(fetch_wait);
  //f_to_f->is_valid = 0;
//if (f_to_d->is_valid || f_to_f->is_valid){      
  if (f_to_d->is_valid){        
    stage_job(	*f_state,
    			code_mem,
				&instruction
#ifdef FETCH_EVERY_CYCLE
				,&d_ctrl
#endif
				);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("fetched  ");
    printf("%04d: %08x      \n",
      (int)f_state->pc, instruction);
#endif
#endif
    set_output(	instruction,
    			f_state,
#ifdef FETCH_EVERY_CYCLE
				f_to_f,
#endif
				f_to_d);
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("FETCH: set_output()\n");
#endif
#endif
#ifdef FETCH_EVERY_CYCLE
    f_to_f->is_valid = valid_or_full && !(*f_state_is_full) && !(is_ctrl) && !(fetch_wait);
#endif
  }
  *f_state_is_full = (	 valid_or_full &&
		  	  	  	 	 !f_to_d->is_valid
#ifdef FETCH_EVERY_CYCLE
						 && !f_to_f->is_valid
#endif
  );// || fetch_wait;
  f_state->is_full = *f_state_is_full;

#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
    printf("fetch_wait: %d\n",(int)fetch_wait); //always 0
    printf("f_state->is_full: %d\n",(int)f_state->is_full); //always 0
    //printf("f_state_is_full: %d\n",(int)*f_state_is_full); //always 0
    printf("d_state_is_full: %d\n",(int)d_state_is_full); //always 0
  	//==============NEXT 4 ALL THE SAME=================================
    printf("valid_input: %d\n",(int)valid_input);
    printf("is_ctrl: %d\n",(int)is_ctrl);
    //printf("valid_or_full: %d\n",(int)valid_or_full);
	//printf(" save_input: %d\n",(int)save_input);
	printf("f_to_d->is_valid: %d\n",(int)f_to_d->is_valid);
	printf("f_to_f->is_valid: %d\n",(int)f_to_f->is_valid);
    printf("f_from_f.is_valid : %d\n",(int)f_from_f.is_valid );
	printf("f_from_d.is_valid : %d\n",(int)f_from_d.is_valid );
	printf("f_from_e_1.is_valid: %d\n",(int) f_from_e_1.is_valid);
#endif
#endif

}
