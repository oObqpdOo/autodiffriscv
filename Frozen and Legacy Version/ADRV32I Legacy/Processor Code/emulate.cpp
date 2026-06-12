#ifndef __SYNTHESIS__
#include <stdio.h>
#include "ad_multicycle_pipeline_ip.h"
#include "print.h"
#include "immediate.h"

void emulate_op(
  reg_t *reg_file,
  decoded_instruction_t d_i){
  if (d_i.rd != 0){
    print_reg_name(d_i.rd);
#ifdef SYMB_REG
    if (d_i.rd!=26 && d_i.rd!=27)
      printf(" ");
#else
    if (d_i.rd < 10)
      printf(" ");
#endif
    printf(" = ");
    if ((d_i.is_load                                 &&
        (d_i.func3  == LBU    || d_i.func3 == LHU))  ||
        (d_i.is_op_imm        && d_i.func3 == SLTIU) ||
        (d_i.opcode == OP     && d_i.func3 == SLTU ))
      printf("%16d (%8x)", (unsigned int)reg_file->gp_reg_file[d_i.rd],
                           (unsigned int)reg_file->gp_reg_file[d_i.rd]);
    else
      printf("%16d (%8x)", reg_file->gp_reg_file[d_i.rd],
    		  	  	  	   reg_file->gp_reg_file[d_i.rd]);
  }
}
void emulate_load(
  reg_t *reg_file,
  decoded_instruction_t d_i){
  unsigned int address = (int)(reg_file->gp_reg_file[d_i.rs1]) +
                         (int)i_immediate(&d_i);
  emulate_op(reg_file, d_i);
  printf("    (m[%8x])", address);
}
void emulate_store(
  reg_t *reg_file,
  decoded_instruction_t d_i){
  unsigned int address = reg_file->gp_reg_file[d_i.rs1] +
                    (int)s_immediate(&d_i);
  printf("m[%8x]", address);
  printf(" = ");
  printf("%16d (%8x)", reg_file->gp_reg_file[d_i.rs2],
		  	  	  	   reg_file->gp_reg_file[d_i.rs2]);
}
void emulate_ad_store(
  reg_t *reg_file,
  decoded_instruction_t d_i){
  unsigned int address = reg_file->gp_reg_file[d_i.rs1];
  if((d_i.func3 != 0b111) && (d_i.func3 != 0b110)){
	  printf("ad_m[%8x][%8x]", address, (int)s_immediate(&d_i));
	  printf(" = ");
	  printf("%16d (%8x)\n", reg_file->gp_reg_file[d_i.rs2],
		  	  	  	   	   reg_file->gp_reg_file[d_i.rs2]);
  }
  else if (d_i.func3 == 0b111){
	  //address = registerINHALT von rv1
	  printf("rs1 =");
	  print_reg_name(d_i.rs1);
	  printf(", rs2 =");
	  print_reg_name(d_i.rs2);
	  printf(", rd =");
	  print_reg_name(d_i.rs2);
	  printf("\n ad_lr, address: [%8x], channel: [%8x], dest_reg = ", address, (int)s_immediate(&d_i));
	  print_reg_name(d_i.rs2);
	  printf(" = ");
	  print_reg_name(d_i.rs2);
	  printf("[%d]=(%d)\n", (unsigned int)s_immediate(&d_i), (unsigned int)address);
  }
  else if (d_i.func3 == 0b110){
	  //address = registerINHALT von rv1
	  printf("rs1 =");
	  print_reg_name(d_i.rs1);
	  printf(", rs2 =");
	  print_reg_name(d_i.rs2);
	  printf(", rd =");
	  print_reg_name(d_i.rs2);
	  printf("\n ad_lw, address: [%8x], channel: [%8x], dest_reg = ", address, (int)s_immediate(&d_i));
	  print_reg_name(d_i.rs2);
	  printf(" = ");
	  print_reg_name(d_i.rs2);
	  printf("[%d]=(%d)\n", (unsigned int)s_immediate(&d_i), (unsigned int)address);
  }
}
void emulate(
  reg_t *reg_file,
  decoded_instruction_t d_i,
  code_address_t next_pc){
  printf("      ");
  switch(d_i.type){
    case R_TYPE://OP
      emulate_op(reg_file, d_i);
      break;
    case I_TYPE://JALR || OP_IMM || LOAD || SYSTEM
      if (d_i.is_jalr){
        printf("pc  = %16d (%8x)", (int)next_pc,
                          (unsigned int)next_pc);
        if (d_i.rd != 0) printf("\n      ");
        emulate_op(reg_file, d_i);
      }
      else if (d_i.is_op_imm)
        emulate_op(reg_file, d_i);
      else if (d_i.is_load)
        emulate_load(reg_file, d_i);
      break;
    case S_TYPE://STORE
      emulate_store(reg_file, d_i);
      break;
    case AD_S_TYPE://AD_STORE
      emulate_ad_store(reg_file, d_i);
      break;
    case B_TYPE://BRANCH
      printf("pc  = %16d (%8x)", (int)next_pc,
                        (unsigned int)next_pc);
      break;
    case U_TYPE://LUI || AUIPC
      emulate_op(reg_file, d_i);
      break;
    case J_TYPE://JAL
      printf("pc  = %16d (%8x)", (int)next_pc,
                        (unsigned int)next_pc);
      if (d_i.rd != 0) printf("\n      ");
      emulate_op(reg_file, d_i);
      break;
    default:
      break;
  }
  printf("\n");
}
#endif
