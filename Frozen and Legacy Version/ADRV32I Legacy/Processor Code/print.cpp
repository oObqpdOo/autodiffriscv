#ifndef __SYNTHESIS__
#include <stdio.h>
#include "ad_multicycle_pipeline_ip.h"
#include "immediate.h"
#include "print.h" //for SYMB_REG

void print_type(type_t type){
  switch(type){
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
    case OTHER_TYPE:
      printf("OTHER_TYPE");
      break;
    case AD_S_TYPE:
      printf("OTHER_TYPE");
      break;
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
  printf("func7:      %2x\n",  (unsigned int)d_i.func7);
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
    case 7: printf("r");  break;//AD WRITE TO REG adlr
  }
}
void print_branch(func3_t func3){
  switch(func3){
    case BEQ : printf("beq");  break;
    case BNE : printf("bne");  break;
    case    2:
    case    3:                 break;
    case BLT : printf("blt");  break;
    case BGE : printf("bge");  break;
    case BLTU: printf("bltu"); break;
    case BGEU: printf("bgeu"); break;
  }
}
void print_reg(reg_t *reg_file){
  unsigned int i, k;
  for (i=1; i<NB_REGISTER; i++){
    print_reg_name(i);
    printf(" ");
#ifdef SYMB_REG
    if (i!=26 && i!=27) printf(" ");
#else
    if (i<10) printf(" ");
#endif
    printf("= %16d (%8x) ;\n", reg_file->gp_reg_file[i],
               (unsigned int)reg_file->gp_reg_file[i]);

    for (k=0; k<AD_DERIV_CNT; k++){
        printf("AD_");
        print_reg_name(i);
        printf("_[%d] ", k);
		#ifdef SYMB_REG
			if (i!=26 && i!=27) printf(" ");
		#else
			if (i<10) printf(" ");
		#endif
		printf("= %16d (%8x)\n", reg_file->ad_reg_file[i][k],
				   (unsigned int)reg_file->ad_reg_file[i][k]);
      }
    printf("______________\n");
  }
}
#endif
