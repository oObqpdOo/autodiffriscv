#include "ad_multicycle_pipeline_ip.h"

void lock_unlock_reg(
  reg_num_t i_destination,
  reg_num_t w_destination,
  reg_t    *reg_file){
#pragma HLS INLINE
  reg_num_p1_t r;
  for (r=1; r<NB_REGISTER; r++){
#pragma HLS UNROLL skip_exit_check
    if (w_destination == r){
    	reg_file->is_reg_computed[w_destination] = 0;
    }
    else if (i_destination == r){
    	reg_file->is_reg_computed[i_destination] = 1;
    }
  }
}
