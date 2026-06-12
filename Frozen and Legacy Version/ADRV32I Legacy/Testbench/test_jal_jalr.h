#ifndef __TEST_JAL_JALR
#define __TEST_JAL_JALR

#include "ad_multicycle_pipeline_ip.h"

/*
       .globl  main
main:
       mv      t0,ra
here0:
       auipc   a0,0
here1:
       auipc   a1,0
       li      a2,0
       li      a4,0
       j       .L1
.L1:
       addi    a2,a2,1
       jal     f
       li      a3,3
       jalr    52(a1)
       jr      44(a0)
       addi    a4,a4,1
there:
       addi    a4,a4,1
       mv      ra,t0
       ret
f:
       addi    a2,a2,1
       ret
*/

instruction_t code_mem[CODE_MEM_SIZE/sizeof(unsigned int)] = {
  0x00008293,
  0x00000517,
  0x00000597,
  0x00000613,
  0x00000713,
  0x0040006f,
  0x00160613,
  0x020000ef,
  0x00300693,
  0x034580e7,
  0x02c50067,
  0x00170713,
  0x00170713,
  0x00028093,
  0x00008067,
  0x00160613,
  0x00008067
};
#endif

