/**********
Copyright (c) 2018, Xilinx, Inc.
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <CL/cl2.hpp>
#include <chrono>

#include "../Compilation/runnable_ad_proc_code.h"
//#include "../../Test_RISC_V/Make/runnable_ad_proc_code.h"
//#include "../Compilation/Make/runnable_ad_proc_code.h"

#define OCL_CHECK(error, call)                                                                   \
    call;                                                                                        \
    if (error != CL_SUCCESS) {                                                                   \
        printf("%s:%d Error calling " #call ", error code is: %d\n", __FILE__, __LINE__, error); \
        exit(EXIT_FAILURE);                                                                      \
    }

#define AD
#define LOG_CODE_MEM_SIZE 16//14//17
#define LOG_DATA_MEM_SIZE 17//15//18
//size in bytes
#define CODE_MEM_SIZE     (1<<LOG_CODE_MEM_SIZE)
#define DATA_MEM_SIZE     (1<<LOG_DATA_MEM_SIZE)
#define AD_DERIV_CNT	  1
#define UNIFIED_MEMORY
#define word_type int

struct timeval tval_before, tval_after, tval_result;

unsigned int code_mem_ad [CODE_MEM_SIZE/sizeof(int)] = {
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
	  //0xfed61ae3,		//jump -12
	  //0xfed618e3, 	//statt 0xfed61ae3 jetzt 0xfed618e3 - pc rel offset -16 instead of -12
	  //0xfed616e3,		//statt 0xfed61ae3 jetzt 0xfed616e3 - pc rel offset -20 instead of -12
	  //0xfed614e3,     	//statt 0xfed61ae3 jetzt 0xfed614e3 - pc rel offset -24 instead of -12
	  //0xfcd618e3,     //statt 0xfed61ae3 jetzt 0xfcd618e3 - pc rel offset -48 instead of -12 8 Nops + 4 inst.
	  //0xfcd610e3,     //statt 0xfed61ae3 jetzt 0xfcd614e3 - pc rel offset -64 instead of -12 12 Nops + 4 inst.
	  //0xfad618e3,     //statt 0xfed61ae3 jetzt 0xfad618e3 - pc rel offset -80 instead of -12 16 Nops + 4 inst.
	  //0xf8d61ee3,     //statt 0xfed61ae3 jetzt 0xf8d61ee3 - pc rel offset -100 instead of -12 20 Nops + 5 inst.
	  //0xfad614e3,     //statt 0xfed61ae3 jetzt 0xfad614e3 - pc rel offset -88 instead of -12 24 Nops + 6 inst.
	  0x00b6700b,	  	//adlr    a1,0(a2) - a1 = a1[ad_channel=0] = a2 -> a1 stays besides one channel
	  0x00000613,		//li      a2,0
	  0x00460613,		//addi    a2,a2,4
	  0x00460613,		//addi    a2,a2,4 //a2 = 8 -> mem[2]
	  0x00f6600b,	  	//adlw    a5,0(a2) - a5 = M[a2][ad_channel=0], in a2 steht die Adresse
	  0x0106600b,	  	//adlw    a6,0(a2) - a6 = M[a2][ad_channel=0], in a2 steht die Adresse -> deletes previous ad_value in a6
	  0x0106610b,	  	//adlw    a6,0(a2) - a6 = M[a2][ad_channel=2], in a2 steht die Adresse -> deletes previous ad_value in a6
	  0x00b62023,		//sw      a1,0(a2) -> sw rs2,offset(rs1) -> M[x[rs1] + offset] = x[rs2] -> M[a3][0] = a1
	  0x0000002b, 
      0x00008067,
	  0x0000002b
};

unsigned int code_mem_pi[CODE_MEM_SIZE/sizeof(int)] = {
    /*main:0*/
    0xfe010113,/*         	addi	sp,sp,-32*/
    0x00112e23,/*         	sw	ra,28(sp)*/
    0x00c10513,/*         	addi	a0,sp,12*/
    0x010000ef,/*         	jal	ra,10160 <leibniz>*/
    0x01c12083,/*         	lw	ra,28(sp)*/
    0x02010113,/*         	addi	sp,sp,32*/
    0x00008067,/*           ret*/
    0x0000002b,				//EXIT
    /*leibniz:28*/
    0xfd010113,/*         	addi	sp,sp,-48*/
    0x02112623,/*         	sw	ra,44(sp)*/
    0x02812423,/*         	sw	s0,40(sp)*/
    0x02912223,/*         	sw	s1,36(sp)*/
    0x03212023,/*         	sw	s2,32(sp)*/
    0x01312e23,/*         	sw	s3,28(sp)*/
    0x01412c23,/*         	sw	s4,24(sp)*/
    0x01512a23,/*         	sw	s5,20(sp)*/
    0x01612823,/*         	sw	s6,16(sp)*/
    0x01712623,/*         	sw	s7,12(sp)*/
    0x01812423,/*         	sw	s8,8(sp)*/
    0x00050c13,/*         	mv	s8,a0*/
    0x3f8004b7,/*         	lui	s1,0x3f800*/
    0x00000a93,/*         	li	s5,0*/
    0x00000a13,/*         	li	s4,0*/
    0x00048413,/*         	mv	s0,s1*/
    0x40000bb7,/*         	lui	s7,0x40000*/
    0x00048b13,/*         	mv	s6,s1*/
    0xd2f1b7b7,/*         	lui	a5,0xd2f1b*/
    0x9fc78913,/*         	addi	s2,a5,-1540 # d2f1a9fc <__global_pointer$+0xd2f08864>*/
    0x3f5067b7,/*         	lui	a5,0x3f506*/
    0x24d78993,/*         	addi	s3,a5,589 # 3f50624d <__global_pointer$+0x3f4f40b5>*/
    0x0340006f,/*         	j	101ec <leibniz+0x8c>*/
    0xf0040753,/*         	fmv.w.x	fa4,s0*/
    0x00e7f7d3,/*         	fadd.s	fa5,fa5,fa4*/
    0x08e7f753,/*         	fsub.s	fa4,fa5,fa4*/
    0xe0070553,/*         	fmv.x.w	a0,fa4*/
    0xe0078453,/*         	fmv.x.w	s0,fa5*/
    0x001a4a13,/*         	xori	s4,s4,1*/
    0x001a8a93,/*         	addi	s5,s5,1*/
    0x17c000ef,/*         	jal	ra,10354 <__extendsfdf2>*/
    0x00090613,/*         	mv	a2,s2*/
    0x00098693,/*         	mv	a3,s3*/
    0x08c000ef,/*         	jal	ra,10270 <__gedf2>*/
    0x02a05e63,/*         	blez	a0,10224 <leibniz+0xc4>*/
    0xf00487d3,/*         	fmv.w.x	fa5,s1*/
    0xf00b8753,/*         	fmv.w.x	fa4,s7*/
    0x00e7f7d3,/*         	fadd.s	fa5,fa5,fa4*/
    0xe00784d3,/*         	fmv.x.w	s1,fa5*/
    0xf00b07d3,/*         	fmv.w.x	fa5,s6*/
    0xf0048753,/*         	fmv.w.x	fa4,s1*/
    0x18e7f7d3,/*         	fdiv.s	fa5,fa5,fa4*/
    0xfa0a1ae3,/*         	bnez	s4,101bc <leibniz+0x5c>*/
    0xf0040753,/*         	fmv.w.x	fa4,s0*/
    0x08f777d3,/*         	fsub.s	fa5,fa4,fa5*/
    0x08f77753,/*         	fsub.s	fa4,fa4,fa5*/
    0xe0070553,/*         	fmv.x.w	a0,fa4*/
    0xe0078453,/*         	fmv.x.w	s0,fa5*/
    0xfb1ff06f,/*         	j	101d0 <leibniz+0x70>*/
    0x015c2023,/*         	sw	s5,0(s8)*/
    0x408007b7,/*         	lui	a5,0x40800*/
    0xf00787d3,/*         	fmv.w.x	fa5,a5*/
    0xf0040753,/*         	fmv.w.x	fa4,s0*/
    0x10f777d3,/*         	fmul.s	fa5,fa4,fa5*/
    0xe0078553,/*         	fmv.x.w	a0,fa5*/
    0x00a02023,/*           sw  a0,0(zero)*/
    0x02c12083,/*         	lw	ra,44(sp)*/
    0x02812403,/*         	lw	s0,40(sp)*/
    0x02412483,/*         	lw	s1,36(sp)*/
    0x02012903,/*         	lw	s2,32(sp)*/
    0x01c12983,/*         	lw	s3,28(sp)*/
    0x01812a03,/*         	lw	s4,24(sp)*/
    0x01412a83,/*         	lw	s5,20(sp)*/
    0x01012b03,/*         	lw	s6,16(sp)*/
    0x00c12b83,/*         	lw	s7,12(sp)*/
    0x00812c03,/*         	lw	s8,8(sp)*/
    0x03010113,/*         	addi	sp,sp,48*/
    0x00008067,/*         	ret*/
    /*__gedf2:300*/
    0x00100737,/*         	lui	a4,0x100*/
    0xfff70713,/*         	addi	a4,a4,-1 # fffff <__global_pointer$+0xede67>*/
    0x0145d813,/*         	srli	a6,a1,0x14*/
    0x00b778b3,/*         	and	a7,a4,a1*/
    0x00050793,/*         	mv	a5,a0*/
    0x00050313,/*         	mv	t1,a0*/
    0x7ff87813,/*         	andi	a6,a6,2047*/
    0x01f5d513,/*         	srli	a0,a1,0x1f*/
    0x7ff00e93,/*         	li	t4,2047*/
    0x0146d593,/*         	srli	a1,a3,0x14*/
    0x00d77733,/*         	and	a4,a4,a3*/
    0x00060e13,/*         	mv	t3,a2*/
    0x7ff5f593,/*         	andi	a1,a1,2047*/
    0x01f6d693,/*         	srli	a3,a3,0x1f*/
    0x01d81a63,/*         	bne	a6,t4,102bc <__gedf2+0x4c>*/
    0x00f8eeb3,/*         	or	t4,a7,a5*/
    0x080e8a63,/*         	beqz	t4,10344 <__gedf2+0xd4>*/
    0xffe00513,/*         	li	a0,-2*/
    0x00008067,/*         	ret*/
    0x01d59663,/*         	bne	a1,t4,102c8 <__gedf2+0x58>*/
    0x00c76eb3,/*         	or	t4,a4,a2*/
    0xfe0e98e3,/*         	bnez	t4,102b4 <__gedf2+0x44>*/
    0x08081063,/*         	bnez	a6,10348 <__gedf2+0xd8>*/
    0x00f8e7b3,/*         	or	a5,a7,a5*/
    0x0017b793,/*         	seqz	a5,a5*/
    0x00059663,/*         	bnez	a1,102e0 <__gedf2+0x70>*/
    0x00c76633,/*         	or	a2,a4,a2*/
    0x04060c63,/*         	beqz	a2,10334 <__gedf2+0xc4>*/
    0x00079c63,/*         	bnez	a5,102f8 <__gedf2+0x88>*/
    0x00d51463,/*         	bne	a0,a3,102ec <__gedf2+0x7c>*/
    0x0305d063,/*         	bge	a1,a6,10308 <__gedf2+0x98>*/
    0x04050063,/*         	beqz	a0,1032c <__gedf2+0xbc>*/
    0xfff00513,/*         	li	a0,-1*/
    0x00008067,/*         	ret*/
    0xfff00513,/*         	li	a0,-1*/
    0x00068a63,/*         	beqz	a3,10310 <__gedf2+0xa0>*/
    0x00068513,/*         	mv	a0,a3*/
    0x00008067,/*         	ret*/
    0x00b85663,/*         	bge	a6,a1,10314 <__gedf2+0xa4>*/
    0xfe0502e3,/*         	beqz	a0,102f0 <__gedf2+0x80>*/
    0x00008067,/*         	ret*/
    0xfd176ce3,/*         	bltu	a4,a7,102ec <__gedf2+0x7c>*/
    0x02e89263,/*         	bne	a7,a4,1033c <__gedf2+0xcc>*/
    0xfc6e68e3,/*         	bltu	t3,t1,102ec <__gedf2+0x7c>*/
    0xffc366e3,/*         	bltu	t1,t3,1030c <__gedf2+0x9c>*/
    0x00000513,/*         	li	a0,0*/
    0x00008067,/*         	ret*/
    0x00100513,/*         	li	a0,1*/
    0x00008067,/*         	ret*/
    0xfe0798e3,/*         	bnez	a5,10324 <__gedf2+0xb4>*/
    0xfb5ff06f,/*         	j	102ec <__gedf2+0x7c>*/
    0xfce8e8e3,/*         	bltu	a7,a4,1030c <__gedf2+0x9c>*/
    0xfe5ff06f,/*         	j	10324 <__gedf2+0xb4>*/
    0xf7058ee3,/*         	beq	a1,a6,102c0 <__gedf2+0x50>*/
    0xf8059ee3,/*         	bnez	a1,102e4 <__gedf2+0x74>*/
    0x00000793,/*         	li	a5,0*/
    0xf89ff06f,/*         	j	102d8 <__gedf2+0x68>*/
    /*__extendsfdf2:528*/
    0x01755713,/*         	srli	a4,a0,0x17*/
    0x0ff77713,/*         	andi	a4,a4,255*/
    0xff010113,/*         	addi	sp,sp,-16*/
    0x00170793,/*         	addi	a5,a4,1*/
    0x00812423,/*         	sw	s0,8(sp)*/
    0x00912223,/*         	sw	s1,4(sp)*/
    0x00951413,/*         	slli	s0,a0,0x9*/
    0x00112623,/*         	sw	ra,12(sp)*/
    0x0fe7f793,/*         	andi	a5,a5,254*/
    0x00945413,/*         	srli	s0,s0,0x9*/
    0x01f55493,/*         	srli	s1,a0,0x1f*/
    0x04078263,/*         	beqz	a5,103c4 <__extendsfdf2+0x70>*/
    0x00345793,/*         	srli	a5,s0,0x3*/
    0x38070713,/*         	addi	a4,a4,896*/
    0x01d41413,/*         	slli	s0,s0,0x1d*/
    0x00c79793,/*         	slli	a5,a5,0xc*/
    0x01471713,/*         	slli	a4,a4,0x14*/
    0x00c7d793,/*         	srli	a5,a5,0xc*/
    0x01f49513,/*         	slli	a0,s1,0x1f*/
    0x00f767b3,/*         	or	a5,a4,a5*/
    0x00c12083,/*         	lw	ra,12(sp)*/
    0x00a7e733,/*         	or	a4,a5,a0*/
    0x00040513,/*         	mv	a0,s0*/
    0x00812403,/*         	lw	s0,8(sp)*/
    0x00412483,/*         	lw	s1,4(sp)*/
    0x00070593,/*         	mv	a1,a4*/
    0x01010113,/*         	addi	sp,sp,16*/
    0x00008067,/*         	ret*/
    0x04071663,/*         	bnez	a4,10410 <__extendsfdf2+0xbc>*/
    0x00000793,/*         	li	a5,0*/
    0xfc0402e3,/*         	beqz	s0,10390 <__extendsfdf2+0x3c>*/
    0x00040513,/*         	mv	a0,s0*/
    0x05c000ef,/*         	jal	ra,10430 <__clzsi2>*/
    0x00a00793,/*         	li	a5,10*/
    0x02a7c263,/*         	blt	a5,a0,10400 <__extendsfdf2+0xac>*/
    0x00b00793,/*         	li	a5,11*/
    0x40a787b3,/*         	sub	a5,a5,a0*/
    0x01550713,/*         	addi	a4,a0,21*/
    0x00f457b3,/*         	srl	a5,s0,a5*/
    0x00e41433,/*         	sll	s0,s0,a4*/
    0x38900713,/*         	li	a4,905*/
    0x40a70733,/*         	sub	a4,a4,a0*/
    0xf95ff06f,/*         	j	10390 <__extendsfdf2+0x3c>*/
    0xff550793,/*         	addi	a5,a0,-11*/
    0x00f417b3,/*         	sll	a5,s0,a5*/
    0x00000413,/*         	li	s0,0*/
    0xfe9ff06f,/*         	j	103f4 <__extendsfdf2+0xa0>*/
    0x00000793,/*         	li	a5,0*/
    0x00040a63,/*         	beqz	s0,10428 <__extendsfdf2+0xd4>*/
    0x00345793,/*         	srli	a5,s0,0x3*/
    0x00080737,/*         	lui	a4,0x80*/
    0x01d41413,/*         	slli	s0,s0,0x1d*/
    0x00e7e7b3,/*         	or	a5,a5,a4*/
    0x7ff00713,/*         	li	a4,2047*/
    0xf65ff06f,/*         	j	10390 <__extendsfdf2+0x3c>*/
    /*__clzsi2:748*/
    0x000107b7,/*         	lui	a5,0x10*/
    0x02f57a63,/*         	bgeu	a0,a5,10468 <__clzsi2+0x38>*/
    0x0ff00793,/*         	li	a5,255*/
    0x00a7b7b3,/*         	sltu	a5,a5,a0*/
    0x00379793,/*         	slli	a5,a5,0x3*/
    0x00011737,/*         	lui	a4,0x11*/
    0x02000693,/*         	li	a3,32*/
    0x40f686b3,/*         	sub	a3,a3,a5*/
    0x00f55533,/*         	srl	a0,a0,a5*/
    0x88470793,/*         	addi	a5,a4,-1916 # 10884 <__clz_tab>*/
    0x00a78533,/*         	add	a0,a5,a0*/
    0x00054503,/*         	lbu	a0,0(a0)*/
    0x40a68533,/*         	sub	a0,a3,a0*/
    0x00008067,/*         	ret*/
    0x01000737,/*         	lui	a4,0x1000*/
    0x01000793,/*         	li	a5,16*/
    0xfce56ae3,/*         	bltu	a0,a4,10444 <__clzsi2+0x14>*/
    0x01800793,/*         	li	a5,24*/
    0xfcdff06f /*         	j	10444 <__clzsi2+0x14>*/
};

#ifdef AD
char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT];
char data_mem_0[DATA_MEM_SIZE/sizeof(int)] = {0};
char data_mem_1[DATA_MEM_SIZE/sizeof(int)] = {0};
char data_mem_2[DATA_MEM_SIZE/sizeof(int)] = {0};
char data_mem_3[DATA_MEM_SIZE/sizeof(int)] = {0};
void copy_code_to_data_mem (word_type code_mem[CODE_MEM_SIZE/sizeof(int)],
                            char    *data_mem_0,
                            char    *data_mem_1,
                            char    *data_mem_2,
                            char    *data_mem_3,
                            char    ad_data_mem_0[(DATA_MEM_SIZE/sizeof(int))*AD_DERIV_CNT],
                            char    ad_data_mem_1[(DATA_MEM_SIZE/sizeof(int))*AD_DERIV_CNT],
                            char    ad_data_mem_2[(DATA_MEM_SIZE/sizeof(int))*AD_DERIV_CNT],
                            char    ad_data_mem_3[(DATA_MEM_SIZE/sizeof(int))*AD_DERIV_CNT]){
    long unsigned i, j;
    for (i = 0; i < DATA_MEM_SIZE/sizeof(int); i++){
        if(i < CODE_MEM_SIZE/sizeof(int)){
            data_mem_0[i] = code_mem[i] & 0xff;
            data_mem_1[i] = (code_mem[i] >> 8) & 0xff;
            data_mem_2[i] = (code_mem[i] >> 16) & 0xff;
            data_mem_3[i] = (code_mem[i] >> 24) & 0xff;
        }
        else{
            data_mem_0[i] = 0;
            data_mem_1[i] = 0;
            data_mem_2[i] = 0;
            data_mem_3[i] = 0;
        }
        for(j = 0; j < AD_DERIV_CNT; j++){
            ad_data_mem_0[i+j*AD_DERIV_CNT] = 0;
            ad_data_mem_1[i+j*AD_DERIV_CNT] = 0;
            ad_data_mem_2[i+j*AD_DERIV_CNT] = 0;
            ad_data_mem_3[i+j*AD_DERIV_CNT] = 0;
        }
    }
}
#else
char data_mem_0[DATA_MEM_SIZE/sizeof(int)] = {0};
char data_mem_1[DATA_MEM_SIZE/sizeof(int)] = {0};
char data_mem_2[DATA_MEM_SIZE/sizeof(int)] = {0};
char data_mem_3[DATA_MEM_SIZE/sizeof(int)] = {0};
void copy_code_to_data_mem (word_type code_mem[CODE_MEM_SIZE/sizeof(int)],
                            char    *data_mem_0,
                            char    *data_mem_1,
                            char    *data_mem_2,
                            char    *data_mem_3){
    long unsigned i, j;
    for (i = 0; i < DATA_MEM_SIZE/sizeof(int); i++){
        if(i < CODE_MEM_SIZE/sizeof(int)){
            data_mem_0[i] = code_mem[i] & 0xff;
            data_mem_1[i] = (code_mem[i] >> 8) & 0xff;
            data_mem_2[i] = (code_mem[i] >> 16) & 0xff;
            data_mem_3[i] = (code_mem[i] >> 24) & 0xff;
        }
        else{
            data_mem_0[i] = 0;
            data_mem_1[i] = 0;
            data_mem_2[i] = 0;
            data_mem_3[i] = 0;
        }
    }
}
#endif

static const int ARR_SIZE = 4096;

//#define CURRENT_HEX_CODE code_mem_ad
//#define CURRENT_HEX_CODE code_mem_pi
#define CURRENT_HEX_CODE code_mem

static const std::string error_message =
    "Error: Result mismatch:\n"
    "i = %d CPU result = %d Device result = %d\n";


// Forward declaration of utility functions included at the end of this file
std::vector<cl::Device> get_xilinx_devices();
char *read_binary_file(const std::string &xclbin_file_name, unsigned &nb);

// ------------------------------------------------------------------------------------
// Main program
// ------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    // ------------------------------------------------------------------------------------
    // Step 1: Initialize the OpenCL environment
    // ------------------------------------------------------------------------------------

    std::cout << "PHASE 1: Initialize the OpenCL environment " << std::endl;

    cl_int err;
    std::string binaryFile = (argc != 2) ? "binary_container_1.xclbin" : argv[1];
    unsigned fileBufSize;
    std::vector<cl::Device> devices = get_xilinx_devices();
    devices.resize(1);
    cl::Device device = devices[0];
    cl::Context context(device, NULL, NULL, NULL, &err);
    char *fileBuf = read_binary_file(binaryFile, fileBufSize);
    cl::Program::Binaries bins{{fileBuf, fileBufSize}};
    cl::Program program(context, devices, bins, NULL, &err);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    //cl::Kernel krnl_adrv32imf(program, "adrv32imf", &err);
    cl::Kernel krnl_adrv32imf(program, "adrv32imf_mp_ip", &err);
    //cl::Kernel krnl_adrv32imf(program, "adrv32imf_function_name", &err);
   
    std::cout << "PHASE 1: Initialize the OpenCL environment finished " << std::endl;


    // ------------------------------------------------------------------------------------
    // Step 2: Create buffers and initialize test values
    // ------------------------------------------------------------------------------------
    // Create the buffers and allocate memory

    std::cout << "PHASE 2: Create buffers and initialize test values " << std::endl;


    // Compute the size of array in bytes
    size_t code_size_in_bytes = CODE_MEM_SIZE;
    size_t code_size_in_elem = CODE_MEM_SIZE / sizeof(int);
    size_t data_size_in_bytes = DATA_MEM_SIZE/sizeof(int); //has to be divided by 4, because it is mem size in 4 channels

    OCL_CHECK(err, cl::Buffer buffer_start_pc(context, CL_MEM_READ_ONLY, sizeof(int), NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_g_code_mem(context, CL_MEM_READ_ONLY, code_size_in_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_g_data_mem_0(context, CL_MEM_READ_WRITE, data_size_in_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_g_data_mem_1(context, CL_MEM_READ_WRITE, data_size_in_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_g_data_mem_2(context, CL_MEM_READ_WRITE, data_size_in_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_g_data_mem_3(context, CL_MEM_READ_WRITE, data_size_in_bytes, NULL, &err));
#ifdef AD
    OCL_CHECK(err, cl::Buffer buffer_g_ad_data_mem_0(context, CL_MEM_READ_WRITE, data_size_in_bytes * AD_DERIV_CNT, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_g_ad_data_mem_1(context, CL_MEM_READ_WRITE, data_size_in_bytes * AD_DERIV_CNT, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_g_ad_data_mem_2(context, CL_MEM_READ_WRITE, data_size_in_bytes * AD_DERIV_CNT, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_g_ad_data_mem_3(context, CL_MEM_READ_WRITE, data_size_in_bytes * AD_DERIV_CNT, NULL, &err));
#endif
    OCL_CHECK(err, cl::Buffer buffer_nb_instruction(context, CL_MEM_WRITE_ONLY, sizeof(long int), NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_nb_cycle(context, CL_MEM_WRITE_ONLY, sizeof(long int), NULL, &err));

    // Map buffers to kernel arguments, thereby assigning them to specific device memory banks
    int narg = 0;
    //OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, 0x00000000));            //unsigned int   start_pc,
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_start_pc));       //unsigned int   start_pc,
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_code_mem));       //instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_data_mem_0));     //char           g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_data_mem_1));     //char           g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_data_mem_2));     //char           g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_data_mem_3));     //char           g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
#ifdef AD
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_ad_data_mem_0));  //char    		 g_ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_ad_data_mem_1));  //char    		 g_ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_ad_data_mem_2));  //char    		 g_ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_ad_data_mem_3));  //char    		 g_ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],   
#endif
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_nb_instruction)); //unsigned int  *nb_instruction,
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_nb_cycle));       //unsigned int  *nb_cycle

    // Map host-side buffer memory to user-space pointers
    int* ptr_start_pc;
    int* ptr_code_mem;
    char* ptr_data_mem_0;
    char* ptr_data_mem_1;
    char* ptr_data_mem_2;
    char* ptr_data_mem_3;
#ifdef AD
    char* ptr_ad_data_mem_0;
    char* ptr_ad_data_mem_1;
    char* ptr_ad_data_mem_2;    
    char* ptr_ad_data_mem_3;
#endif
    long int* ptr_nb_instruction;
    long int* ptr_nb_cycle;

    OCL_CHECK(err,ptr_start_pc = (int*)q.enqueueMapBuffer(buffer_start_pc, CL_TRUE, CL_MAP_WRITE, 0, sizeof(int), NULL, NULL, &err));
    OCL_CHECK(err,ptr_code_mem = (int*)q.enqueueMapBuffer(buffer_g_code_mem, CL_TRUE, CL_MAP_WRITE, 0, code_size_in_bytes, NULL, NULL, &err));
    OCL_CHECK(err,ptr_data_mem_0 = (char*)q.enqueueMapBuffer(buffer_g_data_mem_0, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, data_size_in_bytes, NULL, NULL, &err));
    OCL_CHECK(err,ptr_data_mem_1 = (char*)q.enqueueMapBuffer(buffer_g_data_mem_1, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, data_size_in_bytes, NULL, NULL, &err));
    OCL_CHECK(err,ptr_data_mem_2 = (char*)q.enqueueMapBuffer(buffer_g_data_mem_2, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, data_size_in_bytes, NULL, NULL, &err));
    OCL_CHECK(err,ptr_data_mem_3 = (char*)q.enqueueMapBuffer(buffer_g_data_mem_3, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, data_size_in_bytes, NULL, NULL, &err));
#ifdef AD
    OCL_CHECK(err,ptr_ad_data_mem_0 = (char*)q.enqueueMapBuffer(buffer_g_ad_data_mem_0, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, data_size_in_bytes*AD_DERIV_CNT, NULL, NULL, &err));
    OCL_CHECK(err,ptr_ad_data_mem_1 = (char*)q.enqueueMapBuffer(buffer_g_ad_data_mem_1, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, data_size_in_bytes*AD_DERIV_CNT, NULL, NULL, &err));
    OCL_CHECK(err,ptr_ad_data_mem_2 = (char*)q.enqueueMapBuffer(buffer_g_ad_data_mem_2, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, data_size_in_bytes*AD_DERIV_CNT, NULL, NULL, &err));
    OCL_CHECK(err,ptr_ad_data_mem_3 = (char*)q.enqueueMapBuffer(buffer_g_ad_data_mem_3, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, data_size_in_bytes*AD_DERIV_CNT, NULL, NULL, &err));   
#endif
    OCL_CHECK(err,ptr_nb_instruction = (long int*)q.enqueueMapBuffer(buffer_nb_instruction, CL_TRUE, CL_MAP_READ, 0, sizeof(long int), NULL, NULL, &err));
    OCL_CHECK(err,ptr_nb_cycle = (long int*)q.enqueueMapBuffer(buffer_nb_cycle, CL_TRUE, CL_MAP_READ, 0, sizeof(long int), NULL, NULL, &err));

    *ptr_start_pc = __start_pc;

    // Initialize the vectors used in the test
    /*for (int i = 0; i < ARR_SIZE; i++)
    {
        ptr_data_mem_0[i] = rand() % ARR_SIZE;
        ptr_data_mem_1[i] = rand() % ARR_SIZE;
        ptr_data_mem_2[i] = rand() % ARR_SIZE;
        ptr_data_mem_3[i] = rand() % ARR_SIZE;
        ptr_code_mem[i] = 0;
    }*/
    
    //COPY CODE
    for (unsigned long int i = 0; i < code_size_in_elem; i++) {
        ptr_code_mem[i] = CURRENT_HEX_CODE[i];
    }

    
    #ifdef AD
    #ifdef UNIFIED_MEMORY
    //THIS ALSO FILLS REST OF DATA AND AD_DATA MEMORY
    copy_code_to_data_mem(
                ptr_code_mem,
                ptr_data_mem_0,
                ptr_data_mem_1,
                ptr_data_mem_2,
                ptr_data_mem_3,
                ptr_ad_data_mem_0,
                ptr_ad_data_mem_1,
                ptr_ad_data_mem_2,
                ptr_ad_data_mem_3);
    printf("Finished copy code; next: copy code to data memory\n\r");
    #else // no UNIFIED_MEMORY
    for (unsigned long int i = 0; i < data_size_in_bytes; i++) {
        ptr_data_mem_0[i] = 0;
        ptr_data_mem_1[i] = 0;
        ptr_data_mem_2[i] = 0;
        ptr_data_mem_3[i] = 0;
    }
    
    for (unsigned long int i = 0; i < data_size_in_bytes * AD_DERIV_CNT; i++) {
        ptr_ad_data_mem_0[i] = 0;
        ptr_ad_data_mem_1[i] = 0;
        ptr_ad_data_mem_2[i] = 0;
        ptr_ad_data_mem_3[i] = 0;
    }
    #endif
    #else //no AD
    #ifdef UNIFIED_MEMORY
    //THIS ALSO FILLS REST OF DATA AND AD_DATA MEMORY
    copy_code_to_data_mem(
                ptr_code_mem,
                ptr_data_mem_0,
                ptr_data_mem_1,
                ptr_data_mem_2,
                ptr_data_mem_3);
    printf("Finished copy code; next: copy code to data memory\n\r");
    #else
    for (unsigned long int i = 0; i < data_size_in_bytes; i++) {
        ptr_data_mem_0[i] = 0;
        ptr_data_mem_1[i] = 0;
        ptr_data_mem_2[i] = 0;
        ptr_data_mem_3[i] = 0;
    }
    #endif
    #endif
    
    std::cout << "PHASE 2: Create buffers and initialize test values finished" << std::endl;

    // ------------------------------------------------------------------------------------
    // Step 3: Run the kernel
    // ------------------------------------------------------------------------------------

    std::cout << "PHASE 3: Run the kernel" << std::endl;

    // Set kernel arguments
    // Map buffers to kernel arguments, thereby assigning them to specific device memory banks
    narg = 0;
    //OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, 0x00000000));            //code_address_t start_pc,
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_start_pc));     //code_address_t start_pc,
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_code_mem));       //instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_data_mem_0));     //char           g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_data_mem_1));     //char           g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_data_mem_2));     //char           g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_data_mem_3));     //char           g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
#ifdef AD
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_ad_data_mem_0));  //char    		 g_ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_ad_data_mem_1));  //char    		 g_ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_ad_data_mem_2));  //char    		 g_ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_g_ad_data_mem_3));  //char    		 g_ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
#endif
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_nb_instruction)); //unsigned int  *nb_instruction,
    OCL_CHECK(err, err = krnl_adrv32imf.setArg(narg++, buffer_nb_cycle));       //unsigned int  *nb_cycle

    auto begin_total = std::chrono::high_resolution_clock::now();    
    // Schedule transfer of inputs to device memory, execution of kernel, and transfer of outputs back to host memory
    auto begin_there = std::chrono::high_resolution_clock::now();    
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({   buffer_start_pc,
                                                        buffer_g_code_mem, 
                                                        buffer_g_data_mem_0, 
                                                        buffer_g_data_mem_1, 
                                                        buffer_g_data_mem_2, 
                                                        buffer_g_data_mem_3,
                                                    #ifdef AD
                                                        buffer_g_ad_data_mem_0,
                                                        buffer_g_ad_data_mem_1,
                                                        buffer_g_ad_data_mem_2,
                                                        buffer_g_ad_data_mem_3
                                                    #endif
                                                        }, 
                                                        0 /* 0 means from host*/));
    auto end_there = std::chrono::high_resolution_clock::now();    

    auto begin = std::chrono::high_resolution_clock::now();    
    OCL_CHECK(err, err = q.enqueueTask(krnl_adrv32imf));
    auto end = std::chrono::high_resolution_clock::now();    


    auto begin_back = std::chrono::high_resolution_clock::now();    
    OCL_CHECK(err, q.enqueueMigrateMemObjects({ buffer_g_code_mem, 
                                                buffer_g_data_mem_0, 
                                                buffer_g_data_mem_1, 
                                                buffer_g_data_mem_2, 
                                                buffer_g_data_mem_3,
                                            #ifdef AD
                                                buffer_g_ad_data_mem_0,
                                                buffer_g_ad_data_mem_1,
                                                buffer_g_ad_data_mem_2,
                                                buffer_g_ad_data_mem_3,
                                            #endif
                                                buffer_nb_cycle, 
                                                buffer_nb_instruction
                                                }, 
                                                CL_MIGRATE_MEM_OBJECT_HOST));
    auto end_back = std::chrono::high_resolution_clock::now();    

    // Wait for all scheduled operations to finish
    OCL_CHECK(err, q.finish());
    auto end_total = std::chrono::high_resolution_clock::now();    

    
    auto dur = end - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    auto dur_there = end_there - begin_there;
    auto ms_there = std::chrono::duration_cast<std::chrono::milliseconds>(dur_there).count();
    auto dur_back = end_back - begin_back;
    auto ms_and_back_again = std::chrono::duration_cast<std::chrono::milliseconds>(dur_back).count();
    auto dur_total = end_total - begin_total;
    auto ms_total = std::chrono::duration_cast<std::chrono::milliseconds>(dur_total).count();
    
    std::cout << "PHASE 3: Create buffers and initialize test values finished" << std::endl;

    // ------------------------------------------------------------------------------------
    // Step 4: Check Results and Release Allocated Resources
    // ------------------------------------------------------------------------------------
    
    std::cout << "PHASE 4: Check Results and Release Allocated Resources" << std::endl;

    bool match = true;
    /*for (int i = 0; i < ARR_SIZE; i++)
    {
        int expected = ptr_data_mem_0[i] + ptr_data_mem_1[i] + ptr_data_mem_2[i] + ptr_data_mem_3[i];
        if (ptr_code_mem[i] != expected)
        {
            std::cout << "Error: Result mismatch" << std::endl;
            std::cout << "i = " << i << " CPU result = " << expected << " Device result = " << ptr_code_mem[i] << std::endl;
            match = false;
            break;
        }
    }*/

    printf("Code finished running\n\r");
    
    printf("\ndata memory dump (non null words)\n\r");
    unsigned char b0, b1, b2, b3;
    unsigned char ad_b0, ad_b1, ad_b2, ad_b3;
    unsigned int  ad_w, w;
    unsigned long int nbi, nbc;
    for (unsigned long int i=0; i<data_size_in_bytes; i++){
        b0 = ptr_data_mem_0[i];
        b1 = ptr_data_mem_1[i];
        b2 = ptr_data_mem_2[i];
        b3 = ptr_data_mem_3[i];
        w = ((unsigned int)b3<<24) | ((unsigned int)b2<<16) | ((unsigned int)b1<< 8) | (unsigned int)b0;
        if (w != 0)
            printf("m[%4x] = %16d (%8x) (%f)\n", 4*i, w, (unsigned int)w, *(float*)(&w));
    }
    printf("\nad data memory dump (non null words)\n\r");

#ifdef AD
    //==================ARRAY PARTITIONED IN [][AD_DERIV_CNT] Parts - 7 parts, one for each deriv??
    for (long unsigned int i=0; i<(data_size_in_bytes*AD_DERIV_CNT); i++){
        ad_b0 = ptr_ad_data_mem_0[i];
        ad_b1 = ptr_ad_data_mem_1[i];
        ad_b2 = ptr_ad_data_mem_2[i];
        ad_b3 = ptr_ad_data_mem_3[i];
        ad_w = ((unsigned char)ad_b3<<24) | ((unsigned char)ad_b2<<16) | ((unsigned char)ad_b1<<8) | ((unsigned char)ad_b0);
        if (ad_w != 0)
            printf("ad_m[%4ld][%4ld] = %16d (%8x) (%f)\n", i/AD_DERIV_CNT, i%AD_DERIV_CNT, ad_w, (unsigned int)ad_w, *(float*)(&ad_w));
    }
#endif
    nbi = *ptr_nb_instruction;
    nbc = *ptr_nb_cycle;
    printf("\n%lud fetched and decoded instructions in %lud cycles (ipc = %2.2f)\n", nbi, nbc, ((float)nbi)/nbc);

    printf("\nThis took (ms):\n");
    std::cout << ms << std::endl;

    printf("\nData transfer to device took (ms):\n");
    std::cout << ms_there << std::endl;

    printf("\nData transfer from device took (ms):\n");
    std::cout << ms_and_back_again << std::endl;

    printf("\nTotal duration (ms):\n");
    std::cout << ms_total << std::endl;

    delete[] fileBuf;
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_start_pc, ptr_start_pc));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_g_code_mem, ptr_code_mem));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_g_data_mem_0, ptr_data_mem_0));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_g_data_mem_1, ptr_data_mem_1));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_g_data_mem_2, ptr_data_mem_2));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_g_data_mem_3, ptr_data_mem_3));
#ifdef AD
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_g_ad_data_mem_0, ptr_ad_data_mem_0));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_g_ad_data_mem_1, ptr_ad_data_mem_1));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_g_ad_data_mem_2, ptr_ad_data_mem_2));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_g_ad_data_mem_3, ptr_ad_data_mem_3));
#endif
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_nb_cycle, ptr_nb_cycle));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_nb_instruction, ptr_nb_instruction));
    OCL_CHECK(err, err = q.finish());

    std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl;
    std::cout << "PHASE 4: Check Results and Release Allocated Resources finished" << std::endl;
    return (match ? EXIT_SUCCESS : EXIT_FAILURE);
}

// ------------------------------------------------------------------------------------
// Utility functions
// ------------------------------------------------------------------------------------
std::vector<cl::Device> get_xilinx_devices()
{
    size_t i;
    cl_int err;
    std::vector<cl::Platform> platforms;
    err = cl::Platform::get(&platforms);
    cl::Platform platform;
    for (i = 0; i < platforms.size(); i++)
    {
        platform = platforms[i];
        std::string platformName = platform.getInfo<CL_PLATFORM_NAME>(&err);
        if (platformName == "Xilinx")
        {
            std::cout << "INFO: Found Xilinx Platform" << std::endl;
            break;
        }
    }
    if (i == platforms.size())
    {
        std::cout << "ERROR: Failed to find Xilinx platform" << std::endl;
        exit(EXIT_FAILURE);
    }

    //Getting ACCELERATOR Devices and selecting 1st such device
    std::vector<cl::Device> devices;
    err = platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
    return devices;
}

char *read_binary_file(const std::string &xclbin_file_name, unsigned &nb)
{
    if (access(xclbin_file_name.c_str(), R_OK) != 0)
    {
        printf("ERROR: %s xclbin not available please build\n", xclbin_file_name.c_str());
        exit(EXIT_FAILURE);
    }
    //Loading XCL Bin into char buffer
    std::cout << "INFO: Loading '" << xclbin_file_name << "'\n";
    std::ifstream bin_file(xclbin_file_name.c_str(), std::ifstream::binary);
    bin_file.seekg(0, bin_file.end);
    nb = bin_file.tellg();
    bin_file.seekg(0, bin_file.beg);
    char *buf = new char[nb];
    bin_file.read(buf, nb);
    return buf;
}
