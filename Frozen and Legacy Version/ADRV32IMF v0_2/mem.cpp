#include "adrv32imf_mp_ip.h"
#include "ap_int.h"

#ifndef __SYNTHESIS__
#include "stdio.h"
#endif

#ifdef AD
void mem_load(
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
		data_address_t a2,
		char          *b0,
		char          *b1,
		char          *b2,
		char          *b3,
		char          ad_b0[AD_DERIV_CNT],
		char          ad_b1[AD_DERIV_CNT],
		char          ad_b2[AD_DERIV_CNT],
		char          ad_b3[AD_DERIV_CNT],
		ap_uint<3>    msize){
#pragma HLS INLINE
#ifdef LOCAL_MEMORY
	*b0 = *(data_mem->data_mem_0 + a2);
	*b1 = *(data_mem->data_mem_1 + a2);
	*b2 = *(data_mem->data_mem_2 + a2);
	*b3 = *(data_mem->data_mem_3 + a2);
	copy_array_8bit(ad_b0, data_mem->ad_data_mem_0[a2], AD_DERIV_CNT);
	copy_array_8bit(ad_b1, data_mem->ad_data_mem_1[a2], AD_DERIV_CNT);
	copy_array_8bit(ad_b2, data_mem->ad_data_mem_2[a2], AD_DERIV_CNT);
	copy_array_8bit(ad_b3, data_mem->ad_data_mem_3[a2], AD_DERIV_CNT);
#else
	*b0 = *(data_mem_0 + a2);
	*b1 = *(data_mem_1 + a2);
	*b2 = *(data_mem_2 + a2);
	*b3 = *(data_mem_3 + a2);
	copy_array_8bit(ad_b0, ad_data_mem_0[a2], AD_DERIV_CNT);
	copy_array_8bit(ad_b1, ad_data_mem_1[a2], AD_DERIV_CNT);
	copy_array_8bit(ad_b2, ad_data_mem_2[a2], AD_DERIV_CNT);
	copy_array_8bit(ad_b3, ad_data_mem_3[a2], AD_DERIV_CNT);
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_MEM
	printf("MEM: Called mem_loadfrom MEM[%d], returning\n", a2);
	unsigned int t0 = ((unsigned int)(*b0)) & 0xFF;
	unsigned int t1 = ((unsigned int)(*b1) << 8) & 0xFF00;
	unsigned int t2 = ((unsigned int)(*b2) << 16) & 0xFF0000;
	unsigned int t3 = ((unsigned int)(*b3) << 24) & 0xFF000000;
	unsigned int itemp = t3 | t2 | t1 | t0;
	//unsigned int itemp = ((unsigned int)(*b3) << 24) | ((unsigned int)(*b2) << 16) | ((unsigned int)(*b1) << 8) | ((unsigned int)(*b0));
	float ftemp = *(float*)(&itemp);
	printf("VALUE WILL BE %4d (%8x) (%f)\n", itemp, itemp, ftemp);
#ifdef DEBUG_AD
	int i = 0;
	for(i = 0; i < AD_DERIV_CNT; i++){
#ifdef LOCAL_MEM
		printf("AD_REG[%d][%d] WILL BE %d\n", (unsigned int)a2, i, (unsigned int)(((unsigned int) (data_mem->ad_data_mem_0[a2][i])) | ((unsigned int) data_mem->ad_data_mem_1[a2][i] << 8) | ((unsigned int) data_mem->ad_data_mem_2[a2][i] << 16) | ((unsigned int) data_mem->ad_data_mem_3[a2][i] << 24)));
#else
		printf("AD_REG[%d][%d] WILL BE %d\n", (unsigned int)a2, i, (unsigned int)(((unsigned int) (ad_data_mem_0[a2][i])) | ((unsigned int) ad_data_mem_1[a2][i] << 8) | ((unsigned int) ad_data_mem_2[a2][i] << 16) | ((unsigned int) ad_data_mem_3[a2][i] << 24)));
#endif
	}
#endif
#endif
#endif
}
//TODO: what is this?? Experimental, is not called in my old code?
/*
void new_mem_load(
	data_mem_t    *data_mem,
	ap_uint<2>     a01,
	bit_t          a1,
	data_address_t a2,
	char          *b0,
	char          *b1,
	char          *b2,
	char          *b3,
	char          ad_b0[AD_DERIV_CNT],
	char          ad_b1[AD_DERIV_CNT],
	char          ad_b2[AD_DERIV_CNT],
	char          ad_b3[AD_DERIV_CNT],
	ap_uint<3>     msize){
#pragma HLS INLINE
	char		   nullv = 0;
	char		   onesv = 0xFF;
	char		   fillv;
	char		   ad_tempv;
	char		   ad_fillv;
	switch(msize){
		case LW:
			*b0 = *(data_mem->data_mem_0 + a2);
			*b1 = *(data_mem->data_mem_1 + a2);
			*b2 = *(data_mem->data_mem_2 + a2);
			*b3 = *(data_mem->data_mem_3 + a2);
			copy_array_8bit(ad_b0, data_mem->ad_data_mem_0[a2], AD_DERIV_CNT);
			copy_array_8bit(ad_b1, data_mem->ad_data_mem_1[a2], AD_DERIV_CNT);
			copy_array_8bit(ad_b2, data_mem->ad_data_mem_2[a2], AD_DERIV_CNT);
			copy_array_8bit(ad_b3, data_mem->ad_data_mem_3[a2], AD_DERIV_CNT);
			break;
//		TODO: Be very careful here, if address LSBs are 00, we need the value of b3! NOT b0, it is inverse
		case LH:
			if (a1 == 0){
				*b0 = *(data_mem->data_mem_2 + a2);
				*b1 = *(data_mem->data_mem_3 + a2);
				if(*b1 < 0)
					fillv = onesv;
				else
					fillv = nullv;
				*b2 = fillv;
				*b3 = fillv;
				for (max_der_t i = 0; i < AD_DERIV_CNT; i++){
					#pragma HLS latency max=1
					#pragma HLS PIPELINE II=1
					ad_tempv = data_mem->ad_data_mem_3[a2][i];
					if(ad_tempv < 0)
						ad_fillv = onesv;
					else
						ad_fillv = nullv;
					ad_b0[i] = data_mem->ad_data_mem_2[a2][i];
					ad_b1[i] = data_mem->ad_data_mem_3[a2][i];
					ad_b2[i] = ad_fillv;
					ad_b3[i] = ad_fillv;
				}
			}
			else{
				*b0 = *(data_mem->data_mem_0 + a2);
				*b1 = *(data_mem->data_mem_1 + a2);
				if(*b1 < 0)
					fillv = onesv;
				else
					fillv = nullv;
				*b2 = fillv;
				*b3 = fillv;
				for (max_der_t i = 0; i < AD_DERIV_CNT; i++){
					#pragma HLS latency max=1
					#pragma HLS PIPELINE II=1
					ad_tempv = data_mem->ad_data_mem_1[a2][i];
					if(ad_tempv < 0)
						ad_fillv = onesv;
					else
						ad_fillv = nullv;
					ad_b0[i] = data_mem->ad_data_mem_0[a2][i];
					ad_b1[i] = data_mem->ad_data_mem_1[a2][i];
					ad_b2[i] = ad_fillv;
					ad_b3[i] = ad_fillv;
				}
			}
			break;
		case LHU:
			if (a1 == 0){
				*b0 = *(data_mem->data_mem_2 + a2);
				*b1 = *(data_mem->data_mem_3 + a2);
				*b2 = 0;
				*b3 = 0;
				copy_array_8bit(ad_b0, data_mem->ad_data_mem_2[a2], AD_DERIV_CNT);
				copy_array_8bit(ad_b1, data_mem->ad_data_mem_3[a2], AD_DERIV_CNT);
				copy_value_8bit(ad_b2, &nullv, AD_DERIV_CNT);
				copy_value_8bit(ad_b3, &nullv, AD_DERIV_CNT);
			}
			else{
				*b0 = *(data_mem->data_mem_0 + a2);
				*b1 = *(data_mem->data_mem_1 + a2);
				*b2 = 0;
				*b3 = 0;
				copy_array_8bit(ad_b0, data_mem->ad_data_mem_0[a2], AD_DERIV_CNT);
				copy_array_8bit(ad_b1, data_mem->ad_data_mem_1[a2], AD_DERIV_CNT);
				copy_value_8bit(ad_b2, &nullv, AD_DERIV_CNT);
				copy_value_8bit(ad_b3, &nullv, AD_DERIV_CNT);
			}
			break;
    	case LB:
    		switch(a01){
    			case 0b00:
    				//carefully, adress
    				*b0 = *(data_mem->data_mem_3 + a2);
    				if(*b0 < 0)
    					fillv = onesv;
    				else
    					fillv = nullv;
    				*b1 = fillv;
    				*b2 = fillv;
    				*b3 = fillv;
					for (max_der_t i = 0; i < AD_DERIV_CNT; i++){
						#pragma HLS latency max=1
						#pragma HLS PIPELINE II=1
						ad_tempv = data_mem->ad_data_mem_3[a2][i];
    					if(ad_tempv < 0)
							ad_fillv = onesv;
    					else
    						ad_fillv = nullv;
    					ad_b0[i] = ad_tempv;
    					ad_b1[i] = ad_fillv;
    					ad_b2[i] = ad_fillv;
    					ad_b3[i] = ad_fillv;
    				}
    				break;
    			case 0b01:
    				*b0 = *(data_mem->data_mem_2 + a2);
    				if(*b0 < 0)
    					fillv = onesv;
    				else
    					fillv = nullv;
    				*b1 = fillv;
    				*b2 = fillv;
    				*b3 = fillv;
					for (max_der_t i = 0; i < AD_DERIV_CNT; i++){
						#pragma HLS latency max=1
						#pragma HLS PIPELINE II=1
						ad_tempv = data_mem->ad_data_mem_2[a2][i];
    					if(ad_tempv < 0)
							ad_fillv = onesv;
    					else
    						ad_fillv = nullv;
    					ad_b0[i] = ad_tempv;
    					ad_b1[i] = ad_fillv;
    					ad_b2[i] = ad_fillv;
    					ad_b3[i] = ad_fillv;
    				}
    				break;
    			case 0b10:
    				*b0 = *(data_mem->data_mem_1 + a2);
    				if(*b0 < 0)
    					fillv = onesv;
    				else
    					fillv = nullv;
    				*b1 = fillv;
    				*b2 = fillv;
    				*b3 = fillv;
					for (max_der_t i = 0; i < AD_DERIV_CNT; i++){
						#pragma HLS latency max=1
						#pragma HLS PIPELINE II=1
						ad_tempv = data_mem->ad_data_mem_1[a2][i];
    					if(ad_tempv < 0)
							ad_fillv = onesv;
    					else
    						ad_fillv = nullv;
    					ad_b0[i] = ad_tempv;
    					ad_b1[i] = ad_fillv;
    					ad_b2[i] = ad_fillv;
    					ad_b3[i] = ad_fillv;
    				}
    				break;
    			case 0b11:
    				*b0 = *(data_mem->data_mem_0 + a2);
    				if(*b0 < 0)
    					fillv = onesv;
    				else
    					fillv = nullv;
    				*b1 = fillv;
    				*b2 = fillv;
    				*b3 = fillv;
					for (max_der_t i = 0; i < AD_DERIV_CNT; i++){
						#pragma HLS latency max=1
						#pragma HLS PIPELINE II=1
						ad_tempv = data_mem->ad_data_mem_0[a2][i];
    					if(ad_tempv < 0)
							ad_fillv = onesv;
    					else
    						ad_fillv = nullv;
    					ad_b0[i] = ad_tempv;
    					ad_b1[i] = ad_fillv;
    					ad_b2[i] = ad_fillv;
    					ad_b3[i] = ad_fillv;
    				}
    				break;
    			}
    		break;
		case LBU:
    		switch(a01){
				case 0b00:
					*b0 = *(data_mem->data_mem_3 + a2);
					*b1 = 0;
					*b2 = 0;
					*b3 = 0;
					copy_array_8bit(ad_b0, data_mem->ad_data_mem_3[a2], AD_DERIV_CNT);
					copy_value_8bit(ad_b1, &nullv, AD_DERIV_CNT);
					copy_value_8bit(ad_b2, &nullv, AD_DERIV_CNT);
					copy_value_8bit(ad_b3, &nullv, AD_DERIV_CNT);
					break;
				case 0b01:
					*b0 = *(data_mem->data_mem_2 + a2);
					*b1 = 0;
					*b2 = 0;
					*b3 = 0;
					copy_array_8bit(ad_b0, data_mem->ad_data_mem_2[a2], AD_DERIV_CNT);
					copy_value_8bit(ad_b1, &nullv, AD_DERIV_CNT);
					copy_value_8bit(ad_b2, &nullv, AD_DERIV_CNT);
					copy_value_8bit(ad_b3, &nullv, AD_DERIV_CNT);
					break;
				case 0b10:
					*b0 = *(data_mem->data_mem_1 + a2);
					*b1 = 0;
					*b2 = 0;
					*b3 = 0;
					copy_array_8bit(ad_b0, data_mem->ad_data_mem_1[a2], AD_DERIV_CNT);
					copy_value_8bit(ad_b1, &nullv, AD_DERIV_CNT);
					copy_value_8bit(ad_b2, &nullv, AD_DERIV_CNT);
					copy_value_8bit(ad_b3, &nullv, AD_DERIV_CNT);
					break;
				case 0b11:
					*b0 = *(data_mem->data_mem_0 + a2);
					*b1 = 0;
					*b2 = 0;
					*b3 = 0;
					copy_array_8bit(ad_b0, data_mem->ad_data_mem_0[a2], AD_DERIV_CNT);
					copy_value_8bit(ad_b1, &nullv, AD_DERIV_CNT);
					copy_value_8bit(ad_b2, &nullv, AD_DERIV_CNT);
					copy_value_8bit(ad_b3, &nullv, AD_DERIV_CNT);
					break;
				}
			break;
	}
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
	printf("AD_DEBUG: PREPARE AD_LOAD\n");
	printf("AD_DEBUG:... FROM MEMORY %d\n", (unsigned int)a2);
	printf("MEMORY SLOT IS: %d\n", (int)a01);
	printf("MEM WAS %x\n", (unsigned int)(*(data_mem->data_mem_0 + a2) + (*(data_mem->data_mem_1 + a2) << 8) + (*(data_mem->data_mem_2 + a2) << 16) + (*(data_mem->data_mem_3 + a2) << 24)));
	printf("REG WILL BE %x\n", (unsigned int)(*b0 + (*b1 << 8) + (*b2 << 16) + (*b3 << 24)));
	for(int i = 0; i < AD_DERIV_CNT; i++){
		printf("MEM[%d][%d] WAS %x\n", (unsigned int)a2, i, (unsigned int)(data_mem->ad_data_mem_0[a2][i] + (data_mem->ad_data_mem_1[a2][i] << 8) + (data_mem->ad_data_mem_2[a2][i] << 16) + (data_mem->ad_data_mem_3[a2][i] << 24)));
		printf("AD_REG[%d][%d] WILL BE %x\n", (unsigned int)a2, i, (unsigned int)(ad_b0[i] + (ad_b1[i] << 8) + (ad_b2[i] << 16) + (ad_b3[i] << 24)));
	}
#endif
#endif
}
*/
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
  char          *b3){
#pragma HLS INLINE
  *b0 = *(data_mem_0 + a2);
  *b1 = *(data_mem_1 + a2);
  *b2 = *(data_mem_2 + a2);
  *b3 = *(data_mem_3 + a2);
}
#endif


#ifdef AD

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
  ap_uint<2>     msize){
#pragma HLS INLINE
  unsigned long int i;
  unsigned int nullv = 0;
  switch(msize){
    case SB:
      switch(a01){
        case 0b00:
#ifdef LOCAL_MEMORY
          *(data_mem->data_mem_0 + a2) = rv2_0;
          copy_array_8bit(data_mem->ad_data_mem_0[a2], ad_rv2_0, AD_DERIV_CNT);
#else
          *(data_mem_0 + a2) = rv2_0;
          copy_array_8bit(ad_data_mem_0[a2], ad_rv2_0, AD_DERIV_CNT);
#endif
		  break;
        case 0b01:
#ifdef LOCAL_MEMORY
          *(data_mem->data_mem_1 + a2) = rv2_0;
          copy_array_8bit(data_mem->ad_data_mem_1[a2], ad_rv2_0, AD_DERIV_CNT);
#else
          *(data_mem_1 + a2) = rv2_0;
          copy_array_8bit(ad_data_mem_1[a2], ad_rv2_0, AD_DERIV_CNT);
#endif
          break;
        case 0b10:
#ifdef LOCAL_MEMORY
          *(data_mem->data_mem_2 + a2) = rv2_0;
          copy_array_8bit(data_mem->ad_data_mem_2[a2], ad_rv2_0, AD_DERIV_CNT);
#else
          *(data_mem_2 + a2) = rv2_0;
          copy_array_8bit(ad_data_mem_2[a2], ad_rv2_0, AD_DERIV_CNT);
#endif
          break;
        case 0b11:
#ifdef LOCAL_MEMORY
          *(data_mem->data_mem_3 + a2) = rv2_0;
          copy_array_8bit(data_mem->ad_data_mem_3[a2], ad_rv2_0, AD_DERIV_CNT);
#else
          *(data_mem_3 + a2) = rv2_0;
          copy_array_8bit(ad_data_mem_3[a2], ad_rv2_0, AD_DERIV_CNT);
#endif
          break;
      }
      break;
    case SH:
      if (a1 == 0){
#ifdef LOCAL_MEMORY
        *(data_mem->data_mem_0 + a2) = rv2_0;
        *(data_mem->data_mem_1 + a2) = rv2_1;
        copy_array_8bit(data_mem->ad_data_mem_0[a2], ad_rv2_0, AD_DERIV_CNT);
        copy_array_8bit(data_mem->ad_data_mem_1[a2], ad_rv2_1, AD_DERIV_CNT);
#else
        *(data_mem_0 + a2) = rv2_0;
        *(data_mem_1 + a2) = rv2_1;
        copy_array_8bit(ad_data_mem_0[a2], ad_rv2_0, AD_DERIV_CNT);
        copy_array_8bit(ad_data_mem_1[a2], ad_rv2_1, AD_DERIV_CNT);
#endif
      }
      else{
#ifdef LOCAL_MEMORY
        *(data_mem->data_mem_2 + a2) = rv2_0;
        *(data_mem->data_mem_3 + a2) = rv2_1;
        copy_array_8bit(data_mem->ad_data_mem_2[a2], ad_rv2_0, AD_DERIV_CNT);
        copy_array_8bit(data_mem->ad_data_mem_3[a2], ad_rv2_1, AD_DERIV_CNT);
#else
        *(data_mem_2 + a2) = rv2_0;
        *(data_mem_3 + a2) = rv2_1;
        copy_array_8bit(ad_data_mem_2[a2], ad_rv2_0, AD_DERIV_CNT);
        copy_array_8bit(ad_data_mem_3[a2], ad_rv2_1, AD_DERIV_CNT);
#endif
      }
      break;
    case SW:
#ifdef LOCAL_MEMORY
      *(data_mem->data_mem_0 + a2) = rv2_0;
      *(data_mem->data_mem_1 + a2) = rv2_1;
      *(data_mem->data_mem_2 + a2) = rv2_2;
      *(data_mem->data_mem_3 + a2) = rv2_3;
      copy_array_8bit(data_mem->ad_data_mem_0[a2], ad_rv2_0, AD_DERIV_CNT);
      copy_array_8bit(data_mem->ad_data_mem_1[a2], ad_rv2_1, AD_DERIV_CNT);
      copy_array_8bit(data_mem->ad_data_mem_2[a2], ad_rv2_2, AD_DERIV_CNT);
      copy_array_8bit(data_mem->ad_data_mem_3[a2], ad_rv2_3, AD_DERIV_CNT);
#else
      *(data_mem_0 + a2) = rv2_0;
      *(data_mem_1 + a2) = rv2_1;
      *(data_mem_2 + a2) = rv2_2;
      *(data_mem_3 + a2) = rv2_3;
      copy_array_8bit(ad_data_mem_0[a2], ad_rv2_0, AD_DERIV_CNT);
      copy_array_8bit(ad_data_mem_1[a2], ad_rv2_1, AD_DERIV_CNT);
      copy_array_8bit(ad_data_mem_2[a2], ad_rv2_2, AD_DERIV_CNT);
      copy_array_8bit(ad_data_mem_3[a2], ad_rv2_3, AD_DERIV_CNT);
#endif
      break;
    case 3:

      break;
  }
#ifndef __SYNTHESIS__
#ifdef DEBUG_MEM
	printf("MEM: Called normal mem_store, returning\n");
	unsigned int t0 = ((unsigned int)(rv2_0)) & 0xFF;
	unsigned int t1 = ((unsigned int)(rv2_1) << 8) & 0xFF00;
	unsigned int t2 = ((unsigned int)(rv2_2) << 16) & 0xFF0000;
	unsigned int t3 = ((unsigned int)(rv2_3) << 24) & 0xFF000000;
	unsigned int itemp = t3 | t2 | t1 | t0;
	//unsigned int itemp = ((unsigned int)(*b3) << 24) | ((unsigned int)(*b2) << 16) | ((unsigned int)(*b1) << 8) | ((unsigned int)(*b0));
	float ftemp = *(float*)(&itemp);
	printf("MEM[%d] WILL BE %4d (%8x) (%f)\n", a2, itemp, itemp, ftemp);

	printf("MEM: actual memory value\n");
#ifdef LOCAL_MEMORY
	t0 = ((unsigned int)(*(data_mem->data_mem_0 + a2))) & 0xFF;
	t1 = ((unsigned int)(*(data_mem->data_mem_1 + a2)) << 8) & 0xFF00;
	t2 = ((unsigned int)(*(data_mem->data_mem_2 + a2)) << 16) & 0xFF0000;
	t3 = ((unsigned int)(*(data_mem->data_mem_3 + a2)) << 24) & 0xFF000000;
#else
	t0 = ((unsigned int)(*(data_mem_0 + a2))) & 0xFF;
	t1 = ((unsigned int)(*(data_mem_1 + a2)) << 8) & 0xFF00;
	t2 = ((unsigned int)(*(data_mem_2 + a2)) << 16) & 0xFF0000;
	t3 = ((unsigned int)(*(data_mem_3 + a2)) << 24) & 0xFF000000;
#endif
	itemp = t3 | t2 | t1 | t0;
	//unsigned int itemp = ((unsigned int)(*b3) << 24) | ((unsigned int)(*b2) << 16) | ((unsigned int)(*b1) << 8) | ((unsigned int)(*b0));
	ftemp = *(float*)(&itemp);
	printf("MEM[%d] IS %4d (%8x) (%f)\n", a2, itemp, itemp, ftemp);

#endif
#endif
}

#else //if not AD
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
  ap_uint<2>     msize){
#pragma HLS INLINE
  switch(msize){
    case SB:
      switch(a01){
        case 0b00:
          *(data_mem_0 + a2) = rv2_0;
          break;
        case 0b01:
          *(data_mem_1 + a2) = rv2_0;
          break;
        case 0b10:
          *(data_mem_2 + a2) = rv2_0;
          break;
        case 0b11:
          *(data_mem_3 + a2) = rv2_0;
          break;
      }
      break;
    case SH:
      if (a1 == 0){
        *(data_mem_0 + a2) = rv2_0;
        *(data_mem_1 + a2) = rv2_1;
      }
      else{
        *(data_mem_2 + a2) = rv2_0;
        *(data_mem_3 + a2) = rv2_1;
      }
      break;
    case SW:
      *(data_mem_0 + a2) = rv2_0;
      *(data_mem_1 + a2) = rv2_1;
      *(data_mem_2 + a2) = rv2_2;
      *(data_mem_3 + a2) = rv2_3;
      break;
    case 3:
      break;
  }
}
#endif

#ifdef AD
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
	ap_uint<3>     msize){
#pragma HLS INLINE
#ifndef __SYNTHESIS__
#ifdef DEBUG_MEM
	printf("AD_DEBUG: PREPARE AD_STORE OR LOAD MEM ACCESS\n");
	unsigned int t0, t1, t2, t3, itemp;
#endif
#endif
  switch(msize){
    case SB:
      switch(a01){
        case 0b00:
          //TODO: both adressing modes equal from HLS perspective?
          //*(data_mem->ad_data_mem_0 + (ad_channel * [DATA_MEM_SIZE/sizeof(int)]) + a2) = rv2_0;
#ifdef LOCAL_MEMORY
          data_mem->ad_data_mem_0[a2][ad_channel] = *rv2_0;
#else
          ad_data_mem_0[a2][ad_channel] = *rv2_0;
#endif
          break;
        case 0b01:
#ifdef LOCAL_MEMORY
          data_mem->ad_data_mem_1[a2][ad_channel] = *rv2_0;
#else
          ad_data_mem_1[a2][ad_channel] = *rv2_0;
#endif
          break;
        case 0b10:
#ifdef LOCAL_MEMORY
          data_mem->ad_data_mem_2[a2][ad_channel] = *rv2_0;
#else
          ad_data_mem_2[a2][ad_channel] = *rv2_0;
#endif
          break;
        case 0b11:
#ifdef LOCAL_MEMORY
          data_mem->ad_data_mem_3[a2][ad_channel] = *rv2_0;
#else
          ad_data_mem_3[a2][ad_channel] = *rv2_0;
#endif
          break;
      }		//TODO: Check whether rest has to be set to ZERO?
      break;
    case SH:
      if (a1 == 0){
#ifdef LOCAL_MEMORY
    	  data_mem->ad_data_mem_0[a2][ad_channel]  = *rv2_0;
    	  data_mem->ad_data_mem_1[a2][ad_channel]  = *rv2_1;
#else
    	  ad_data_mem_0[a2][ad_channel]  = *rv2_0;
    	  ad_data_mem_1[a2][ad_channel]  = *rv2_1;
#endif
      }
      else{
#ifdef LOCAL_MEMORY
    	  data_mem->ad_data_mem_2[a2][ad_channel]  = *rv2_0;
    	  data_mem->ad_data_mem_3[a2][ad_channel]  = *rv2_1;
#else
    	  ad_data_mem_2[a2][ad_channel]  = *rv2_0;
    	  ad_data_mem_3[a2][ad_channel]  = *rv2_1;
#endif
      }
      break;
    case SW:
#ifdef LOCAL_MEMORY
      data_mem->ad_data_mem_0[a2][ad_channel] = *rv2_0;
      data_mem->ad_data_mem_1[a2][ad_channel] = *rv2_1;
      data_mem->ad_data_mem_2[a2][ad_channel] = *rv2_2;
      data_mem->ad_data_mem_3[a2][ad_channel] = *rv2_3;
#else
      ad_data_mem_0[a2][ad_channel] = *rv2_0;
      ad_data_mem_1[a2][ad_channel] = *rv2_1;
      ad_data_mem_2[a2][ad_channel] = *rv2_2;
      ad_data_mem_3[a2][ad_channel] = *rv2_3;
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_MEM
	  printf("AD_DEBUG: PREPARE AD_STORE MEM ACCESS ");
	  printf("... BUT TO MEMORY %d CHANNEL %d\n", (unsigned int)a2, (unsigned int)ad_channel);
	  t0 = ((unsigned int)(*rv2_0)) & 0xFF;
	  t1 = ((unsigned int)(*rv2_1) << 8) & 0xFF00;
	  t2 = ((unsigned int)(*rv2_2) << 16) & 0xFF0000;
	  t3 = ((unsigned int)(*rv2_3) << 24) & 0xFF000000;
	  itemp = t3 | t2 | t1 | t0;
	  printf("VALUE WILL BE %d (%x)\n", itemp, itemp);
#endif
#endif
      break;
    case 3:
      //this is for RV64
      break;
    case 0b111:
      //overwrite "has_no_dest" to enable writeback to rd as if it was a load
      //AD_store is also is_ad_op true from compute_result()
      //rd is already the 5 LSBs of the shift so that would be incorrect already -> we take rv2 and just overwrite ONE ad-channel value
      *has_no_dest = 0;
      *is_ad_op = true;
      //TODO: BUT the value has to be somehow transferred to wb stage
      //m_state->value_0,
      //m_state->value_1,
      //m_state->value_2,
      //m_state->value_3,
      //m_state->value_0-3 and ad_value0-3[AD_DERIV_CNT] is present in m_state anyway and WILL be passed to wb,
      //m_state->rd is present and WILL be passed as well, has to be changed in value to rv2, can be passed from execute1 stage, no problem
      //result has to be written into ad_rv2_0 into the right place
      //TODO: what exactly IS the result, where do we find it and into what array fraction does it go?
	  *rd = *rs2;
	  //rv2_0 .. rv2_3 stays exactly the same, as well the majority of ad_channels, just one channel changes.
	  ad_rv2_0[ad_channel] = (char)rv1;
	  ad_rv2_1[ad_channel] = (char)(rv1 >> 8);
	  ad_rv2_2[ad_channel] = (char)(rv1 >> 16);
	  ad_rv2_3[ad_channel] = (char)(rv1 >> 24);
#ifndef __SYNTHESIS__
#ifdef DEBUG_MEM
	  printf("AD_DEBUG:... BUT TO REGISTER %d CHANNEL %d\n", (unsigned int)*rd, (unsigned int)ad_channel);
	  printf("VALUE WILL BE %d\n", (unsigned int)rv1);
	  printf("REMAIN RS2 REGISTER (%d) ", (unsigned int)*rd);
	  t0 = ((unsigned int)(*rv2_0)) & 0xFF;
	  t1 = ((unsigned int)(*rv2_1) << 8) & 0xFF00;
	  t2 = ((unsigned int)(*rv2_2) << 16) & 0xFF0000;
	  t3 = ((unsigned int)(*rv2_3) << 24) & 0xFF000000;
	  itemp = t3 | t2 | t1 | t0;
	  printf("VALUE WILL BE %d (%x)\n", itemp, itemp);
#endif
#endif
	break;
	//TODO: USE extra Custom-comand opcode 0b01010(11) ? for consistency? (instead of 00010)
	//otherwise, this works pretty much exactly like the above adlr, but adlw loads from memory, not register value "immediate"
	case 0b110:
	  *has_no_dest = 0;	//has to writeback to registerfile
	  *is_ad_op = true;	//has to writeback to registerfile, use instruction to clear/reset the ad-section of that register
	  *rd = *rs2;			//destination
	  char zero = 0;
	  copy_value_8bit(ad_rv2_0, &zero, AD_DERIV_CNT);
	  copy_value_8bit(ad_rv2_1, &zero, AD_DERIV_CNT);
	  copy_value_8bit(ad_rv2_2, &zero, AD_DERIV_CNT);
	  copy_value_8bit(ad_rv2_3, &zero, AD_DERIV_CNT);
	  //write rv2 back to rs2, but with ad_channel value in gp_reg part and zeroed-out ad-channels
#ifdef LOCAL_MEMORY
	  *rv2_0 = data_mem->ad_data_mem_0[a2][ad_channel];
	  *rv2_1 = data_mem->ad_data_mem_1[a2][ad_channel];
	  *rv2_2 = data_mem->ad_data_mem_2[a2][ad_channel];
	  *rv2_3 = data_mem->ad_data_mem_3[a2][ad_channel];
#else
	  *rv2_0 = ad_data_mem_0[a2][ad_channel];
	  *rv2_1 = ad_data_mem_1[a2][ad_channel];
	  *rv2_2 = ad_data_mem_2[a2][ad_channel];
	  *rv2_3 = ad_data_mem_3[a2][ad_channel];
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_MEM
	  printf("AD_DEBUG: PREPARE AD_LOAD MEM ACCESS AD_LW ");
	  printf("... FROM MEMORY %d (%x) CHANNEL %d, ", (unsigned int)a2, (unsigned int)a2, (unsigned int)ad_channel);
	  printf("VALUES WILL BE %d (%x), %d (%x), %d (%x), %d (%x)\n", *rv2_0, *rv2_0, *rv2_1, *rv2_1, *rv2_2, *rv2_2, *rv2_3, *rv2_3);
	  printf("VALUES WILL BE %d (%x), %d (%x), %d (%x), %d (%x)\n", *rv2_0, *rv2_0, ((unsigned int) *rv2_1) << 8, ((unsigned int) *rv2_1) << 8, ((unsigned int) *rv2_2) << 16, ((unsigned int) *rv2_2) << 16, ((unsigned int) *rv2_3) << 24, ((unsigned int) *rv2_3) << 24);
	  /*unsigned int t0 = ((unsigned int)(rv2_0)) & 0xFF;
	  unsigned int t1 = ((unsigned int)(rv2_1) << 8) & 0xFF00;
	  unsigned int t2 = ((unsigned int)(rv2_2) << 16) & 0xFF0000;
	  unsigned int t3 = ((unsigned int)(rv2_3) << 24) & 0xFF000000;
	  unsigned int itemp = t3 | t2 | t1 | t0;*/
	  printf("REGISTER (%d) VALUE WILL BE %d (%x)\n", (unsigned int)*rd, (unsigned int)(((unsigned int) *rv2_0 & 0xFF) + ((((unsigned int) *rv2_1) << 8) & 0xFF00) + ((((unsigned int) *rv2_2) << 16) & 0xFF0000) + ((((unsigned int) *rv2_3) << 24) & 0xFF0000)), (unsigned int)(((unsigned int) *rv2_0 & 0xFF) + ((((unsigned int) *rv2_1) << 8) & 0xFF00) + ((((unsigned int) *rv2_2) << 16) & 0xFF0000) + ((((unsigned int) *rv2_3) << 24) & 0xFF000000)));
	  printf("AD_DEBUG: AD_CHANNELS OF THAT REG WILL BE DELETED\n");
#endif
#endif
	break;
  }
#ifndef __SYNTHESIS__
#ifdef DEBUG_MEM
	printf("AD_DEBUG: Called ad_mem_store, returning\n");
#endif
#endif
}
#endif
