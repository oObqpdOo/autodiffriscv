#ifndef __MEM
#define __MEM

#include "ap_int.h"
#include "adrv32imf_mp_ip.h"


#ifdef AD
void mem_load(
#ifdef LOCAL_MEMORY
	data_mem_t    *data_mem,
#else
/*	char data_mem_0[DATA_MEM_SIZE/sizeof(int)],
	char data_mem_1[DATA_MEM_SIZE/sizeof(int)],
	char data_mem_2[DATA_MEM_SIZE/sizeof(int)],
	char data_mem_3[DATA_MEM_SIZE/sizeof(int)],
	char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	char ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	char ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
	char ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],*/
	char* data_mem_0,
	char* data_mem_1,
	char* data_mem_2,
	char* data_mem_3,
    char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    char ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    char ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    char ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
#endif
	//ap_uint<2>     a01,
	//bit_t          a1,
	data_address_t a2,
	char          *b0,
	char          *b1,
	char          *b2,
	char          *b3,
	char          ad_b0[AD_DERIV_CNT],
	char          ad_b1[AD_DERIV_CNT],
	char          ad_b2[AD_DERIV_CNT],
	char          ad_b3[AD_DERIV_CNT],
	ap_uint<3>    msize);

void mem_store(
#ifdef LOCAL_MEMORY
	data_mem_t    *data_mem,
#else
	char* data_mem_0,
	char* data_mem_1,
	char* data_mem_2,
	char* data_mem_3,
    char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    char ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    char ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    char ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
#endif
	ap_uint<2>     a01,
	bit_t          a1,
	data_address_t a2,
	char           rv2_0,
	char           rv2_1,
	char           rv2_2,
	char           rv2_3,
	char           ad_rv2_0[AD_DERIV_CNT],
	char           ad_rv2_1[AD_DERIV_CNT],
	char           ad_rv2_2[AD_DERIV_CNT],
	char           ad_rv2_3[AD_DERIV_CNT],
	ap_uint<2>     msize);

void ad_mem_store(
#ifdef LOCAL_MEMORY
	data_mem_t    *data_mem,
#else
	char* data_mem_0,
	char* data_mem_1,
	char* data_mem_2,
	char* data_mem_3,
    char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    char ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    char ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
    char ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
#endif
	ap_uint<2>     a01,
	bit_t          a1,
	data_address_t a2, //ist registerINHALT von rv1, aber nur address_size bit lang...
	int			 rv1,
	char           *rv2_0,
	char           *rv2_1,
	char           *rv2_2,
	char           *rv2_3,
	char           ad_rv2_0[AD_DERIV_CNT],
	char           ad_rv2_1[AD_DERIV_CNT],
	char           ad_rv2_2[AD_DERIV_CNT],
	char           ad_rv2_3[AD_DERIV_CNT],
	max_der_t		 ad_channel,
	bit_t			 *has_no_dest,
	bit_t			 *is_ad_op,
	reg_num_t		 *rd,
	reg_num_t		 *rs2,
	ap_uint<3>     msize);
#else
void mem_load(
  char          *data_mem_0,
  char          *data_mem_1,
  char          *data_mem_2,
  char          *data_mem_3,
  data_address_t a2,
  char          *b0,
  char          *b1,
  char          *b2,
  char          *b3);
void mem_store(
  char          *data_mem_0,
  char          *data_mem_1,
  char          *data_mem_2,
  char          *data_mem_3,
  ap_uint<2>     a01,
  bit_t          a1,
  data_address_t a2,
  char           rv2_0,
  char           rv2_1,
  char           rv2_2,
  char           rv2_3,
  ap_uint<2>     msize);
#endif
#endif
