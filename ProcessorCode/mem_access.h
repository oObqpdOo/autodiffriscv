#ifndef __MEM_ACCESS
#define __MEM_ACCESS

#include "adrv32imf_mp_ip.h"

void init_m_state(m_state_t *m_state);
void mem_access(
  from_e_1_to_m_t m_from_e_1,
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
  int            *reg_file,
#ifdef FPU
  float			 *freg_file,
  int			 *fcsr,
#endif
#endif
#endif
#ifdef AD
#ifdef LOCAL_MEMORY
  data_mem_t	 *data_mem_s,
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
#else
  char           *data_mem_0,
  char           *data_mem_1,
  char           *data_mem_2,
  char           *data_mem_3,
#endif
  m_state_t      *m_state,
  from_m_to_w_t  *m_to_w,
  bit_t          *m_state_is_full);

#endif
