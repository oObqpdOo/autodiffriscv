#ifndef __TEST_MEM
#define __TEST_MEM

#include "ad_multicycle_pipeline_ip.h"

/*
       .globl  main
main:
       li      a0,0
       li      a1,0
       li      a2,0
       addi    a1,a1,1
       nop
       nop
       nop
       nop
	   adsw    a1,0(a2)
	   nop
       nop
       nop
       nop
	   adsw    a1,0(a2)
	   nop
       nop
       nop
       nop
       lw      a4,0(a2)
       nop
       nop
       nop
       nop
       adlr    a1,0(a2)
       nop
       nop
       nop
       nop
       addi    a3,a2,40
.L1:
       addi    a1,a1,1
       nop
       nop
       nop
       nop
       sw      a1,0(a2)
       nop
       nop
       nop
       nop
       adlr    a1,0(a2)
	   nop
       nop
       nop
       nop
       adsw    a1,0(a2)
	   nop
       nop
       nop
       nop
	   adsw    a1,2(a2)
       nop
       nop
       nop
       nop
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


instruction_t eeeeehm_code_mem[CODE_MEM_SIZE/sizeof(int)] = {
	  0x00000513,       //li      a0,0
	  0x00000593,		//li      a1,0
	  0x00000613,		//li      a2,0
	  0x00158593,		//addi  a1,a1,1
	  /*0x00000013,		//nop
	  0x00000013,
	  0x00000013,
	  0x00000013,*/
	  0x00b6200b,		//=adsw a1,0(a2) no offset store M[a2][0] = a1
	  /*0x00000013,		//nop
	  0x00000013,
	  0x00000013,
	  0x00000013,*/
	  /*0x00b6208b,		//=adsw    a1,1(a2) - offset store M[a2][ad_channel=1] = a1 -> would be channel 1
	  0x00000013,		//4 * nop
	  0x00000013,
	  0x00000013,
	  0x00000013,*/
	  0x00062703,		//=lw a4,0(a2) load a4 = M[a2]
	  /*0x00000013,		//nop
	  0x00000013,
	  0x00000013,
	  0x00000013,*/
	  0x00b6700b,	  	//=adlr    a1,0(a2) - a1 = a1[ad_channel=0] = a2 -> a1 stays besides one channel
	  /*0x00000013,		//4 * nop
	  0x00000013,
	  0x00000013,
	  0x00000013,*/
	  /*0x02860693,		//JUMP HERE
	  0x00158593,		//addi  a1,a1,1
	  0x00000013,		//nop
	  0x00000013,
	  0x00000013,
	  0x00000013,
	  0x00b62023,		//sw    a1,0(a2) -> sw rs2,offset(rs1) -> M[x[rs1] + offset] = x[rs2]
	  0x00000013,		//4 * nop
	  0x00000013,
	  0x00000013,
	  0x00000013,
	  0x00b6700b,	  	//=adlr    a1,0(a2) - a1 = a1[ad_channel=0] = a2 -> a1 stays besides one channel
	  0x00000013,		//4 * nop
	  0x00000013,
	  0x00000013,
	  0x00000013,
	  /*0x00b6200b,		//=adsw    a1,0(a2) - no offset store M[a2][0] = a1
	  0x00000013,		//4 * nop
	  0x00000013,
	  0x00000013,
	  0x00000013,*/
	  /*0x00b6210b,		//=adsw    a1,2(a2) - offset store M[a2][ad_channel=2] = a1 -> would be channel 2
	  0x00000013,		//4 * nop
	  0x00000013,
	  0x00000013,
	  0x00000013,
	  0x00460613,		//addi    a2,a2,4
	  0x00000013,		//4 * nop
	  0x00000013,
	  0x00000013,
	  0x00000013,*/
	  //0xfed61ae3,		//jump -12
	  //0xfed618e3, 	//statt 0xfed61ae3 jetzt 0xfed618e3 - pc rel offset -16 instead of -12
	  //0xfed616e3,		//statt 0xfed61ae3 jetzt 0xfed616e3 - pc rel offset -20 instead of -12
	  //0xfed614e3,     //statt 0xfed61ae3 jetzt 0xfed614e3 - pc rel offset -24 instead of -12
	  //0xfcd618e3,     //statt 0xfed61ae3 jetzt 0xfcd618e3 - pc rel offset -48 instead of -12 8 Nops + 4 inst.
	  //0xfcd610e3,     //statt 0xfed61ae3 jetzt 0xfcd614e3 - pc rel offset -64 instead of -12 12 Nops + 4 inst.
	  //0xfad618e3,     //statt 0xfed61ae3 jetzt 0xfad618e3 - pc rel offset -80 instead of -12 16 Nops + 4 inst.
	 //0xf8d61ee3,     //statt 0xfed61ae3 jetzt 0xf8d61ee3 - pc rel offset -100 instead of -12 20 Nops + 5 inst.
	  //0xfad614e3,     //statt 0xfed61ae3 jetzt 0xfad614e3 - pc rel offset -88 instead of -12 24 Nops + 6 inst.
	  //0x00000593,
	  //0x00000613,
	  //0x00062703,
	  //0x00460613,
	  //0x00e50533,
	  //0xfed61ae3,
	  0x00008067
};


instruction_t code_mem_4d[CODE_MEM_SIZE/sizeof(int)] = {
	  0x00000513,       //li      a0,0
	  0x00000593,		//li      a1,0
	  0x00000613,		//li      a2,0
	  0x00158593,		//addi    a1,a1,1
	  0x00b6200b,		//adsw 	  a1,0(a2) no offset store M[a2][0] = a1
	  0x00062703,		//lw 	  a4,0(a2) load a4 = M[a2]
	  0x00062803,		//lw 	  a6,0(a2) load a6 = M[a2]		-> a6 gets ad value
	  0x00460613,		//addi    a2,a2,4
	  0x00b6700b,	  	//adlr    a1,0(a2) - a1 = a1[ad_channel=0] = a2 -> a1 stays besides one channel
	  0x02860693,		//JUMP    addi    a3,a2,40
	  0x00158593,		//addi    a1,a1,1
	  0x00b62023,		//sw      a1,0(a2) -> sw rs2,offset(rs1) -> M[x[rs1] + offset] = x[rs2]
	  0x00b6200b,		//adsw    a1,0(a2) - no offset store M[a2][0] = a1
	  0x00158593,		//addi    a1,a1,1
	  0x00b6210b,		//adsw    a1,2(a2) - offset store M[a2][ad_channel=2] = a1 -> would be channel 2
	  0x00b6700b,	  	//adlr    a1,0(a2) - a1 = a1[ad_channel=0] = a2 -> a1 stays besides one channel
	  0x00460613,		//addi    a2,a2,4
	  0xfed612e3,       //JUMP 	  pc rel offset -28 instead of -12, 7 instr
	  0x00b6700b,	  	//adlr    a1,0(a2) - a1 = a1[ad_channel=0] = a2 -> a1 stays besides one channel
	  0x00000613,		//li      a2,0
	  0x00460613,		//addi    a2,a2,4
	  0x00460613,		//addi    a2,a2,4 //a2 = 8 -> mem[2]
	  0x00f6600b,	  	//adlw    a5,0(a2) - a5 = M[a2][ad_channel=0], in a2 steht die Adresse
	  0x0106610b,	  	//adlw    a6,0(a2) - a6 = M[a2][ad_channel=2], in a2 steht die Adresse -> deletes previous ad_value in a6
	  0x00008067
};

instruction_t code_mem[CODE_MEM_SIZE/sizeof(int)] = {
	  0x00000513,       //li      a0,0
	  0x00000593,		//li      a1,0
	  0x00000613,		//li      a2,0
	  0x00158593,		//addi    a1,a1,1
	  0x00b6200b,		//adsw 	  a1,0(a2) no offset store M[a2][0] = a1
	  0x00062703,		//lw 	  a4,0(a2) load a4 = M[a2]
	  0x00062803,		//lw 	  a6,0(a2) load a6 = M[a2]		-> a6 gets ad value
	  0x00460613,		//addi    a2,a2,4
	  0x00b6700b,	  	//adlr    a1,0(a2) - a1 = a1[ad_channel=0] = a2 -> a1 stays besides one channel
	  0x02860693,		//JUMP    addi    a3,a2,40
	  0x00158593,		//addi    a1,a1,1
	  0x00b62023,		//sw      a1,0(a2) -> sw rs2,offset(rs1) -> M[x[rs1] + offset] = x[rs2]
	  0x00b6200b,		//adsw    a1,0(a2) - no offset store M[a2][0] = a1
	  0x00158593,		//addi    a1,a1,1
	  0x00b6210b,		//adsw    a1,2(a2) - offset store M[a2][ad_channel=2] = a1 -> would be channel 2
	  0x00b6700b,	  	//adlr    a1,0(a2) - a1 = a1[ad_channel=0] = a2 -> a1 stays besides one channel
	  0x00460613,		//addi    a2,a2,4
	  0xfed612e3,       //JUMP 	  pc rel offset -28 instead of -12, 7 instr
	  0x00b6700b,	  	//adlr    a1,0(a2) - a1 = a1[ad_channel=0] = a2 -> a1 stays besides one channel
	  0x00000613,		//li      a2,0
	  0x00460613,		//addi    a2,a2,4
	  0x00460613,		//addi    a2,a2,4 //a2 = 8 -> mem[2]
	  0x00f6600b,	  	//adlw    a5,0(a2) - a5 = M[a2][ad_channel=0], in a2 steht die Adresse
	  0x0106600b,	  	//adlw    a6,0(a2) - a6 = M[a2][ad_channel=0], in a2 steht die Adresse -> deletes previous ad_value in a6
	  0x0106610b,	  	//adlw    a6,0(a2) - a6 = M[a2][ad_channel=2], in a2 steht die Adresse -> deletes previous ad_value in a6
	  0x00b62023,		//sw      a1,0(a2) -> sw rs2,offset(rs1) -> M[x[rs1] + offset] = x[rs2] -> M[a3][0] = a1
	  //0x00008067,
	  0x0000002b
};
#endif
