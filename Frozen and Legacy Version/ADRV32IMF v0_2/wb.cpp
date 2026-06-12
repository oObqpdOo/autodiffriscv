#include "debug_adrv32imf_mp_ip.h"
#include "adrv32imf_mp_ip.h"
#include "ap_int.h"
#include "disassemble.h"
#include "emulate.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include "stdio.h"
#endif
#endif


#ifdef AD
static void new_build_word(
  unsigned char b0,
  unsigned char b1,
  unsigned char b2,
  unsigned char b3,
  ap_uint<2>    a01,
  bit_t         a1,
  func3_t       msize,
  int          *result){
	*result = b0 + (b1 << 8) + (b2 << 16) + (b3 << 24);
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
	printf("AD_DEBUG: WB BUILD_WORD CALCULATED RESULT: %d \n", *result);
#endif
#endif
}
#endif

static void build_word(
  unsigned char b0,
  unsigned char b1,
  unsigned char b2,
  unsigned char b3,
  ap_uint<2>    a01,
  bit_t         a1,
  func3_t       msize,
  int          *result){
  char           b;
  unsigned char  ub;
  short          h, h0, h1;
  unsigned short uh, uh0, uh1;
  int            w, ib, ih;
  unsigned int   iub, iuh;
  h0  = ((unsigned short)b1 <<  8) | (unsigned short)b0;
  uh0 = h0;
  h1  = ((unsigned short)b3 <<  8) | (unsigned short)b2;
  uh1 = h1;
  switch(a01){
    case 0b00: b = b0; break;
    case 0b01: b = b1; break;
    case 0b10: b = b2; break;
    case 0b11: b = b3; break;
  }
  ub  = b;
  ib  = b;
  iub = ub;
  h   = (a1)?h1:h0;
  uh  = h;
  ih  = h;
  iuh = uh;
  //The following adaption was made by me in the ad processor
  //TODO: w = (uh1 << 16) | uh0; //Why isn't there a Typecast???????????????????
#ifdef USE_OLD_BUG
  w = (uh1 << 16) | uh0;
#else
  w = ((unsigned int)uh1 << 16) | uh0;
#endif
  switch(msize){
    case LB:
      *result = ib;  break;
    case LH:
      *result = ih;  break;
    case LW:
      *result = w;   break;
    case 3:
      *result = 0;   break;
    case LBU:
      *result = iub; break;
    case LHU:
      *result = iuh; break;
    case 6:
    case 7:
      *result = 0;   break;
  }
#ifdef AD
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
  printf("AD_DEBUG: WB BUILD_WORD CALCULATED RESULT: %d \n", *result);
#endif
#endif
#endif
}

#ifdef AD
static void build_ad_word(
	char ad_b0[AD_DERIV_CNT],
	char ad_b1[AD_DERIV_CNT],
	char ad_b2[AD_DERIV_CNT],
	char ad_b3[AD_DERIV_CNT],
	ap_uint<2>    a01,
	bit_t         a1,
	func3_t       msize,
	int           ad_result[AD_DERIV_CNT],
	bit_t 		  is_ad_store,
	unsigned long length){
		unsigned long int i;
		//TODO: PRAGMAS CHECK
#pragma HLS ARRAY_PARTITION variable=ad_b0 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=ad_b1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=ad_b2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=ad_b3 dim=1 complete
		//
#pragma HLS PIPELINE II=1
#pragma HLS LATENCY max=1
		for(i = 0; i < length; i++){
#pragma HLS unroll
			//can't just call build_word here, when ad_lr is the instruction, cause it checks for fnc3 code, but that is custom 0b111
			//0b111 isn't checked, build words only checks 2 LSBs for lw, lh or lb type, for our custom ad_lr we improvise
			//TODO: do i need to use original build_word for anything at all? lb instruction?
			//TODO: TODO
			//TODO: TODO
			if(!is_ad_store){
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
				printf("AD_DEBUG: NO AD_STORE - USE STANDARD BUILD_WORD");
#endif
#endif
				build_word(	ad_b0[i],
							ad_b1[i],
							ad_b2[i],
							ad_b3[i],
							a01,
							a1,
							msize,
							&ad_result[i]);
			}
			else{
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
				printf("AD_DEBUG: AD_STORE - DON'T!! USE STANDARD BUILD_WORD\n");
#endif
#endif
				ad_result[i] = ((unsigned int)ad_b0[i] & 0XFF) + (((unsigned int)ad_b1[i] << 8) & 0xFF00) + (((unsigned int)ad_b2[i] << 16) & 0XFF0000) + (((unsigned int)ad_b3[i] << 24) & 0xFF000000);
			}
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
			printf("AD_DEBUG:BUILD_AD_WORD due to IS_AD_STORE ADSW OR ADLR\n");
			printf("AD_DEBUG: char0 = %d\n", ad_b0[i]);
			printf("AD_DEBUG: char1 = %d\n", ad_b1[i]);
			printf("AD_DEBUG: char2 = %d\n", ad_b2[i]);
			printf("AD_DEBUG: char3 = %d\n", ad_b3[i]);
			printf("AD_DEBUG: RESULT = %d\n", ad_result[i]);
#endif
#endif
		}
	return;
}
#endif

static void get_input(
  from_e_2_to_w_t w_from_e_2,
  from_m_to_w_t   w_from_m,
  w_state_t      *w_state){
#pragma HLS INLINE
  int result;
#ifdef AD
  int ad_result[AD_DERIV_CNT];
  //TODO: understand why the following creates extreme neg. slack due to dependencies?
  #pragma HLS ARRAY_PARTITION variable=ad_result dim=1 complete
  if (w_from_m.is_valid){
	w_state->is_ad_store = w_from_m.is_ad_store;
	//can't just call build_word here as well, when ad_lr is the instruction, cause it checks for fnc3 code, but that is custom 0b111
	//0b111 isn't checked, build words only checks 2 LSBs for lw, lh or lb type, for our custom ad_lr we improvise
	//TODO: do i need to use original build_word for anything at all? lb instruction?
	//TODO: TODO
	//TODO: TODO
	if(!w_state->is_ad_store){
		build_word(w_from_m.value_0,
				   w_from_m.value_1,
				   w_from_m.value_2,
				   w_from_m.value_3,
				   w_from_m.a01,
				   w_from_m.a1,
				   w_from_m.msize,
				  &result);
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
		printf("AD_DEBUG: DETECTED NORMAL STORE: REGISTER_RESULT = %d \n", result);
#endif
#endif
	}
	else{
		//basically branch here if op = ADLR
		result = (w_from_m.value_0 & 0xFF) + (((unsigned int)w_from_m.value_1 << 8) & 0xFF00) + (((unsigned int)w_from_m.value_2 << 16) & 0xFF0000) + (((unsigned int)w_from_m.value_3 << 24) & 0xFF000000);
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
		printf("AD_DEBUG: DETECTED AD_STORE OP DURING WRITEBACK: CUSTOM_REGISTER_RESULT = %d (%x) \n", result, result);
#endif
#endif
	}
    build_ad_word(
    		   w_from_m.ad_value_0,
			   w_from_m.ad_value_1,
			   w_from_m.ad_value_2,
			   w_from_m.ad_value_3,
               w_from_m.a01,
               w_from_m.a1,
               w_from_m.msize,
               ad_result,
			   w_from_m.is_ad_store,
			   AD_DERIV_CNT);
#else
  if (w_from_m.is_valid){
	build_word(w_from_m.value_0,
			   w_from_m.value_1,
			   w_from_m.value_2,
			   w_from_m.value_3,
			   w_from_m.a01,
			   w_from_m.a1,
			   w_from_m.msize,
			  &result);
#endif
    w_state->value.i             = result;
#ifdef AD
    copy_int_array_to_float_int_t(w_state->ad_value, ad_result, AD_DERIV_CNT);
    w_state->is_ad_op			 = w_from_m.is_ad_op;
#endif
    w_state->rd                  = w_from_m.rd;
    w_state->has_no_dest         = w_from_m.has_no_dest;
    w_state->is_ret              = 0;
#ifndef __SYNTHESIS__
    w_state->pc                  = w_from_m.pc;
    w_state->instruction         = w_from_m.instruction;
    w_state->decoded_instruction = w_from_m.decoded_instruction;
#endif
#ifdef FPU
    w_state->wb_info_fpu		 = w_from_m.wb_info_fpu;
#endif
  }
  else{ //if (w_from_e_2.is_valid){
    w_state->value               = w_from_e_2.result;
#ifdef AD
    copy_array_float_int_t(w_state->ad_value, w_from_e_2.ad_result, AD_DERIV_CNT);
    w_state->is_ad_op			 = w_from_e_2.is_ad_op;
#endif
    w_state->rd                  = w_from_e_2.rd;
    w_state->has_no_dest         = w_from_e_2.has_no_dest;
    w_state->is_ret              = w_from_e_2.is_ret;
    w_state->is_exit             = w_from_e_2.is_exit;
#ifndef __SYNTHESIS__
#ifdef DEBUG_EXIT
       printf("WB: IS_EXIT: %d\n", w_state->is_exit);
#endif
#endif
#ifdef FPU
    w_state->wb_info_fpu		 = w_from_e_2.wb_info_fpu;
#endif
#ifndef __SYNTHESIS__
    w_state->pc                  = w_from_e_2.pc;
    w_state->instruction         = w_from_e_2.instruction;
    w_state->decoded_instruction = w_from_e_2.decoded_instruction;
    w_state->target_pc           = w_from_e_2.target_pc;
#endif
  }

#ifndef __SYNTHESIS__
#ifdef DEBUG_WB
  	  printf("WB: w_from_m: %d; w_from_e2: %d\n", (w_from_m.is_valid), (w_from_e_2.is_valid));
      printf("WB: res: %d;\n", w_from_e_2.result);

#endif
#endif

}

static void stage_job(
  w_state_t 		   	  *w_state,
#ifdef AD
  reg_t					  *reg_file_s,
  bit_t					  *wb_type
#else //AD
  int       		   	  *reg_file
#ifdef FPU
  ,float 			   	  *freg_file,
  int                  	  *fcsr, //TODO: get FCSR here and PROTECT/ LOCK register
  bit_t			 	      *wb_type
#endif //FPU
  //float_int_t           rv1,
  //float_int_t           rv2,
//decoded_instruction_t d_i, //TODO: DO NOT PASS ALL, INSTEAD PASS ONLY NEEDED ONES
  //float_int_t           result
#endif //ifndef AD
						){
#pragma HLS INLINE
#ifndef FPU
  if (!w_state->has_no_dest){
    reg_file[w_state->rd] = w_state->value.i;
  }
}
#else
#ifdef AD
  //TODO: pointer passing good option? or use original pointers directly
  int *reg_file = reg_file_s->reg_file; //name of the array is the pointer to the first element
  float *freg_file = reg_file_s->freg_file;
  int *fcsr = &(reg_file_s->fcsr);
  int null_v = 0;
  float null_v_f = 0.0;
#endif
  ap_uint<5> fflag;
  bit_t      is_fmv_fcmp_fcvt;
  bit_t      has_destination;
  bit_t      rv1_is_zero;
  bit_t      rv2_is_zero;

  fpu_inst_for_wb_s wb_i = w_state->wb_info_fpu; //writeback_info
  float_int_t       rv1     = wb_i.rv1;
  float_int_t       rv2 	= wb_i.rv2;
  float_int_t		result  = w_state->value;
  is_fmv_fcmp_fcvt  = (wb_i.is_fmv_x_w || wb_i.is_fcmp || wb_i.is_fcvt_w_s);
  //has_destination   =  wb_i.rd != 0;
  has_destination   = !(w_state->has_no_dest);
  rv1_is_zero       = (rv1.i == POSITIVE_ZERO) || (rv1.i == NEGATIVE_ZERO);
  rv2_is_zero       = (rv2.i == POSITIVE_ZERO) || (rv2.i == NEGATIVE_ZERO);
  fflag             = (((((fflag | NX) | UF) | OF) | DZ) | NV) ;
#ifdef DEBUG_WB
#ifndef __SYNTHESIS__
  	  printf("WB: result = %d (%f)(%8x)\n", result.i, result.f, result.i);
  	  printf("WB: has_destination = %d\n", !w_state->has_no_dest);
#endif
#endif
  //if (has_destination){
  //TODO: Above makes no sense when writing to F0
  if (has_destination){ //Has destination now INCLUDES f0 but excludes r0//|| (!has_destination && (wb_i.is_float && !is_fmv_fcmp_fcvt && !wb_i.is_store_fp))){
#ifdef AD
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
	printf("AD_DEBUG: WRITEBACK - HAS DESTINATION\n");
	printf("AD_DEBUG: IS_AD_OP = %d\n", (unsigned int)w_state->is_ad_op);
#endif
#endif
#endif
	if (is_fmv_fcmp_fcvt                                     ||
       (!wb_i.is_branch && !wb_i.is_store && !wb_i.is_float) ||
       ( wb_i.is_system &&  wb_i.func3 == CSRRS)){
       reg_file[w_state->rd] = result.i ;
       *wb_type = WB_TYPE_NO_FP;
#ifdef DEBUG_WB
#ifndef __SYNTHESIS__
	   printf("WB: written to REG[%d] = %d (%8x)\n", w_state->rd, result.i, result.i);
#endif
#endif
    }
    if (wb_i.is_float && !is_fmv_fcmp_fcvt && !wb_i.is_store_fp){
       freg_file[w_state->rd] = result.f;
    	*wb_type = WB_TYPE_FP;
#ifdef DEBUG_WB
#ifndef __SYNTHESIS__
	   printf("WB: written to FREG[%d] = %f (%8x)\n", w_state->rd, result.f, result.i);
#endif
#endif
    }
    if (wb_i.is_system && (wb_i.func3 == CSRRW)){
       reg_file[w_state->rd] = wb_i.fcsr;
       *wb_type = WB_TYPE_NO_FP;
#ifdef DEBUG_WB
#ifndef __SYNTHESIS__
	   printf("WB: written to REG[%d] = %d (%8x)\n", w_state->rd, wb_i.fcsr, wb_i.fcsr);
#endif
#endif
    }
#ifdef AD
   if(!w_state->is_ad_op){  //If it is NOT an ad op we STILL have to overwrite the potential OLD values of the register to zero at least.
     if (is_fmv_fcmp_fcvt                                     ||
		(!wb_i.is_branch && !wb_i.is_store && !wb_i.is_float) ||
		( wb_i.is_system &&  wb_i.func3 == CSRRS)){
	   copy_value_32bit(reg_file_s->ad_reg_file[w_state->rd], &null_v, AD_DERIV_CNT);
#ifdef DEBUG_WB
#ifndef __SYNTHESIS__
	   printf("AD_DEBUG: FLUSHING ALL AD_REGISTERS TO ZERO!!!!\n");
#endif
#endif
	  }
	  if (wb_i.is_float && !is_fmv_fcmp_fcvt && !wb_i.is_store_fp){
	    copy_value_float(reg_file_s->ad_freg_file[w_state->rd], &null_v_f, AD_DERIV_CNT);
#ifdef DEBUG_WB
#ifndef __SYNTHESIS__
	    printf("AD_DEBUG: FLUSHING ALL AD_FPU_REGISTERS TO ZERO!!!!\n");
#endif
#endif
	  }
	  if (wb_i.is_system && (wb_i.func3 == CSRRW)){
		//TODO: what do we do in this case? nothing, i assume
	  }
	}
	//only write back and overwrite AD_Register if instruction is AD_OP, else (e.g. logic OPS etc. leave as they are)
	//TODO: WHY IS THIS ADDING A DEPENDENCY BETWEEN EXECUTE2 AND WB?? NO, MEM AND WB (waiting for m_to_w.is_valid)
	//TODO: OR DUE TOO is_ad_op getting written in MEM state?
   else if(w_state->is_ad_op){
	  if (is_fmv_fcmp_fcvt                                     ||
	    (!wb_i.is_branch && !wb_i.is_store && !wb_i.is_float) ||
	    ( wb_i.is_system &&  wb_i.func3 == CSRRS)){
		copy_float_int_t_array_to_int(reg_file_s->ad_reg_file[w_state->rd], w_state->ad_value, AD_DERIV_CNT);
#ifdef DEBUG_WB
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
	    printf("AD_DEBUG: WRITEBACK TO REG\n");
	    for(int i = 0; i < AD_DERIV_CNT; i++){
	      //printf("AD_REG[%d][%d] WILL BE %d\n", (unsigned int)w_state->rd, i, w_state->ad_value[i]);
	      printf("AD_REG[%d][%d] SET TO %d\n", (unsigned int)w_state->rd, i, reg_file_s->ad_reg_file[w_state->rd][i]);
	    }
#endif
#endif
#endif
	  }
	  if (wb_i.is_float && !is_fmv_fcmp_fcvt && !wb_i.is_store_fp){
		copy_float_int_t_array_to_float(reg_file_s->ad_freg_file[w_state->rd], w_state->ad_value, AD_DERIV_CNT);
#ifdef DEBUG_WB
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
	    printf("AD_DEBUG: WRITEBACK TO FREG\n");
	    for(int i = 0; i < AD_DERIV_CNT; i++){
		  //printf("AD_REG[%d][%d] WILL BE %d\n", (unsigned int)w_state->rd, i, w_state->ad_value[i]);
		  printf("AD_FREG[%d][%d] SET TO %f\n", (unsigned int)w_state->rd, i, reg_file_s->ad_freg_file[w_state->rd][i]);
	    }
#endif
#endif
#endif
	  }
	  if (wb_i.is_system && (wb_i.func3 == CSRRW)){
	    //TODO: what do we do in this case? nothing, i assume
	  }
    }//end of else if(w_state->is_ad_op){
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
    //TODO: this is never going to happen
	else
	  printf("AD_DEBUG: NO WRITEBACK TO AD_REGS\n");
#endif
#endif
#endif
  } // end of if(has_destination){
#ifdef AD
 #ifdef DEBUG_AD
#ifndef __SYNTHESIS__
  else{
	printf("AD_DEBUG: has_destination = %d\n", has_destination);
  }
#endif
#endif
#endif
  if (wb_i.is_system && (wb_i.func3 == CSRRW))
    *fcsr = result.i & fflag;
  if (wb_i.is_float && !wb_i.is_store_fp && !wb_i.is_load_fp){
    if (((result.i == QNAN)                                                    ||
       (wb_i.is_fcmp     && (rv1.i == QNAN          || rv2.i == QNAN)))        ||
       (wb_i.is_fcvt_w_s && (rv1.i == PLUS_INFINITY || rv1.i == MINUS_INFINITY ||
                            (rv1.i == QNAN)         || (rv1.part.exponent > EMAX)))){
      *fcsr = wb_i.fcsr | NV;
#ifndef __SYNTHESIS__
#ifdef DEBUG_FPU_FLAGS
    	printf("FCSR: SET NV\n");
#endif
#endif
    }
  }
  if (wb_i.is_op_fp && (wb_i.func5 == FDIV) && !rv1_is_zero && rv2_is_zero){
   *fcsr = wb_i.fcsr | DZ;
#ifndef __SYNTHESIS__
#ifdef DEBUG_FPU_FLAGS
    	printf("FCSR: SET DZ\n");
#endif
#endif
  }
  if (wb_i.is_execpt_inst && (result.part.exponent > EMAX)){
   *fcsr = wb_i.fcsr  | OF | NX;
   //*fcsr = wb_i.fcsr  | NX ;
#ifndef __SYNTHESIS__
#ifdef DEBUG_FPU_FLAGS
	printf("FCSR: SET OF\n");
	printf("FCSR: SET NX\n");
#endif
#endif
  }
  if (wb_i.is_execpt_inst && (result.part.exponent < EMIN) && (result.part.mantissa == 0)){
   *fcsr = wb_i.fcsr  | UF;
#ifndef __SYNTHESIS__
#ifdef DEBUG_FPU_FLAGS
	printf("FCSR: SET UF\n");
#endif
#endif
  }
}
#endif

void write_back(
  from_e_2_to_w_t w_from_e_2,
  from_m_to_w_t   w_from_m,
#ifdef AD
  reg_t			 *reg_file_s,
#else
  int            *reg_file,
#ifdef FPU
  float			 *freg_file,
  int			 *fcsr,
#endif
#endif
  reg_num_t      *w_destination,
#ifdef FPU
  bit_t			 *wb_type,
#endif
  w_state_t      *w_state){
#pragma HLS INLINE off
  bit_t     save_input;
  reg_num_t dest;
  dest = (w_from_m.is_valid)?
          w_from_m.rd       :
          w_from_e_2.rd;
  if (w_from_e_2.is_valid || w_from_m.is_valid)
    *w_destination = dest;
  else
    *w_destination = 0;
  save_input = (w_from_e_2.is_valid || w_from_m.is_valid);
  if (save_input){
    get_input(w_from_e_2, w_from_m, w_state);
#ifdef AD
    stage_job(w_state, reg_file_s, wb_type);
#else
    stage_job(w_state, reg_file
#ifdef FPU
    		,freg_file, fcsr, wb_type
#endif
    		);
#endif
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("wb       ");
    printf("%04d\n", (int)w_state->pc);
    if (!w_state->decoded_instruction.is_branch &&
        !w_state->decoded_instruction.is_jalr)
#ifdef FPU
#ifdef AD
    	//TODO: freg_file, fcsr don't exist so far
    	emulate(reg_file_s->reg_file,
    			reg_file_s->freg_file,
    			reg_file_s->fcsr,
			  w_state->decoded_instruction,
			  w_state->target_pc);
#else
    	//TODO: freg_file, fcsr don't exist so far
    	emulate(reg_file,
    			freg_file,
    			*fcsr,
			  w_state->decoded_instruction,
			  w_state->target_pc);
#endif
#else
    	emulate(reg_file,
              w_state->decoded_instruction,
              w_state->target_pc);
#endif
#else
#ifdef DEBUG_FETCH
    printf("%04d: %08x      ",
      (int)w_state->pc, w_state->instruction);
#ifndef DEBUG_DISASSEMBLE
    printf("\n");
#endif
#endif
#ifdef DEBUG_DISASSEMBLE
    disassemble(w_state->pc,
                w_state->instruction,
                w_state->decoded_instruction);
#endif
#ifdef DEBUG_EMULATE
#ifdef FPU
#ifdef AD // AD AND FPU
    	emulate(reg_file_s->reg_file,
    			reg_file_s->freg_file,
    			reg_file_s->fcsr,
			  w_state->decoded_instruction,
			  w_state->target_pc);
#else //JUST FPU NO AD
    	emulate(reg_file,
    			freg_file,
    			*fcsr,
			  w_state->decoded_instruction,
			  w_state->target_pc);
#endif
#else //NEITHER FPU NOR AD
    	emulate(reg_file,
              w_state->decoded_instruction,
              w_state->target_pc);
#endif
#endif
#endif
#endif
  }
}
