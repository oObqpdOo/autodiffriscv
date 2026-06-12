#include "debug_multicycle_pipeline_ip.h"
#include "ad_multicycle_pipeline_ip.h"
#include "type.h"
#include "immediate.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include <stdio.h>
#include "disassemble.h"
#endif
#endif

static void decode_instruction(
  instruction_t instruction,
  decoded_instruction_t *decoded_instruction){
#pragma HLS INLINE
  decoded_instruction->inst_31    = (instruction >> 31);
  decoded_instruction->inst_30_25 = (instruction >> 25);
  decoded_instruction->inst_24_21 = (instruction >> 21);
  decoded_instruction->inst_20    = (instruction >> 20);
  decoded_instruction->inst_19_12 = (instruction >> 12);
  decoded_instruction->inst_11_8  = (instruction >>  8);
  decoded_instruction->inst_7     = (instruction >>  7);
  decoded_instruction->inst_6_2   = (instruction >>  2);
  decoded_instruction->opcode     = (instruction >>  2);
  decoded_instruction->inst_1_0   = (instruction      );
  decoded_instruction->rd         = (instruction >>  7);
  decoded_instruction->func3      = (instruction >> 12);
  decoded_instruction->rs1        = (instruction >> 15);
  decoded_instruction->rs2        = (instruction >> 20);
  decoded_instruction->func7      = (instruction >> 25);
  decoded_instruction->is_rs1_reg =
                  (decoded_instruction->opcode != JAL   &&
                   decoded_instruction->opcode != LUI   &&
                   decoded_instruction->opcode != AUIPC &&
                   decoded_instruction->rs1    != 0);
  decoded_instruction->is_rs2_reg =
                  (decoded_instruction->opcode != OP_IMM &&
                   decoded_instruction->opcode != LOAD   &&
                   decoded_instruction->opcode != JAL    &&
                   decoded_instruction->opcode != JALR   &&
                   decoded_instruction->opcode != LUI    &&                   
                   decoded_instruction->opcode != AUIPC  &&
                   decoded_instruction->rs2    != 0);
  decoded_instruction->is_load    =
                  (decoded_instruction->opcode == LOAD);
  decoded_instruction->is_store   =
                  (decoded_instruction->opcode == STORE);
  decoded_instruction->is_ad_store   =
                  (decoded_instruction->opcode == AD_STORE);
  decoded_instruction->is_ad_op   = false; //TODO: just assume it is false and just set in compute in case AD-ALU was active or during mem
  decoded_instruction->is_branch  =
                  (decoded_instruction->opcode == BRANCH);
  decoded_instruction->is_jal     =
                  (decoded_instruction->opcode == JAL);
  decoded_instruction->is_jalr    =
                  (decoded_instruction->opcode == JALR);
  decoded_instruction->is_ret     = (instruction == RET);
  decoded_instruction->is_exit    = (instruction == EXIT);
  decoded_instruction->is_lui     =
                  (decoded_instruction->opcode == LUI);
  decoded_instruction->is_op_imm  =
                  (decoded_instruction->opcode == OP_IMM);
  decoded_instruction->is_system  =
                  (decoded_instruction->opcode == SYSTEM);
  decoded_instruction->is_jump    =
                  (decoded_instruction->is_jal ||
                   decoded_instruction->is_jalr);
  decoded_instruction->is_ctrl    =
                  (decoded_instruction->is_branch ||
                   decoded_instruction->is_jump);
  decoded_instruction->has_no_dest=
                  (decoded_instruction->is_branch ||
                   decoded_instruction->is_store  ||
                   decoded_instruction->is_ad_store  ||
                  (decoded_instruction->rd == 0));
  decoded_instruction->opch       =
                   decoded_instruction->opcode>>3;
  decoded_instruction->opcl       =
                   decoded_instruction->opcode;
  decoded_instruction->type       =
              type(decoded_instruction->opch,
                   decoded_instruction->opcl);
  switch(decoded_instruction->type){
    case UNDEFINED_TYPE: decoded_instruction->imm = 0;
                         break;
    case R_TYPE:         decoded_instruction->imm = 0;
                         break;
    case I_TYPE:         decoded_instruction->imm =
                           i_immediate(decoded_instruction);
                         break;
    case S_TYPE:         decoded_instruction->imm =
                           s_immediate(decoded_instruction);
                         break;
    case AD_S_TYPE:      decoded_instruction->imm =
                           s_immediate(decoded_instruction);
						 #ifndef __SYNTHESIS__
						 #ifdef DEBUG_AD
						   printf("AD_DEBUG: DECODED_AD_S_TYPE\n");
						 #endif
						 #endif
                         break;
    case B_TYPE:         decoded_instruction->imm =
                           b_immediate(decoded_instruction);
                         break;
    case U_TYPE:         decoded_instruction->imm =
                           u_immediate(decoded_instruction);
                         break;
    case J_TYPE:         decoded_instruction->imm =
                           j_immediate(decoded_instruction);
                         break;
    case OTHER_TYPE:     decoded_instruction->imm = 0;
                         break;
  }
  decoded_instruction->is_r_type  =
                  (decoded_instruction->type   == R_TYPE);
}
void init_d_state(d_state_t *d_state){
  d_state->is_full = 0;
}
static void get_input(
  from_f_to_d_t d_from_f,
  d_state_t    *d_state){
#pragma HLS INLINE
  d_state->pc          = d_from_f.pc;
  d_state->instruction = d_from_f.instruction;
}
static void stage_job(
  d_state_t              d_state,
  decoded_instruction_t *decoded_instruction,
  code_address_t        *target_pc){
#pragma HLS INLINE
  decode_instruction(d_state.instruction,
                     decoded_instruction);
  *target_pc = d_state.pc +
    ((decoded_instruction->is_jal)   ?
     (code_address_t)(decoded_instruction->imm << 1) :
     (code_address_t) 4);
}
static void set_output(
  decoded_instruction_t decoded_instruction,
  code_address_t        target_pc,
  d_state_t             d_state,
  from_d_to_f_t        *d_to_f,
  from_d_to_i_t        *d_to_i){
#pragma HLS INLINE
  d_to_f->target_pc           = target_pc;
  d_to_i->pc                  = d_state.pc;
  d_to_i->decoded_instruction = decoded_instruction;
  if (decoded_instruction.has_no_dest)
    d_to_i->decoded_instruction.rd = 0;
#ifndef __SYNTHESIS__
  d_to_i->instruction         = d_state.instruction;
  d_to_i->target_pc           = target_pc;
#endif
}
void set_bits(
  bit_t  i_state_is_full,
  bit_t  d_from_f_is_valid,
  bit_t *d_state_is_full,
  bit_t *is_valid,
  bit_t *save_input,
  bit_t *d_to_i_is_valid){
#pragma HLS INLINE
  *is_valid        = (d_from_f_is_valid ||   *d_state_is_full);
  *save_input      = (d_from_f_is_valid && !(*d_state_is_full));
  *d_to_i_is_valid = !i_state_is_full && *is_valid;
  *d_state_is_full =  i_state_is_full && *is_valid;
}
void decode(
  from_f_to_d_t d_from_f,
  bit_t          i_state_is_full,
  d_state_t     *d_state,
  from_d_to_f_t *d_to_f,
  from_d_to_i_t *d_to_i,
  bit_t         *d_state_is_full){
#pragma HLS INLINE off
  bit_t                 save_input;
  bit_t                 wait_for_target_address;
  bit_t                 is_valid;
  decoded_instruction_t decoded_instruction;
  code_address_t        target_pc;
  set_bits(i_state_is_full,
           d_from_f.is_valid,
         &(d_state->is_full),
         &is_valid,
         &save_input,
         &(d_to_i->is_valid));
  if (save_input)
    get_input(d_from_f, d_state);
  if (d_to_i->is_valid){
    stage_job(*d_state, &decoded_instruction, &target_pc);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("decoded  %04d: ", (int)d_state->pc);
    disassemble(d_state->pc,
                d_state->instruction,
                decoded_instruction);
#endif
#endif
    set_output(decoded_instruction,
               target_pc,
              *d_state,
               d_to_f,
               d_to_i);
  }
  *d_state_is_full = d_state->is_full;
  wait_for_target_address       =
     d_to_i->is_valid &&
    (decoded_instruction.is_branch ||
     decoded_instruction.is_jalr);
  d_to_f->is_valid =
    d_to_i->is_valid &&
   !wait_for_target_address;
}
