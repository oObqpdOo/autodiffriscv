#ifndef __MEM_ACCESS
#define __MEM_ACCESS

#include "ad_multicycle_pipeline_ip.h"

void mem_access(
  from_e_1_to_m_t m_from_e_1,
  data_mem_t     *data_mem,
  m_state_t      *m_state,
  from_m_to_w_t  *m_to_w);

#endif
