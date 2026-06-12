#ifndef __TEST_MEM
#define __TEST_MEM

#include "ad_multicycle_pipeline_ip.h"

/*
       .globl  main
main:
       li      a0,0
       li      a1,0
       li      a2,0
       addi    a3,a2,40
.L1:
       addi    a1,a1,1
       sw      a1,0(a2)
       addi    a2,a2,4
       bne     a2,a3,.L1
       li      a1,0
       li      a2,0
.L2:
       lw      a4,0(a2)
       addi    a2,a2,4
       add     a0,a0,a4
       bne     a2,a3,.L2
       ret
*/

instruction_t code_mem[CODE_MEM_SIZE/sizeof(int)] = {
  0x00000513,
  0x00000593,
  0x00000613,
  0x02860693,
  0x00158593,
  0x00b62023,
  0x00460613,
  0xfed61ae3,
  0x00000593,
  0x00000613,
  0x00062703,
  0x00460613,
  0x00e50533,
  0xfed61ae3,
  0x00008067
};
#endif
