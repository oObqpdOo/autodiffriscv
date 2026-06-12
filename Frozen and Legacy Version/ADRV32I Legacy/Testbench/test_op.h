#ifndef __TEST_OP
#define __TEST_OP

#include "ad_multicycle_pipeline_ip.h"

/*
       .globl  main
main:
       li      a0,13
       li      a4,12
       li      a1,7
       li      t0,28
       li      t6,-10
       li      s2,2022
       add     a2,a1,zero
       and     a3,a2,a0
       or      a5,a3,a4
       xor     a6,a5,t0
       sub     a6,a6,a1
       sltu    a7,a6,a0
       sll     t1,a6,t0
       slt     t2,t1,t6
       sltu    t3,t1,s2
       srl     t4,t1,t0
       sra     t5,t1,t0
       ret
*/

instruction_t code_mem[CODE_MEM_SIZE/sizeof(unsigned int)] = {
  0x00d00513,
  0x00c00713,
  0x00700593,
  0x01c00293,
  0xff600f93,
  0x7e600913,
  0x00058633,
  0x00a676b3,
  0x00e6e7b3,
  0x0057c833,
  0x40b80833,
  0x00a838b3,
  0x00581333,
  0x01f323b3,
  0x01233e33,
  0x00535eb3,
  0x40535f33,
  0x00008067
};
#endif
