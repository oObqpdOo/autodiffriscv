/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xad_multicycle_pipeline_ip.h"
#include "xparameters.h"


#define LOG_DATA_MEM_SIZE 11
//size in bytes
#define DATA_MEM_SIZE     (1<<LOG_DATA_MEM_SIZE)
#define AD_DERIV_CNT	   3//7

XAd_multicycle_pipeline_ip_Config *cfg_ptr;
XAd_multicycle_pipeline_ip         ip;

word_type code_mem[27] = {
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
		  0x00008067
};

int ad_main()
{
  char b0, b1, b2, b3;
  char ad_b0, ad_b1, ad_b2, ad_b3;
  int  ad_w, w, nbi, nbc;
  cfg_ptr = XAd_multicycle_pipeline_ip_LookupConfig(XPAR_XAD_MULTICYCLE_PIPELINE_IP_0_DEVICE_ID);
  XAd_multicycle_pipeline_ip_CfgInitialize(&ip, cfg_ptr);
  XAd_multicycle_pipeline_ip_Set_start_pc(&ip, 0);
  XAd_multicycle_pipeline_ip_Write_code_mem_Words(&ip, 0, code_mem, 27);
  XAd_multicycle_pipeline_ip_Start(&ip);
  while (!XAd_multicycle_pipeline_ip_IsDone(&ip));
  printf("\ndata memory dump (non null words)\n");
  //remember: char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT] = {0};
  for (int i=0; i<DATA_MEM_SIZE/sizeof(int)/4; i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_0_0_Bytes(&ip, i, &b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_1_0_Bytes(&ip, i, &b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_2_0_Bytes(&ip, i, &b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_3_0_Bytes(&ip, i, &b3, 1);
	w = ((unsigned char)b3<<24) | ((unsigned char)b2<<16) | ((unsigned char)b1<<8) | (unsigned char)b0;
	if (w != 0)
		printf("m[%4d] = %16d (%8x)\n", i, w, (unsigned int)w);
  }
  for (int i=0; i<DATA_MEM_SIZE/sizeof(int)/4; i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_0_1_Bytes(&ip, i, &b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_1_1_Bytes(&ip, i, &b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_2_1_Bytes(&ip, i, &b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_3_1_Bytes(&ip, i, &b3, 1);
	w = ((unsigned char)b3<<24) | ((unsigned char)b2<<16) | ((unsigned char)b1<<8) | (unsigned char)b0;
	if (w != 0)
		printf("m[%4d] = %16d (%8x)\n", i, w, (unsigned int)w);
  }
  for (int i=0; i<DATA_MEM_SIZE/sizeof(int)/4; i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_0_2_Bytes(&ip, i, &b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_1_2_Bytes(&ip, i, &b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_2_2_Bytes(&ip, i, &b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_3_2_Bytes(&ip, i, &b3, 1);
	w = ((unsigned char)b3<<24) | ((unsigned char)b2<<16) | ((unsigned char)b1<<8) | (unsigned char)b0;
	if (w != 0)
		printf("m[%4d] = %16d (%8x)\n", i, w, (unsigned int)w);
  }
  for (int i=0; i<DATA_MEM_SIZE/sizeof(int)/4; i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_0_3_Bytes(&ip, i, &b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_1_3_Bytes(&ip, i, &b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_2_3_Bytes(&ip, i, &b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_data_mem_3_3_Bytes(&ip, i, &b3, 1);
	w = ((unsigned char)b3<<24) | ((unsigned char)b2<<16) | ((unsigned char)b1<<8) | (unsigned char)b0;
	if (w != 0)
		printf("m[%4d] = %16d (%8x)\n", i, w, (unsigned int)w);
  }
  //==================ARRAY PARTITIONED IN [][AD_DERIV_CNT] Parts - 7 parts, one for each deriv??
  for (int i=0; i<(DATA_MEM_SIZE/sizeof(int)/AD_DERIV_CNT); i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_0_0_Bytes(&ip, i, &ad_b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_1_0_Bytes(&ip, i, &ad_b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_2_0_Bytes(&ip, i, &ad_b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_3_0_Bytes(&ip, i, &ad_b3, 1);
	ad_w = ((unsigned char)ad_b3<<24) | ((unsigned char)ad_b2<<16) | ((unsigned char)ad_b1<<8) | ((unsigned char)ad_b0);
	if (ad_w != 0)
		printf("ad_m[%4d][%4d] = %16d (%8x)\n", i, 0, ad_w, (unsigned int)ad_w);
  }
  for (int i=0; i<(DATA_MEM_SIZE/sizeof(int)/AD_DERIV_CNT); i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_0_1_Bytes(&ip, i, &ad_b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_1_1_Bytes(&ip, i, &ad_b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_2_1_Bytes(&ip, i, &ad_b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_3_1_Bytes(&ip, i, &ad_b3, 1);
	ad_w = ((unsigned char)ad_b3<<24) | ((unsigned char)ad_b2<<16) | ((unsigned char)ad_b1<<8) | ((unsigned char)ad_b0);
	if (ad_w != 0)
		printf("ad_m[%4d][%4d] = %16d (%8x)\n", i, 1, ad_w, (unsigned int)ad_w);
  }
  for (int i=0; i<(DATA_MEM_SIZE/sizeof(int)/AD_DERIV_CNT); i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_0_2_Bytes(&ip, i, &ad_b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_1_2_Bytes(&ip, i, &ad_b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_2_2_Bytes(&ip, i, &ad_b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_3_2_Bytes(&ip, i, &ad_b3, 1);
	ad_w = ((unsigned char)ad_b3<<24) | ((unsigned char)ad_b2<<16) | ((unsigned char)ad_b1<<8) | ((unsigned char)ad_b0);
	if (ad_w != 0)
		printf("ad_m[%4d][%4d] = %16d (%8x)\n", i, 2, ad_w, (unsigned int)ad_w);
  }
  /*for (int i=0; i<(DATA_MEM_SIZE/sizeof(int)/AD_DERIV_CNT); i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_0_3_Bytes(&ip, i, &ad_b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_1_3_Bytes(&ip, i, &ad_b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_2_3_Bytes(&ip, i, &ad_b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_3_3_Bytes(&ip, i, &ad_b3, 1);
	ad_w = ((unsigned char)ad_b3<<24) | ((unsigned char)ad_b2<<16) | ((unsigned char)ad_b1<<8) | ((unsigned char)ad_b0);
	if (ad_w != 0)
		printf("ad_m[%4d][%4d] = %16d (%8x)\n", i, 3, ad_w, (unsigned int)ad_w);
  }
  for (int i=0; i<(DATA_MEM_SIZE/sizeof(int)/AD_DERIV_CNT); i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_0_4_Bytes(&ip, i, &ad_b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_1_4_Bytes(&ip, i, &ad_b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_2_4_Bytes(&ip, i, &ad_b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_3_4_Bytes(&ip, i, &ad_b3, 1);
	ad_w = ((unsigned char)ad_b3<<24) | ((unsigned char)ad_b2<<16) | ((unsigned char)ad_b1<<8) | ((unsigned char)ad_b0);
	if (ad_w != 0)
		printf("ad_m[%4d][%4d] = %16d (%8x)\n", i, 4, ad_w, (unsigned int)ad_w);
  }
  for (int i=0; i<(DATA_MEM_SIZE/sizeof(int)/AD_DERIV_CNT); i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_0_5_Bytes(&ip, i, &ad_b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_1_5_Bytes(&ip, i, &ad_b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_2_5_Bytes(&ip, i, &ad_b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_3_5_Bytes(&ip, i, &ad_b3, 1);
	ad_w = ((unsigned char)ad_b3<<24) | ((unsigned char)ad_b2<<16) | ((unsigned char)ad_b1<<8) | ((unsigned char)ad_b0);
	if (ad_w != 0)
		printf("ad_m[%4d][%4d] = %16d (%8x)\n", i, 5, ad_w, (unsigned int)ad_w);
  }
  for (int i=0; i<(DATA_MEM_SIZE/sizeof(int)/AD_DERIV_CNT); i++){
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_0_6_Bytes(&ip, i, &ad_b0, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_1_6_Bytes(&ip, i, &ad_b1, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_2_6_Bytes(&ip, i, &ad_b2, 1);
	XAd_multicycle_pipeline_ip_Read_data_mem_ad_data_mem_3_6_Bytes(&ip, i, &ad_b3, 1);
	ad_w = ((unsigned char)ad_b3<<24) | ((unsigned char)ad_b2<<16) | ((unsigned char)ad_b1<<8) | ((unsigned char)ad_b0);
	if (ad_w != 0)
		printf("ad_m[%4d][%4d] = %16d (%8x)\n", i, 6, ad_w, (unsigned int)ad_w);
  }*/
  nbi = XAd_multicycle_pipeline_ip_Get_nb_instruction(&ip);
  nbc = XAd_multicycle_pipeline_ip_Get_nb_cycle(&ip);
  printf("\n%d fetched and decoded instructions\
 in %d cycles (ipc = %2.2f)\n", nbi, nbc, ((float)nbi)/nbc);
  return 1;
}

int main()
{
    init_platform();
    print("Hello World\n\r");
    print("Successfully ran Hello World application");
    ad_main();
    cleanup_platform();
    return 0;
}
