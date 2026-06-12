#include <stdio.h>
#include "platform.h"
#include "xmulticycle_pipeline_ip.h"
#include "xparameters.h"

#define LOG_DATA_MEM_SIZE 11
//size in bytes
#define DATA_MEM_SIZE     (1<<LOG_DATA_MEM_SIZE)

XMulticycle_pipeline_ip_Config *cfg_ptr;
XMulticycle_pipeline_ip         ip;

word_type code_mem[16] = {
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

int main()
{
  char b0, b1, b2, b3;
  int  w, nbi, nbc;
  cfg_ptr = XMulticycle_pipeline_ip_LookupConfig(XPAR_XMULTICYCLE_PIPELINE_IP_0_DEVICE_ID);
  XMulticycle_pipeline_ip_CfgInitialize(&ip, cfg_ptr);
  XMulticycle_pipeline_ip_Set_start_pc(&ip, 0);
  XMulticycle_pipeline_ip_Write_code_mem_Words(&ip, 0, code_mem, 16);
  XMulticycle_pipeline_ip_Start(&ip);
  while (!XMulticycle_pipeline_ip_IsDone(&ip));
  printf("\ndata memory dump (non null words)\n");
  for (int i=0; i<DATA_MEM_SIZE/sizeof(int); i++){
    XMulticycle_pipeline_ip_Read_data_mem_0_Bytes(&ip, i, &b0, 1);
    XMulticycle_pipeline_ip_Read_data_mem_1_Bytes(&ip, i, &b1, 1);
    XMulticycle_pipeline_ip_Read_data_mem_2_Bytes(&ip, i, &b2, 1);
    XMulticycle_pipeline_ip_Read_data_mem_3_Bytes(&ip, i, &b3, 1);
    w = ((unsigned char)b3<<24) | ((unsigned char)b2<<16) | ((unsigned char)b1<<8) | (unsigned char)b0;
    if (w != 0)
      printf("m[%4d] = %16d (%8x)\n", i, w, (unsigned int)w);
  }
  nbi = XMulticycle_pipeline_ip_Get_nb_instruction(&ip);
  nbc = XMulticycle_pipeline_ip_Get_nb_cycle(&ip);
  printf("\n%d fetched and decoded instructions\
 in %d cycles (ipc = %2.2f)\n", nbi, nbc, ((float)nbi)/nbc);
}

