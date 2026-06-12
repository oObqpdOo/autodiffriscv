#ifndef __NEW_CYCLE
#define __NEW_CYCLE

#include "ad_multicycle_pipeline_ip.h"

void new_cycle(
  from_f_to_d_t      f_to_d,
  from_d_to_f_t      d_to_f,
  from_d_to_i_t      d_to_i,
  from_i_to_e_1_t    i_to_e_1,
  from_e_1_to_f_t    e_1_to_f,
  from_e_1_to_e_2_t  e_1_to_e_2,
  from_e_1_to_m_t    e_1_to_m,
  from_e_2_to_w_t    e_2_to_w,
  from_m_to_w_t      m_to_w,
  from_d_to_f_t     *f_from_d,
  from_e_1_to_f_t   *f_from_e_1,
  from_f_to_d_t     *d_from_f,
  from_d_to_i_t     *i_from_d,
  from_i_to_e_1_t   *e_1_from_i,
  from_e_1_to_e_2_t *e_2_from_e_1,
  from_e_1_to_m_t   *m_from_e_1,
  from_e_2_to_w_t   *w_from_e_2,
  from_m_to_w_t     *w_from_m);

#endif
