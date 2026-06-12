#ifndef __TEST_OP_IMM
#define __TEST_OP_IMM

#include "ad_multicycle_pipeline_ip.h"

/*
       .globl  main
main:
       li      a1,5
       addi    a2,a1,1
       andi    a3,a2,12
       addi    a4,a3,-1
       ori     a5,a4,5
       xori    a6,a5,12
       sltiu   a7,a6,13
       sltiu   t0,a6,11
       slli    t1,a6,0x1c
       slti    t2,t1,-10
       sltiu   t3,t1,2022
       srli    t4,t1,0x1c
       srai    t5,t1,0x1c
       ret
*/

unsigned int code_mem[CODE_MEM_SIZE/sizeof(unsigned int)] = {
  0x00500593,
  0x00158613,
  0x00c67693,
  0xfff68713,
  0x00576793,
  0x00c7c813,
  0x00d83893,
  0x00b83293,
  0x01c81313,
  0xff632393,
  0x7e633e13,
  0x01c35e93,
  0x41c35f13,
  0x00008067
};
#endif
