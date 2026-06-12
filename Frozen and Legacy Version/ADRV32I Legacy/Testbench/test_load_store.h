#ifndef __TEST_LOAD_STORE
#define __TEST_LOAD_STORE

#include "ad_multicycle_pipeline_ip.h"

/*
       .globl  main
main:
       li      t0,1
       li      t1,2
       li      t2,-3
       li      t3,-4
       li      a0,0
       sw      t0,0(a0)
       addi    a0,a0,4
       sh      t1,0(a0)
       sh      t0,2(a0)
       addi    a0,a0,4
       sb      t3,0(a0)
       sb      t2,1(a0)
       sb      t1,2(a0)
       sb      t0,3(a0)
       lb      a1,0(a0)
       lb      a2,1(a0)
       lb      a3,2(a0)
       lb      a4,3(a0)
       lbu     a5,0(a0)
       lbu     a6,1(a0)
       lbu     a7,2(a0)
       addi    a0,a0,-4
       lh      s0,2(a0)
       lh      s1,0(a0)
       lhu     s2,4(a0)
       lhu     s3,6(a0)
       addi    a0,a0,-4
       lw      s4,8(a0)
       ret
*/

instruction_t code_mem[CODE_MEM_SIZE/sizeof(int)] = {
  0x00100293,
  0x00200313,
  0xffd00393,
  0xffc00e13,
  0x00000513,
  0x00552023,
  0x00450513,
  0x00651023,
  0x00551123,
  0x00450513,
  0x01c50023,
  0x007500a3,
  0x00650123,
  0x005501a3,
  0x00050583,
  0x00150603,
  0x00250683,
  0x00350703,
  0x00054783,
  0x00154803,
  0x00254883,
  0xffc50513,
  0x00251403,
  0x00051483,
  0x00455903,
  0x00655983,
  0xffc50513,
  0x00852a03,
  0x00008067
};
#endif

