#include <stdio.h>
#include "ap_int.h"
#include "print.h"
#include "adrv32imf_mp_ip.h"
#include "immediate.h"

#ifndef __SYNTHESIS__

void disassemble(
  code_address_t pc,
  instruction_t i,
  decoded_instruction_t d_i){
  switch(d_i.type){
#ifdef FPU
	case R_TYPE://OP
	   if (d_i.opcode == OP_FP    &&
		   d_i.func5  != FSQRT    &&
		   d_i.func5  != FMV_X_W  &&
		   d_i.func5  != FMV_W_X  &&
		   d_i.func5  != FCVT_W_S &&
		   d_i.func5  != FCVT_S_W &&
		   d_i.func5  != FCMP){
		print_op(d_i.func3, d_i.func7, d_i.func5, d_i.opcode, d_i.rs2);
		printf(" ");
		print_freg_name(d_i.rd);
		printf(", ");
		print_freg_name(d_i.rs1);
		printf(", ");
		print_freg_name(d_i.rs2);
	   }
	   else if (d_i.opcode == OP_FP && d_i.func5  == FSQRT){
		 print_op(d_i.func3, d_i.func7, d_i.func5,d_i.opcode, d_i.rs2);
		 printf(" ");
		 print_freg_name(d_i.rd);
		 printf(", ");
		 print_freg_name(d_i.rs1);
	   }
	   else if (d_i.opcode == OP_FP &
			  (d_i.func5 == FMV_X_W || d_i.func5 == FCVT_W_S) ){
		 print_op(d_i.func3, d_i.func7, d_i.func5,d_i.opcode, d_i.rs2);
		printf(" ");
		print_reg_name(d_i.rd);
		printf(", ");
		print_freg_name(d_i.rs1);
	   }
	   else if (d_i.opcode == OP_FP &&
			  (d_i.func5 == FMV_W_X ||d_i.func5 == FCVT_S_W)){
		 print_op(d_i.func3, d_i.func7, d_i.func5,d_i.opcode, d_i.rs2);
		printf(" ");
		print_freg_name(d_i.rd);
		printf(", ");
		print_reg_name(d_i.rs1);
	   }
	   else if (d_i.opcode == OP_FP  && d_i.func5 == FCMP){
		 print_op(d_i.func3, d_i.func7, d_i.func5, d_i.opcode, d_i.rs2);
		 printf(" ");
		 print_reg_name(d_i.rd);
		 printf(", ");
		 print_freg_name(d_i.rs1);
		 printf(", ");
		 print_freg_name(d_i.rs2);
	   }
	   else if (d_i.opcode == SYSTEM && d_i.func3 == CSRRW ){
		 print_op(d_i.func3, d_i.func7, d_i.func5, d_i.opcode, d_i.rs2);
		 printf(" ");
		 print_reg_name(d_i.rd);
		 printf(", ");
		 printf("fcsr");
		 printf(", ");
		 print_reg_name(d_i.rs1);
	   }
	  else if (d_i.opcode == SYSTEM && d_i.func3 == CSRRS ){
		 print_op(d_i.func3, d_i.func7, d_i.func5, d_i.opcode, d_i.rs2);
		 printf(" ");
		 print_reg_name(d_i.rd);
		 printf(", ");
		 printf("fcsr");
		 printf(", ");
		 print_reg_name(d_i.rs1);
	   }
	   else{
		 print_op(d_i.func3, d_i.func7, d_i.func5,d_i.opcode, d_i.rs2);
		 printf(" ");
		 print_reg_name(d_i.rd);
		 printf(", ");
		 print_reg_name(d_i.rs1);
		 printf(", ");
		 print_reg_name(d_i.rs2);}
	   break;
	 case R4_TYPE://MADD
		print_op(d_i.func3, d_i.func7,  d_i.func5, d_i.opcode, d_i.rs2);
		printf(" ");
		print_freg_name(d_i.rd);
		printf(", ");
		print_freg_name(d_i.rs1);
		printf(", ");
		print_freg_name(d_i.rs2);
		printf(", ");
		print_freg_name(d_i.rs3);
		 break;
#ifdef AD
	 case AD_S_TYPE://AD_STORE
	    printf("ad_s");
	    print_msize(d_i.func3);
	    printf(" ");
	    print_reg_name(d_i.rs2);
	    printf(", ");
	    printf("%d(", (int)s_immediate(&d_i));
	    print_reg_name(d_i.rs1);
	    printf(")");
	    break;
#endif
	 case I_TYPE://JALR || OP_IMM || LOAD || SYSTEM
	   if (d_i.opcode == JALR){
		 if (d_i.rd == 0 && d_i.rs1 == RA) printf("ret");
		 else{
		   if (d_i.rd == 0){
			 printf("jr");
			 printf(" ");
		   }
		   else{
			 printf("jalr");
			 printf(" ");
			if (d_i.rd != RA){
			  print_reg_name(d_i.rd);
			   printf(", ");
			}
		   }
		   if (i_immediate(&d_i) == 0)
			 print_reg_name(d_i.rs1);
		   else{
			 printf("%d(", (int)i_immediate(&d_i));
			 print_reg_name(d_i.rs1);
			 printf(")");
		   }
		 }
	   }
	   else if (d_i.opcode == OP_IMM){
		 if (i == NOP) printf("nop");
		 else{
		   if (d_i.func3==ADDI && d_i.rs1==0)
			 printf("li");
		   else print_op_imm(d_i.func3, d_i.func7);
		   printf(" ");
		   print_reg_name(d_i.rd);
		   printf(", ");
		   if (d_i.func3!=ADDI || d_i.rs1!=0){
			 print_reg_name(d_i.rs1);
			 printf(", ");
		   }
		   if (d_i.func3!=SLLI && d_i.func3!=SRLI)
			 printf("%d",
				   (int)i_immediate(&d_i));
		   else
			 printf("%d",
				   (unsigned int)(((ap_uint<5>)d_i.inst_24_21)<<1 |
								   (ap_uint<5>)d_i.inst_20));
		 }
	   }
	   else if ((d_i.opcode == LOAD) || (d_i.opcode == LOAD_FP)){
		if (d_i.is_float){
			 printf("flw");
			 printf(" ");
			 print_freg_name(d_i.rd);
			 printf(", ");
			 printf("%d(", (int)i_immediate(&d_i));
			 print_reg_name(d_i.rs1);
			 printf(")");
			   }
		else{
		   printf("l");
		   print_msize(d_i.func3);
		   printf(" ");
		   print_reg_name(d_i.rd);
		   printf(", ");
		   printf("%d(", (int)i_immediate(&d_i));
		   print_reg_name(d_i.rs1);
		   printf(")");
		 }
	   }
	   else if (d_i.opcode == SYSTEM){
		 if ((i_immediate(&d_i)&1) == ECALL)
		   printf("ecall");
		 else
		   printf("ebreak");
		 printf(" ");
		 printf("%d", d_i.func3);
	   }
	   break;
#else
    case R_TYPE://OP
      print_op(d_i.func3, d_i.func7);
      printf(" ");
      print_reg_name(d_i.rd);
      printf(", ");
      print_reg_name(d_i.rs1);
      printf(", ");
      print_reg_name(d_i.rs2);
      break;
    case I_TYPE://JALR || OP_IMM || LOAD || SYSTEM
      if (d_i.opcode == JALR){
        if (d_i.rd == 0 && d_i.rs1 == RA) printf("ret");
        else{
          if (d_i.rd == 0){
            printf("jr");
            printf(" ");
          }
          else{
            printf("jalr");
            printf(" ");
        	if (d_i.rd != RA){
        	  print_reg_name(d_i.rd);
              printf(", ");
        	}
          }
          if (i_immediate(&d_i) == 0)
            print_reg_name(d_i.rs1);
          else{
            printf("%d(", (int)i_immediate(&d_i));
            print_reg_name(d_i.rs1);
            printf(")");
          }
        }
      }
      else if (d_i.opcode == OP_IMM){
        if (i == NOP) printf("nop");
        else{
          if (d_i.func3==ADDI && d_i.rs1==0)
            printf("li");
          else print_op_imm(d_i.func3, d_i.func7);
          printf(" ");
          print_reg_name(d_i.rd);
          printf(", ");
          if (d_i.func3!=ADDI || d_i.rs1!=0){
            print_reg_name(d_i.rs1);
            printf(", ");
          }
          if (d_i.func3!=SLLI && d_i.func3!=SRLI)
            printf("%d",
                  (int)i_immediate(&d_i));
          else
            printf("%d",
                  (unsigned int)(((ap_uint<5>)d_i.inst_24_21)<<1 |
                                  (ap_uint<5>)d_i.inst_20));
        }
      }
      else if (d_i.opcode == LOAD){
        printf("l");
        print_msize(d_i.func3);
        printf(" ");
        print_reg_name(d_i.rd);
        printf(", ");
        printf("%d(", (int)i_immediate(&d_i));
        print_reg_name(d_i.rs1);
        printf(")");
      }
      else if (d_i.opcode == SYSTEM){
        if ((i_immediate(&d_i)&1) == ECALL)
          printf("ecall");
        else
          printf("ebreak");
        printf(" ");
        printf("%d", d_i.func3);
      }
      break;
#endif
    case S_TYPE://STORE
#ifdef FPU
	if (d_i.opcode == STORE){
#endif
      printf("s");
      print_msize(d_i.func3);
      printf(" ");
      print_reg_name(d_i.rs2);
      printf(", ");
      printf("%d(", (int)s_immediate(&d_i));
      print_reg_name(d_i.rs1);
      printf(")");
#ifdef FPU
	}
	else{
	  printf("fsw");
	  printf(" ");
	  print_freg_name(d_i.rs2);
	  printf(", ");
	  printf("%d(", (int)s_immediate(&d_i));
	  print_reg_name(d_i.rs1);
	  printf(")");
	}
#endif
      break;
    case B_TYPE://BRANCH
      print_branch(d_i.func3);
      printf(" ");
      print_reg_name(d_i.rs1);
      printf(", ");
      print_reg_name(d_i.rs2);
      printf(", ");
      printf("%d", (int)pc+(((int)b_immediate(&d_i))<<1));
      break;
    case U_TYPE://LUI || AUIPC
      if (d_i.opcode == LUI) printf("lui");
      else printf("auipc");
      printf(" ");
      print_reg_name(d_i.rd);
      printf(", ");
      printf("%d", (int)u_immediate(&d_i));
      break;
    case J_TYPE://JAL
      if (d_i.rd == 0){
        printf("j");
        printf(" ");
      }
      else{
        printf("jal");
        printf(" ");
        print_reg_name(d_i.rd);
        printf(", ");
      }
      printf("%d", (int)pc+(((int)j_immediate(&d_i))<<1));
      break;
    default://UNDEFINED_TYPE, OTHER_TYPE
      printf("disassembled undefined or other type");
      break;
  }
  printf("\n");
}
#endif
