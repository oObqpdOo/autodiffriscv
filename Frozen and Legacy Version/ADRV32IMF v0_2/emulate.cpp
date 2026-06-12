#include <stdio.h>
#include "adrv32imf_mp_ip.h"
#include "print.h"
#include "immediate.h"

#ifndef __SYNTHESIS__
#ifdef FPU
static void emulate_op(
  int                  *reg_file,
  float                *freg_file,
  int                   fcsr,
  decoded_instruction_t d_i){
  if (d_i.rd != 0){
    if((d_i.opcode == LOAD_FP ||
        d_i.opcode == OP_FP   ||
        d_i.opcode == MADD    ||
        d_i.opcode == MSUB    ||
        d_i.opcode == NMADD   ||
        d_i.opcode == NMSUB ) &&
       (d_i.func5  != FMV_X_W && d_i.func5 != FCMP && d_i.func5 != FCVT_W_S))
    	print_freg_name(d_i.rd);
    else
        print_reg_name(d_i.rd);
#ifdef SYMB_REG
    if (d_i.rd!=26 && d_i.rd!=27)
      printf(" ");
#else
    if (d_i.rd < 10)
      printf(" ");
#endif
    printf(" = ");
    if ((d_i.opcode == LOAD   &&
        (d_i.func3  == LBU    || d_i.func3 == LHU))  ||
        (d_i.opcode == OP_IMM && d_i.func3 == SLTIU) ||
        (d_i.opcode == OP     && d_i.func3 == SLTU ))
      printf("%16d (%8x)", (unsigned int)reg_file[d_i.rd],
                           (unsigned int)reg_file[d_i.rd]);
    else if ((d_i.opcode == LOAD_FP || d_i.opcode == OP_FP  ||
              d_i.opcode == MADD    || d_i.opcode == MSUB   ||
              d_i.opcode == NMADD   || d_i.opcode == NMSUB) &&
             (d_i.func5  != FMV_X_W && d_i.func5 != FCMP && d_i.func5 != FCVT_W_S))
      printf("%23f (%8x)", freg_file[d_i.rd], *(unsigned int*)(&freg_file[d_i.rd]));
    else if (d_i.opcode == LOAD_FP &&
            (d_i.func5  == FMV_X_W  || d_i.func5 == FCMP || d_i.func5 == FCVT_W_S))
      printf("%16d (%8x)", reg_file[d_i.rd],
                           reg_file[d_i.rd]);
    else
      printf("%16d (%8x)", reg_file[d_i.rd],
                           reg_file[d_i.rd]);
  }
  else if (d_i.opcode == SYSTEM){
    if(d_i.func3 == CSRRW){
    printf("fcsr");
    printf(" ");
    printf(" = ");
    printf("%16d ", fcsr);
    }
    if (d_i.func3 == CSRRS){
    printf("fcsr");
    printf(" ");
    printf(" = ");
    printf("%16d ", reg_file[d_i.rd]);
    }
  }
}
#else
void emulate_op(
  int *reg_file,
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
      printf("%16d (%8x)", (unsigned int)reg_file[d_i.rd],
                           (unsigned int)reg_file[d_i.rd]);
    else
      printf("%16d (%8x)", reg_file[d_i.rd],
                           reg_file[d_i.rd]);
  }
}
#endif

void emulate_load(
  int *reg_file,
#ifdef FPU
  float    *freg_file,
  int      fcsr,
#endif
  decoded_instruction_t d_i){
  unsigned int address = (int)(reg_file[d_i.rs1]) +
                         (int)i_immediate(&d_i);
#ifdef FPU
  emulate_op(reg_file,freg_file, fcsr, d_i);
#else
  emulate_op(reg_file, d_i);
#endif
  printf("    (m[%8x])", address);
}

void emulate_store(
  int *reg_file,
#ifdef FPU
  float    			*freg_file,
  float_int_t       u,
#endif
  decoded_instruction_t d_i){
#ifdef FPU
  u.f                  = freg_file[d_i.rs2] ;
#endif
  unsigned int address = reg_file[d_i.rs1] +
                    (int)s_immediate(&d_i);
  printf("m[%8x]", address);
  printf(" = ");
#ifdef FPU
  if (d_i.is_float)
    printf("%14d  (%8x)", u.i, u.i);
  else
#endif
  printf("%16d (%8x)", reg_file[d_i.rs2],
                       reg_file[d_i.rs2]);
}

#ifdef AD
void emulate_ad_store(
  //reg_t *reg_file,
  int *reg_file,
  decoded_instruction_t d_i){
  //unsigned int address = reg_file->gp_reg_file[d_i.rs1];
  unsigned int address = reg_file[d_i.rs1];
    if((d_i.func3 != 0b111) && (d_i.func3 != 0b110)){
	  printf("ad_m[%8x][%8x]", address, (int)s_immediate(&d_i));
	  printf(" = ");
	  //printf("%16d (%8x)\n", reg_file->gp_reg_file[d_i.rs2],
	  //	  	  	  	   	   reg_file->gp_reg_file[d_i.rs2]);
	  printf("%16d (%8x)\n",  reg_file[d_i.rs2],
			  	  	  	  	  reg_file[d_i.rs2]);

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
	  printf("[%d]=address(%d)\n", (unsigned int)s_immediate(&d_i), (unsigned int)address);
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
	  printf("[%d]=address(%d)\n", (unsigned int)s_immediate(&d_i), (unsigned int)address);
  }
}
#endif

void emulate(
  int 	   *reg_file,
#ifdef FPU
  float    *freg_file,
  int      fcsr,
#endif
  decoded_instruction_t d_i,
  code_address_t next_pc){
#ifdef FPU
  float_int_t u;
#endif
  printf("      ");
  switch(d_i.type){
    case R_TYPE://OP
#ifdef FPU
      emulate_op(reg_file, freg_file, fcsr, d_i);
#else
      emulate_op(reg_file, d_i);
#endif
      break;
#ifdef FPU
    case R4_TYPE://OP
      emulate_op(reg_file,freg_file,  fcsr, d_i);
      break;
#ifdef AD
    case AD_S_TYPE://AD_STORE
      emulate_ad_store(reg_file, d_i);
      break;
#endif
#endif
    case I_TYPE://JALR || OP_IMM || LOAD || SYSTEM
      if (d_i.is_jalr){
        printf("pc  = %16d (%8x)", (int)next_pc,
                          (unsigned int)next_pc);
        if (d_i.rd != 0) printf("\n      ");
#ifdef FPU
        emulate_op(reg_file,freg_file, fcsr, d_i);
#else
        emulate_op(reg_file, d_i);
#endif
      }
      else if (d_i.is_op_imm)
#ifdef FPU
        emulate_op(reg_file,freg_file, fcsr, d_i);
#else
        emulate_op(reg_file, d_i);
#endif
#ifdef FPU
	else if (d_i.is_load || d_i.is_load_fp)
		emulate_load(reg_file,freg_file, fcsr, d_i);
#else
        else if (d_i.is_load)
        emulate_load(reg_file, d_i);
#endif
      break;
    case S_TYPE://STORE & STORE_FP
#ifdef FPU
        emulate_store(reg_file,freg_file, u, d_i);
#else
    	emulate_store(reg_file, d_i);
#endif
      break;
    case B_TYPE://BRANCH
      printf("pc  = %16d (%8x)", (int)next_pc,
                        (unsigned int)next_pc);
      break;
    case U_TYPE://LUI || AUIPC
#ifdef FPU
      emulate_op(reg_file,freg_file, fcsr, d_i);
#else
      emulate_op(reg_file, d_i);
#endif
      break;
    case J_TYPE://JAL
      printf("pc  = %16d (%8x)", (int)next_pc,
                        (unsigned int)next_pc);
      if (d_i.rd != 0) printf("\n      ");
#ifdef FPU
        emulate_op(reg_file,freg_file, fcsr, d_i);
#else
    	emulate_op(reg_file, d_i);
#endif
      break;
    default:
      break;
  }
  printf("\n");
}
#endif
