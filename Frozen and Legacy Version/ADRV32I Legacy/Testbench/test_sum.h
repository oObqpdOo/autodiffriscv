#ifndef __TEST_SUM
#define __TEST_SUM

#include "ad_multicycle_pipeline_ip.h"

/*
       .globl  main
main:
       li      a0,0
       li      a1,0
       li      a2,10
.L1:
       addi    a1,a1,1
       add     a0,a0,a1
       bne     a1,a2,.L1
       ret
*/

instruction_t code_mem[CODE_MEM_SIZE/sizeof(unsigned int)] = {
  0x00000513,
  0x00000593,
  0x00a00613,
  0x00158593,
  0x00b50533,
  0xfec59ce3,
  0x00008067
};
#endif
