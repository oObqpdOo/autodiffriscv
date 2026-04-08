#include <stdio.h>
#include "adrv32imf_mp_ip.h"
#include "immediate.h"
#include "print.h" //for SYMB_REG

#ifndef __SYNTHESIS__

void print_type(type_t type){
  switch(type){
	//In FPU case UNDEFINED_TYPE == OTHER_TYPE == 0
    case UNDEFINED_TYPE:
      printf("UNDEFINED_TYPE");
      break;
    case R_TYPE:
      printf("R_TYPE");
      break;
    case I_TYPE:
      printf("I_TYPE");
      break;
    case S_TYPE:
      printf("S_TYPE");
      break;
    case B_TYPE:
      printf("B_TYPE");
      break;
    case U_TYPE:
      printf("U_TYPE");
      break;
    case J_TYPE:
      printf("J_TYPE");
      break;
#ifdef FPU
    case R4_TYPE:
      printf("R4_TYPE");
      break;
#else
    case OTHER_TYPE:
      printf("OTHER_TYPE");
      break;
#endif
      }
}
void print_decode(decoded_instruction_t d_i){
  printf("inst_31:     %1x\n", (unsigned int)d_i.inst_31);
  printf("inst_30_25: %2x\n",  (unsigned int)d_i.inst_30_25);
  printf("inst_24_21:  %1x\n", (unsigned int)d_i.inst_24_21);
  printf("inst_20:     %1x\n", (unsigned int)d_i.inst_20);
  printf("inst_19_12: %2x\n",  (unsigned int)d_i.inst_19_12);
  printf("inst_11_8:   %1x\n", (unsigned int)d_i.inst_11_8);
  printf("inst_7:      %1x\n", (unsigned int)d_i.inst_7);
  printf("inst_6_2:   %2x\n",  (unsigned int)d_i.inst_6_2);
  printf("opcode:     %2x\n",  (unsigned int)d_i.opcode);
  printf("inst_1_0:    %1x\n", (unsigned int)d_i.inst_1_0);
  printf("rd:         %2x\n",  (unsigned int)d_i.rd);
  printf("func3:       %1x\n", (unsigned int)d_i.func3);
  printf("rs1:        %2x\n",  (unsigned int)d_i.rs1);
  printf("rs2:        %2x\n",  (unsigned int)d_i.rs2);
#ifdef FPU
  printf("rs3:        %2x\n",  (unsigned int)d_i.rs3);
#endif
  printf("func7:      %2x\n",  (unsigned int)d_i.func7);
#ifdef FPU
  printf("func5:      %2x\n",  (unsigned int)d_i.func5);
#endif
  print_type(d_i.type);
  printf("\n");
}
void print_opcode(opcode_t opcode){
  switch(opcode){
    case 0b00000: printf("LOAD");           break;
    case 0b00001: printf("LOAD-FP");        break;
    case 0b00010: printf("CUSTOM-0");       break;
    case 0b00011: printf("MISC-MEM");       break;
    case 0b00100: printf("OP-IMM");         break;
    case 0b00101: printf("AUIPC");          break;
    case 0b00110: printf("OP-IMM-32");      break;
    case 0b00111: printf("RV48-0");         break;
    case 0b01000: printf("STORE");          break;
    case 0b01001: printf("STORE-FP");       break;
    case 0b01010: printf("CUSTOM-1");       break;
    case 0b01011: printf("AMO");            break;
    case 0b01100: printf("OP");             break;
    case 0b01101: printf("LUI");            break;
    case 0b01110: printf("OP-32");          break;
    case 0b01111: printf("RV64");           break;
    case 0b10000: printf("MADD");           break;
    case 0b10001: printf("MSUB");           break;
    case 0b10010: printf("NMSUB");          break;
    case 0b10011: printf("NMADD");          break;
    case 0b10100: printf("OP-FP");          break;
    case 0b10101: printf("RESERVED-0");     break;
    case 0b10110: printf("CUSTOM-2-RV128"); break;
    case 0b10111: printf("RV48-1");         break;
    case 0b11000: printf("BRANCH");         break;
    case 0b11001: printf("JALR");           break;
    case 0b11010: printf("RESERVED-1");     break;
    case 0b11011: printf("JAL");            break;
    case 0b11100: printf("SYSTEM");         break;
    case 0b11101: printf("RESERVED-2");     break;
    case 0b11110: printf("CUSTOM-3-RV128"); break;
    case 0b11111: printf("RV80");           break;
  }
}
void print_reg_name(reg_num_t r){
#ifdef SYMB_REG
  switch(r){
    case  0: printf("zero"); break;
    case  1: printf("ra");   break;
    case  2: printf("sp");   break;
    case  3: printf("gp");   break;
    case  4: printf("tp");   break;
    case  5: printf("t0");   break;
    case  6: printf("t1");   break;
    case  7: printf("t2");   break;
    case  8: printf("s0");   break;
    case  9: printf("s1");   break;
    case 10: printf("a0");   break;
    case 11: printf("a1");   break;
    case 12: printf("a2");   break;
    case 13: printf("a3");   break;
    case 14: printf("a4");   break;
    case 15: printf("a5");   break;
    case 16: printf("a6");   break;
    case 17: printf("a7");   break;
    case 18: printf("s2");   break;
    case 19: printf("s3");   break;
    case 20: printf("s4");   break;
    case 21: printf("s5");   break;
    case 22: printf("s6");   break;
    case 23: printf("s7");   break;
    case 24: printf("s8");   break;
    case 25: printf("s9");   break;
    case 26: printf("s10");  break;
    case 27: printf("s11");  break;
    case 28: printf("t3");   break;
    case 29: printf("t4");   break;
    case 30: printf("t5");   break;
    case 31: printf("t6");   break;
  }
#else
  switch(r){
    case  0: printf("x0");  break;
    case  1: printf("x1");  break;
    case  2: printf("x2");  break;
    case  3: printf("x3");  break;
    case  4: printf("x4");  break;
    case  5: printf("x5");  break;
    case  6: printf("x6");  break;
    case  7: printf("x7");  break;
    case  8: printf("x8");  break;
    case  9: printf("x9");  break;
    case 10: printf("x10"); break;
    case 11: printf("x11"); break;
    case 12: printf("x12"); break;
    case 13: printf("x13"); break;
    case 14: printf("x14"); break;
    case 15: printf("x15"); break;
    case 16: printf("x16"); break;
    case 17: printf("x17"); break;
    case 18: printf("x18"); break;
    case 19: printf("x19"); break;
    case 20: printf("x20"); break;
    case 21: printf("x21"); break;
    case 22: printf("x22"); break;
    case 23: printf("x23"); break;
    case 24: printf("x24"); break;
    case 25: printf("x25"); break;
    case 26: printf("x26"); break;
    case 27: printf("x27"); break;
    case 28: printf("x28"); break;
    case 29: printf("x29"); break;
    case 30: printf("x30"); break;
    case 31: printf("x31"); break;
  }
#endif
}
#ifdef FPU
void print_freg_name(reg_num_t r){
#ifdef SYMB_FREG
  switch(r){
    case  0: printf("ft0"); break;
    case  1: printf("ft1");   break;
    case  2: printf("ft2");   break;
    case  3: printf("ft3");   break;
    case  4: printf("ft4");   break;
    case  5: printf("ft5");   break;
    case  6: printf("ft6");   break;
    case  7: printf("ft7");   break;
    case  8: printf("fs0");   break;
    case  9: printf("fs1");   break;
    case 10: printf("fa0");   break;
    case 11: printf("fa1");   break;
    case 12: printf("fa2");   break;
    case 13: printf("fa3");   break;
    case 14: printf("fa4");   break;
    case 15: printf("fa5");   break;
    case 16: printf("fa6");   break;
    case 17: printf("fa7");   break;
    case 18: printf("fs2");   break;
    case 19: printf("fs3");   break;
    case 20: printf("fs4");   break;
    case 21: printf("fs5");   break;
    case 22: printf("fs6");   break;
    case 23: printf("fs7");   break;
    case 24: printf("fs8");   break;
    case 25: printf("fs9");   break;
    case 26: printf("fs10");  break;
    case 27: printf("fs11");  break;
    case 28: printf("ft8");   break;
    case 29: printf("ft9");   break;
    case 30: printf("ft10");  break;
    case 31: printf("ft11");  break;
  }
#else
  switch(r){
    case  0: printf("f0");   break;
    case  1: printf("f1");   break;
    case  2: printf("f2");   break;
    case  3: printf("f3");   break;
    case  4: printf("f4");   break;
    case  5: printf("f5");   break;
    case  6: printf("f6");   break;
    case  7: printf("f7");   break;
    case  8: printf("f8");   break;
    case  9: printf("f9");   break;
    case 10: printf("f10");  break;
    case 11: printf("f11");  break;
    case 12: printf("f12");  break;
    case 13: printf("f13");  break;
    case 14: printf("f14");  break;
    case 15: printf("f15");  break;
    case 16: printf("f16");  break;
    case 17: printf("f17");  break;
    case 18: printf("f18");  break;
    case 19: printf("f19");  break;
    case 20: printf("f20");  break;
    case 21: printf("f21");  break;
    case 22: printf("f22");  break;
    case 23: printf("f23");  break;
    case 24: printf("f24");  break;
    case 25: printf("f25");  break;
    case 26: printf("f26");  break;
    case 27: printf("f27");  break;
    case 28: printf("f28");  break;
    case 29: printf("f29");  break;
    case 30: printf("f30");  break;
    case 31: printf("f31");  break;
  }
#endif
}
#endif

#ifdef FPU
void print_op(func3_t func3, func7_t func7, func5_t func5, opcode_t opcode, reg_num_t rs2){
  bit_t f7_6 = (func7)>>5;
  bit_t f7_0 = func7;
  if (opcode == OP_FP){
	switch(func5){
	  case FADD     : printf("fadd.s");
					  break;
	  case FSUB     : printf("fsub.s");
					  break;
	  case FMUL     : printf("fmul.s");
					  break;
	  case FDIV     : printf("fdiv.s");
					  break;
	  case FSQRT    : printf("fsqrt.s");
					  break;
#ifdef USE_HLS_MATH
	  case FMIN_MAX : if (func3 == MMAX) printf("fmax.s");
#else
	  case FMIN_MAX : if (func3 == MAX) printf("fmax.s");
#endif
					  else printf("fmin.s");
					  break;
	  case FMV_X_W  : if (func3 == FCLASS) printf("fclass.s") ;
					  else printf("fmv.x.w");
					  break;
	  case FMV_W_X  : printf("fmv.w.x");
					  break;
	  case FCVT_W_S : if (rs2 == 1) printf("fcvt.wu.s");
					  else printf("fcvt.w.s");
					  break;
	  case FCVT_S_W : if (rs2 == 1) printf("fcvt.s.wu");
					  else printf("fcvt.s.w");
					  break;
	  case FSGNJ    : if (func3 == JN) printf("fsgnjn.s");
					  else if (func3 == JX) printf("fsgnjx.s");
					  else printf("fsgnj.s");
					  break;
	  case FCMP     : if (func3 == FLT) printf("flt.s");
					  else if (func3 == FLE) printf("fle.s");
					  else printf("feq.s");
					  break;}
  }
  else if (opcode == MADD)  printf ("fmadd.s");
  else if (opcode == MSUB)  printf ("fmsub.s");
  else if (opcode == NMSUB) printf ("fnmsub.s");
  else if (opcode == NMADD) printf ("fnmadd.s");
  else if (opcode == SYSTEM){
	switch(func3){
	case CSRRS : printf ("csrrs");break;
	case CSRRW : printf ("csrrw");break;
	}
  }
  else{
	  switch(func3){
		  case ADD : if (f7_6) printf("sub");
					 else if (f7_0) printf("mul");
					 else printf("add");
					 break;
		  case SLL : if (f7_0) printf("mulh");
					 else printf("sll");
					 break;
		  case SLT : if (f7_0) printf("mulhsu");
					 else printf("slt");
					 break;
		  case SLTU: if (f7_0) printf ("mulhu");
					 else printf("sltu");
					 break;
		  case XOR : if (f7_0) printf("div");
					 else printf("xor");
					 break;
		  case SRL : if (f7_6) printf("sra");
					 else if (f7_0) printf("divu");
					 else printf("srl");
					 break;
		  case OR  : if (f7_0) printf("rem");
					 else printf("or");
					 break;
		  case AND : if (f7_0) printf("remu");
					 else printf("and");
					 break;
		}}
}
#else
void print_op(func3_t func3, func7_t func7){
  bit_t f7_6 = (func7)>>5;
  bit_t f7_0 = func7;
  switch(func3){
    case ADD : if (f7_6) printf("sub");
               else if (f7_0) printf("mul");
               else printf("add");
               break;
    case SLL : if (f7_0) printf("mulh");
               else printf("sll");
               break;
    case SLT : if (f7_0) printf("mulhsu");
               else printf("slt");
               break;
    case SLTU: if (f7_0) printf ("mulhu");
               else printf("sltu");
               break;
    case XOR : if (f7_0) printf("div");
               else printf("xor");
               break;
    case SRL : if (f7_6) printf("sra");
               else if (f7_0) printf("divu");
               else printf("srl");
               break;
    case OR  : if (f7_0) printf("rem");
               else printf("or");
               break;
    case AND : if (f7_0) printf("remu");
               else printf("and");
               break;
  }
}
#endif

void print_op_imm(func3_t func3, func7_t func7){
  bit_t f7_6 = func7>>5;
  switch(func3){
    case ADDI : printf("addi");
                break;
    case SLLI : printf("slli");
                break;
    case SLTI : printf("slti");
                break;
    case SLTIU: printf("sltiu");
                break;
    case XORI : printf("xori");
                break;
    case SRLI : if (f7_6) printf("srai");
                else printf("srli");
                break;
    case ORI  : printf("ori");
                break;
    case ANDI : printf("andi");
                break;
  }
}
void print_msize(func3_t func3){
  switch(func3){
    case 0: printf("b");  break;
    case 1: printf("h");  break;
    case 2: printf("w");  break;
    case 3:               break;
    case 4: printf("bu"); break;
    case 5: printf("hu"); break;
    case 6:
    case 7:               break;
  }
}
void print_branch(func3_t func3){
  switch(func3){
    case BEQ : printf("beq");  break;
    case BNE : printf("bne");  break;
    case    2:				   break;
    case    3:                 break;
    case BLT : printf("blt");  break;
    case BGE : printf("bge");  break;
    case BLTU: printf("bltu"); break;
    case BGEU: printf("bgeu"); break;
  }
}
void print_reg(
#ifdef AD
		reg_t *reg_file_s
#else
		int *reg_file
#endif
		){
  unsigned int i;
#ifdef AD
  int* reg_file = reg_file_s->reg_file;
#endif
  for (i=1; i<NB_REGISTER; i++){
    print_reg_name(i);
    printf(" ");
#ifdef SYMB_REG
    if (i!=26 && i!=27) printf(" ");
#else
    if (i<10) printf(" ");
#endif
    printf("= %16d (%8x)\n", reg_file[i],
               (unsigned int)reg_file[i]);
#ifdef AD
    for (unsigned int k=0; k<AD_DERIV_CNT; k++){
        printf("AD_");
        print_reg_name(i);
        printf("_[%d] ", k);
		#ifdef SYMB_REG
			if (i!=26 && i!=27) printf(" ");
		#else
			if (i<10) printf(" ");
		#endif
		printf("= %16d (%8x)\n", reg_file_s->ad_reg_file[i][k],
				   (unsigned int)reg_file_s->ad_reg_file[i][k]);
      }
    printf("______________\n");
#endif
  }
}


#ifdef FPU
void print_freg(
#ifdef AD
		reg_t *reg_file_s
#else
		float *freg_file
#endif
		){
  unsigned int i;
#ifdef AD
  float* freg_file = reg_file_s->freg_file;
#endif
  float_int_t u ;
  for (i=0; i<NB_REGISTER; i++){
    print_freg_name(i);
    printf(" ");
#ifdef SYMB_FREG
    if ((i==30) || (i==31) || (i==27) || (i==26)) printf("");
    else printf(" ");
#else
    if (i<10) printf("  ");
    else printf (" ");
#endif
    u.f = freg_file[i];
    printf("= %23f (%16x) \n", u.f,
    		    (unsigned int )u.i);
#ifdef AD
    for (unsigned int k=0; k<AD_DERIV_CNT; k++){
        printf("AD_");
        print_freg_name(i);
        printf("_[%d] ", k);
#ifdef SYMB_FREG
        if ((i==30) || (i==31) || (i==27) || (i==26)) printf("");
        else printf(" ");
#else
		if (i<10) printf("  ");
		else printf (" ");
#endif
		u.f = reg_file_s->ad_freg_file[i][k];
		printf("= %23f (%16x) \n", u.f,
					(unsigned int )u.i);
      }
    printf("______________\n");
#endif
  }
}
void print_fcsr(int fcsr){
  printf("fcsr");
  printf(" ");
  printf("= %16d (%23x) \n",fcsr,
              (unsigned int)fcsr);
}
#endif

#endif

