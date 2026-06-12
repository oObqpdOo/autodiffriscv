#include "adrv32imf_mp_ip.h"

void lock_unlock_reg(
  reg_num_t i_destination,
#ifdef FPU
  bit_t i_type,
#endif
  reg_num_t w_destination,
#ifdef FPU
  bit_t wb_type,
#endif
#ifdef AD
  reg_t    *reg_file_s
#else
  bit_t    *is_reg_computed
#endif
  ){
#pragma HLS INLINE off
#ifdef AD
  bit_t *is_reg_computed = reg_file_s->is_reg_computed;
#endif
  reg_num_p1_t r;
  for (r=1; r<NB_REGISTER; r++){ //starts with 1, as WZR cannot be written anyway by design!
#pragma HLS UNROLL skip_exit_check
    if (w_destination == r
#ifdef FPU
        && wb_type == WB_TYPE_NO_FP
#endif
	    ){
      is_reg_computed[w_destination] = 0;
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG_MINIMAL
      printf("UNLOCKED REGISTER R%d\n", r);
#endif
#endif
    }
    else if (i_destination == r
#ifdef FPU
    	&& i_type == I_TYPE_NO_FP
#endif
	  ){
      is_reg_computed[i_destination] = 1;
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG_MINIMAL
      printf("LOCKED REGISTER R%d\n", r);
#endif
#endif
    }
  }
}

//TODO: Do I Have to LOCK FCSR as well before reading it???
//TODO: Or just keep access to FCSR LOCAL TO Execute 1 Stage maybe??

#ifdef FPU
void lock_unlock_freg(
  reg_num_t i_destination,
  bit_t i_type,
  reg_num_t w_destination,
  bit_t wb_type,
#ifdef AD
  reg_t    *reg_file_s
#else
  bit_t    *is_freg_computed,
  bit_t	   *is_fcsr_computed
#endif
  ){
#pragma HLS INLINE off
#ifdef AD
  bit_t *is_freg_computed = reg_file_s->is_freg_computed;
  bit_t *is_fcsr_computed = &(reg_file_s->is_fcsr_computed);
#endif
  reg_num_p1_t r;
  for (r=0; r<NB_REGISTER; r++){
#pragma HLS UNROLL skip_exit_check
    if (w_destination == r && wb_type == WB_TYPE_FP){
      is_freg_computed[w_destination] = 0;
#ifdef LOCK_FCSR
      *is_fcsr_computed = 0;
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG_MINIMAL
      printf("UNLOCKED FLOATINGPOINT REGISTER F%d\n", r);
#ifdef LOCK_FCSR
      printf("UNLOCKED FCSR REGISTER FOR CSSRS/CSSRW!\n");
#endif
#endif
#endif
    }
    else if (i_destination == r && i_type == I_TYPE_FP){
      is_freg_computed[i_destination] = 1;
#ifdef LOCK_FCSR
      *is_fcsr_computed = 1;
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG_MINIMAL
      printf("LOCKED FLOATINGPOINT REGISTER F%d\n", r);
#ifdef LOCK_FCSR
      printf("LOCKED FCSR REGISTER FOR CSSRS/CSSRW!\n");
#endif
#endif
#endif
    }
  }
}
#endif
