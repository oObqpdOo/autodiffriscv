#include "debug_adrv32imf_mp_ip.h"
#include "adrv32imf_mp_ip.h"
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
#ifndef AD
  decoded_instruction->rd         = (instruction >>  7);
  decoded_instruction->func3      = (instruction >> 12);

#else
  decoded_instruction->func3      = (instruction >> 12);
  decoded_instruction->is_ad_load_move =       				  //needed for locking - if true, rs2 has to be locked to prevent RaW
 		  	  	  ((decoded_instruction->opcode == AD_STORE) && ((decoded_instruction->func3 == 0b110) || (decoded_instruction->func3 == 0b111)));
  if(decoded_instruction->is_ad_load_move)
	  decoded_instruction->rd         = (instruction >>  20); //in case of ADLR we actually have rd in the spot of rs2
  else
	  decoded_instruction->rd         = (instruction >>  7);
#endif
  decoded_instruction->rs1        = (instruction >> 15);
  decoded_instruction->rs2        = (instruction >> 20);
#ifdef FPU
  decoded_instruction->rs3        = (instruction >> 27);
#endif
  decoded_instruction->func7      = (instruction >> 25);
#ifdef FPU
  decoded_instruction->func5          = (instruction>>27);
  decoded_instruction->func5l         = (decoded_instruction->func5);
  decoded_instruction->func5h         = (decoded_instruction->func5 >> 2);
  decoded_instruction->is_system      = (decoded_instruction->opcode == SYSTEM);
  decoded_instruction->is_load_fp     = (decoded_instruction->opcode == LOAD_FP);
  decoded_instruction->is_store_fp    = (decoded_instruction->opcode == STORE_FP);
  decoded_instruction->is_madd        = (decoded_instruction->opcode == MADD);
  decoded_instruction->is_msub        = (decoded_instruction->opcode == MSUB);
  decoded_instruction->is_nmsub       = (decoded_instruction->opcode == NMSUB);
  decoded_instruction->is_nmadd       = (decoded_instruction->opcode == NMADD);
  decoded_instruction->is_op_fp       = (decoded_instruction->opcode == OP_FP);
  decoded_instruction->is_fsgnj       = (decoded_instruction->is_op_fp && (decoded_instruction->func5 == FSGNJ));
  decoded_instruction->is_fmin_max    = (decoded_instruction->is_op_fp && (decoded_instruction->func5 == FMIN_MAX));
  decoded_instruction->is_fsqrt       = (decoded_instruction->is_op_fp && (decoded_instruction->func5 == FSQRT));
  decoded_instruction->is_fcmp        = (decoded_instruction->is_op_fp && (decoded_instruction->func5 == FCMP));
  decoded_instruction->is_fcvt_w_s    = (decoded_instruction->is_op_fp && (decoded_instruction->func5 == FCVT_W_S));
  decoded_instruction->is_fcvt_s_w    = (decoded_instruction->is_op_fp && (decoded_instruction->func5 == FCVT_S_W));
  decoded_instruction->is_fmv_x_w     = (decoded_instruction->is_op_fp && (decoded_instruction->func5 == FMV_X_W));
  decoded_instruction->is_fmv_w_x     = (decoded_instruction->is_op_fp && (decoded_instruction->func5 == FMV_W_X));
  decoded_instruction->is_fused       = (decoded_instruction->is_madd    || decoded_instruction->is_msub     ||
                         decoded_instruction->is_nmsub   || decoded_instruction->is_nmadd);
  decoded_instruction->is_float       = (decoded_instruction->is_load_fp || decoded_instruction->is_store_fp ||
                         decoded_instruction->is_fused   || decoded_instruction->is_op_fp);
  decoded_instruction->is_execpt_inst = ((decoded_instruction->is_op_fp &&
                         (decoded_instruction->func5 == FADD || decoded_instruction->func5 == FSUB  ||
                          decoded_instruction->func5 == FMUL || decoded_instruction->func5 == FDIV))||
                          decoded_instruction->is_madd  || decoded_instruction->is_msub || decoded_instruction->is_nmsub ||
                          decoded_instruction->is_nmadd);
#endif
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
  decoded_instruction->is_mul     =
                  (decoded_instruction->opcode == OP)    &&
                  (decoded_instruction->func3 <  4)      &&
                  (decoded_instruction->func7 == 1);
  decoded_instruction->is_div     =
                  (decoded_instruction->opcode == OP)    &&
                  (decoded_instruction->func3 >= 4)      &&
                  (decoded_instruction->func7 == 1);
  decoded_instruction->is_load    =
                  (decoded_instruction->opcode == LOAD);
  decoded_instruction->is_store   =
                  (decoded_instruction->opcode == STORE);
#ifdef AD
  decoded_instruction->is_ad_store   =
                  (decoded_instruction->opcode == AD_STORE);
  decoded_instruction->is_ad_op   = false; //TODO: just assume it is false and just set in compute in case AD-ALU was active or during mem
#endif
  decoded_instruction->is_branch  =
                  (decoded_instruction->opcode == BRANCH);
  decoded_instruction->is_jal     =
                  (decoded_instruction->opcode == JAL);
  decoded_instruction->is_jalr    =
                  (decoded_instruction->opcode == JALR);
  decoded_instruction->is_ret     = (instruction == RET);
  decoded_instruction->is_exit    = (instruction == EXIT);
#ifndef __SYNTHESIS__
#ifdef DEBUG_EXIT
       printf("DECODE: IS_EXIT: %d\n", decoded_instruction->is_exit);
#endif
#endif
  decoded_instruction->is_lui     =
                  (decoded_instruction->opcode == LUI);
  decoded_instruction->is_op_imm  =
                  (decoded_instruction->opcode == OP_IMM);
  decoded_instruction->is_system  =
                  (decoded_instruction->opcode == SYSTEM);
  decoded_instruction->is_mem     =
                  (decoded_instruction->is_load ||
                   decoded_instruction->is_store
#ifdef FPU
				   || decoded_instruction->is_load_fp
				   || decoded_instruction->is_store_fp
#ifdef AD
				   //TODO: IS THAT A GOOD IDEA? FOR EXECUTE YES, BUT EVERYWHERE ELSE AS WELL??
				   || decoded_instruction->is_ad_store
#endif
#endif
				   );
  decoded_instruction->is_bjalr   =
                  (decoded_instruction->is_branch ||
                   decoded_instruction->is_jalr);
  decoded_instruction->is_jump    =
                  (decoded_instruction->is_jal ||
                   decoded_instruction->is_jalr);
  decoded_instruction->is_ctrl    =
                  (decoded_instruction->is_branch ||
                   decoded_instruction->is_jump);
                   
  //the following means no WRITEBACK!
  decoded_instruction->has_no_dest=
                   (
                   decoded_instruction->is_branch ||
                   decoded_instruction->is_store  ||
#ifdef AD
				   (
				   decoded_instruction->is_ad_store
				   &&
				   (!(decoded_instruction->is_ad_load_move))
				   )||
#endif
#ifdef FPU
                   decoded_instruction->is_store_fp  ||
#endif
				   //TODO: has_no_dest in case of FP-OP???
#ifndef FPU
				   (decoded_instruction->rd == 0));
#else
				   (
                    (decoded_instruction->rd == 0)     //sorgt dafür, dass WZR nicht überschrieben wird und sollte unbedingt so bleiben!
		            &&	   	   	   	   	   	   	   	   //Ausnahme bei float operationen, fr0 darf überschrieben werden
				   !(
                    decoded_instruction->is_float 
                    &&
				   !(decoded_instruction->is_fmv_x_w ||
				     decoded_instruction->is_fcmp ||
					 decoded_instruction->is_fcvt_w_s) 
                     &&
				    !decoded_instruction->is_store_fp
                    )
                    )
                    );
#endif

  decoded_instruction->opch       =
                   decoded_instruction->opcode>>3;
  decoded_instruction->opcl       =
                   decoded_instruction->opcode;
  decoded_instruction->type       =
              type(decoded_instruction->opch,
                   decoded_instruction->opcl);
  switch(decoded_instruction->type){
  	//In FPU case UNDEFINED_TYPE == OTHER_TYPE == 0
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
    case B_TYPE:         decoded_instruction->imm =
                           b_immediate(decoded_instruction);
                         break;
    case U_TYPE:         decoded_instruction->imm =
                           u_immediate(decoded_instruction);
                         break;
    case J_TYPE:         decoded_instruction->imm =
                           j_immediate(decoded_instruction);
                         break;
#ifdef FPU
    case R4_TYPE:        decoded_instruction->imm = 0;
    					 break;
#else
    case OTHER_TYPE:     decoded_instruction->imm = 0;
                         break;
#endif
#ifdef AD
    case AD_S_TYPE:      decoded_instruction->imm =
                           s_immediate(decoded_instruction);
						 #ifndef __SYNTHESIS__
						 #ifdef DEBUG_AD
						   printf("AD_DEBUG: DECODED_AD_S_TYPE\n");
						 #endif
						 #endif
                         break;
#endif
  }
  decoded_instruction->is_r_type  =
                  (decoded_instruction->type == R_TYPE);
//TODO: check if that is the only condition where we have an R3
#ifdef FPU
  decoded_instruction->is_rs3_reg =
		  	  	  (decoded_instruction->type == R4_TYPE);
#endif
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
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
  printf("RECEIVED: %04d: %08x      \n",
      (int)d_from_f.pc, d_state->instruction);
#endif
#endif

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
  if (decoded_instruction.has_no_dest){
    d_to_i->decoded_instruction.rd = 0;
  }
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG_MINIMAL
  printf("decoded_instruction.has_no_dest: %d\n", (int)decoded_instruction.has_no_dest);  
  printf("d_to_i->decoded_instruction.rd: %d\n", (int)d_to_i->decoded_instruction.rd);
#endif
#endif
#ifndef __SYNTHESIS__
  d_to_i->instruction         = d_state.instruction;
  d_to_i->target_pc           = target_pc;
#endif
}
void decode(
  from_f_to_d_t  d_from_f,
  bit_t          f_state_is_full,
  bit_t          i_state_is_full,
  d_state_t     *d_state,
  from_d_to_f_t *d_to_f,
  from_d_to_i_t *d_to_i,
  bit_t         *d_state_is_full){
#pragma HLS INLINE off
  bit_t                 save_input;
  bit_t                 is_valid;
  decoded_instruction_t decoded_instruction;
  code_address_t        target_pc;
  bit_t                 valid_or_full;
  valid_or_full = d_from_f.is_valid ||  d_state->is_full;
  save_input    = d_from_f.is_valid && !d_state->is_full;
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("\n=====DECODE======\n");
#endif
#endif
  if (save_input){
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("DECODE: get_input()\n");
#endif
#endif
    get_input(d_from_f, d_state);
  }
  stage_job(*d_state, &decoded_instruction, &target_pc);
  d_to_i->is_valid =
   (!f_state_is_full || decoded_instruction.is_bjalr ) &&
    !i_state_is_full && valid_or_full;
  /*d_to_i->is_valid = !i_state_is_full && valid_or_full;*/
#ifdef FETCH_EVERY_CYCLE
  d_to_f->is_valid =
        !f_state_is_full &&
        //!i_state_is_full &&
         valid_or_full   &&
        !decoded_instruction.is_bjalr
        //comment the next line to return to fetch every two cycles
        && decoded_instruction.is_jal;   
#endif
  if (d_to_i->is_valid){
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("DECODE: stage_job()\n");
#endif
#endif
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

#ifndef FETCH_EVERY_CYCLE
  d_to_f->is_valid =
   !f_state_is_full &&
   !i_state_is_full &&
    valid_or_full   &&
   !decoded_instruction.is_bjalr;
#endif

#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
    //printf("f_state->is_full: %d\n",(int)f_state->is_full); //always 0
    //printf("d_state_is_full: %d\n",(int)d_state_is_full); //always 0
  	//==============NEXT 4 ALL THE SAME=================================
	//printf("valid_input: %d\n",(int)valid_input);
	//printf("valid_or_full: %d\n",(int)valid_or_full);
	//printf("save_input: %d\n",(int)save_input);
	printf("d_from_f.is_valid : %d\n",(int)d_from_f.is_valid );
	printf("d_to_i.is_valid : %d\n",(int)d_to_i->is_valid );
	printf("d_to_f.is_valid : %d\n",(int)d_to_f->is_valid );
    printf("d_state->is_full: %d\n",(int)d_state->is_full);
#endif
#endif
  *d_state_is_full = valid_or_full && !d_to_i->is_valid && !d_to_f->is_valid;
  d_state->is_full = *d_state_is_full;
}
