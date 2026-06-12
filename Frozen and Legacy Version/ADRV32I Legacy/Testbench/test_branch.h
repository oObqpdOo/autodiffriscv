#ifndef __TEST_BRANCH
#define __TEST_BRANCH

#include "ad_multicycle_pipeline_ip.h"

/*
       .globl  main
main:
       li      a0,-8
       li      a1,5
       beq     a0,a1,.L1
       li      a2,1
.L1:
       bne     a0,a1,.L2
       li      a2,2
.L2:
       blt     a0,a1,.L3
       li      a3,1
.L3:
       bge     a0,a1,.L4
       li      a3,2
.L4:
       bltu    a0,a1,.L5
       li      a4,1
.L5:
       bgeu    a0,a1,.L6
       li      a4,2
.L6:
       ret
*/

instruction_t code_mem[CODE_MEM_SIZE/sizeof(unsigned int)] = {
  0xff800513,
  0x00500593,
  0x00b50463,
  0x00100613,
  0x00b51463,
  0x00200613,
  0x00b54463,
  0x00100693,
  0x00b55463,
  0x00200693,
  0x00b56463,
  0x00100713,
  0x00b57463,
  0x00200713,
  0x00008067
};
#endif

