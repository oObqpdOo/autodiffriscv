#ifndef __TEST_LUI_AUIPC
#define __TEST_LUI_AUIPC

#include "ad_multicycle_pipeline_ip.h"

/*
       .globl  main
main:
       lui     a1,0x1
       auipc   a2,0x1
       sub     a2,a2,a1
       addi    a2,a2,20
       jr      a2
       li      a1,3
.L1:
       li      a3,100
       ret
*/

instruction_t code_mem[CODE_MEM_SIZE/sizeof(unsigned int)] = {
  0x000015b7,
  0x00001617,
  0x40b60633,
  0x01460613,
  0x00060067,
  0x00300593,
  0x06400693,
  0x00008067
};
#endif
