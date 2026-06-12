#include "debug_multicycle_pipeline_ip.h"
#include "ad_multicycle_pipeline_ip.h"
#include "ap_int.h"
#include "disassemble.h"
#include "emulate.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include "stdio.h"
#endif
#endif

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

static void build_word(
  unsigned char b0,
  unsigned char b1,
  unsigned char b2,
  unsigned char b3,
  ap_uint<2>    a01,
  bit_t         a1,
  func3_t       msize,
  int          *result){
#pragma HLS INLINE
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
  //TODO:   w = (uh1 << 16) | uh0; //Why isn't there a Typecast???????????????????
  w = ((unsigned int)uh1 << 16) | uh0;
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
	#ifdef DEBUG_AD
	#ifndef __SYNTHESIS__
	printf("AD_DEBUG: WB BUILD_WORD CALCULATED RESULT: %d \n", *result);
	#endif
	#endif
}


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
				ad_result[i] = ((unsigned int)ad_b0[i] & 0XFF)+ (((unsigned int)ad_b1[i] << 8) & 0xFF00) + (((unsigned int)ad_b2[i] << 16) & 0XFF0000) + (((unsigned int)ad_b3[i] << 24) & 0xFF000000);
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


static void get_input(
  from_e_2_to_w_t w_from_e_2,
  from_m_to_w_t   w_from_m,
  w_state_t      *w_state){
#pragma HLS INLINE
  int result;
  int ad_result[AD_DERIV_CNT];
//TODO: understand why the following creates extreme neg. slack due to dependencies?
#pragma HLS ARRAY_PARTITION variable=ad_result dim=1 complete
  if (w_from_m.is_valid){
	w_state->is_ad_store			 = w_from_m.is_ad_store;
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
    w_state->value               = result;
    copy_array_32bit(w_state->ad_value, ad_result, AD_DERIV_CNT);
    w_state->rd                  = w_from_m.rd;
    w_state->has_no_dest         = w_from_m.has_no_dest;
    w_state->is_ad_op			 = w_from_m.is_ad_op;
    w_state->is_ret              = 0;
#ifndef __SYNTHESIS__
    w_state->pc                  = w_from_m.pc;
    w_state->instruction         = w_from_m.instruction;
    w_state->decoded_instruction = w_from_m.decoded_instruction;
#endif
  }
  else{ //if (w_from_e_2.is_valid){
    w_state->value               = w_from_e_2.result;
    copy_array_32bit(w_state->ad_value, w_from_e_2.ad_result, AD_DERIV_CNT);
    w_state->rd                  = w_from_e_2.rd;
    w_state->has_no_dest         = w_from_e_2.has_no_dest;
    //useless
    w_state->is_ad_op			 = w_from_e_2.is_ad_op;
    w_state->is_ret              = w_from_e_2.is_ret;
    w_state->is_exit             = w_from_e_2.is_exit;
#ifndef __SYNTHESIS__
    w_state->pc                  = w_from_e_2.pc;
    w_state->instruction         = w_from_e_2.instruction;
    w_state->decoded_instruction = w_from_e_2.decoded_instruction;
    w_state->target_pc           = w_from_e_2.target_pc;
#endif
  }
}
static void stage_job(
  w_state_t *w_state,
  reg_t     *reg_file){
  int null_v = 0;
#pragma HLS INLINE
  //e.g. ad_store has_no_dest, except func3 = 111
  if (!w_state->has_no_dest){
	#ifdef DEBUG_AD
	#ifndef __SYNTHESIS__
		printf("AD_DEBUG: WRITEBACK - HAS DESTINATION\n");
		printf("AD_DEBUG: IS_AD_OP = %d\n", (unsigned int)w_state->is_ad_op);
	#endif
	#endif
	if (w_state->rd == 0){
		reg_file->gp_reg_file[w_state->rd] = 0;
		//JUST THIS ONE LINE BREAKS MY II=2 ... but WHY????? --> Answer: not enough ressources on small FPGA?
		copy_value_32bit(reg_file->ad_reg_file[w_state->rd], &null_v, AD_DERIV_CNT);
		/*TRY AND ERROR
		 * what if we don't use copy_value_32bit method? independence from ressources
		 * JUST THIS ONE LINE BREAKS MY II=2 AS WELL??? ... but WHY?????
		 * --> reg_file->ad_reg_file[0][0] = 0;
		 * ERROR: still got memory dependence
		 * has to be reg_file->ad_reg_file, between mem.c and wb.c, but why?
		 */
		//reg_file->ad_reg_file[0][0] = 0;
		//I am just plain and simple stupid:
		//#pragma HLS ARRAY_PARTITION variable=reg_file.ad_reg_file dim=2 complete
		//was the mistake: dim=2 means only 2nd dimension will be partitioned... full reg file, ALL ad_regs have to be in registers instead of memory
		//so dim = 0 fixes it ... Lesson learned, Prof. Goossens even wrote that in doc p. 110
		#ifdef DEBUG_AD
		#ifndef __SYNTHESIS__
			printf("AD_DEBUG: WRITEBACK TO ZERO_REGISTER NOT ALLOWED!!!!\n");
		#endif
		#endif
	}
	else{
		reg_file->gp_reg_file[w_state->rd] = w_state->value;
	}
	//STUPID BUG -> If it is NOT an ad op we STILL have to overwrite the potential OLD values of the register to zero at least.
	if(!w_state->is_ad_op){
		copy_value_32bit(reg_file->ad_reg_file[w_state->rd], &null_v, AD_DERIV_CNT);
		#ifdef DEBUG_AD
		#ifndef __SYNTHESIS__
			printf("AD_DEBUG: FLUSHING ALL AD_REGISTERS TO ZERO!!!!\n");
		#endif
		#endif
	}
	//only write back and overwrite AD_Register if instruction is AD_OP, else (e.g. logic OPS etc. leave as they are)
	//TODO: WHY IS THIS ADDING A DEPENDENCY BETWEEN EXECUTE2 AND WB?? NO, MEM AND WB (waiting for m_to_w.is_valid)
	//TODO: OR DUE TOO is_ad_op getting written in MEM state?
	if(w_state->is_ad_op){
		if (w_state->rd == 0){
			copy_value_32bit(reg_file->ad_reg_file[w_state->rd], &null_v, AD_DERIV_CNT);
			#ifdef DEBUG_AD
			#ifndef __SYNTHESIS__
				printf("AD_DEBUG: WRITEBACK TO AD_ZERO_REGISTER NOT ALLOWED!!!!\n");
			#endif
			#endif
		}
		else{
			copy_array_32bit(reg_file->ad_reg_file[w_state->rd], w_state->ad_value, AD_DERIV_CNT);
			//int nullll = 0;
			//copy_value_32bit(reg_file->ad_reg_file[5], &nullll, AD_DERIV_CNT);
			//reg_file->ad_reg_file[12][0] = 0;
			#ifdef DEBUG_WB
			#ifndef __SYNTHESIS__
			printf("AD_DEBUG: WRITEBACK\n");
			for(int i = 0; i < AD_DERIV_CNT; i++){
				//printf("AD_REG[%d][%d] WILL BE %d\n", (unsigned int)w_state->rd, i, w_state->ad_value[i]);
				printf("AD_REG[%d][%d] SET TO %d\n", (unsigned int)w_state->rd, i, reg_file->ad_reg_file[w_state->rd][i]);
			}
			#endif
			#endif
		}
	}
	#ifdef DEBUG_AD
	#ifndef __SYNTHESIS__
		else
			printf("AD_DEBUG: NO WRITEBACK TO AD_REGS\n");
	#endif
	#endif
  }
	#ifdef DEBUG_AD
	#ifndef __SYNTHESIS__
		else
			printf("AD_DEBUG: NO!!!!!!!!!! WRITEBACK AT ALL\n");
	#endif
	#endif
}
void write_back(
  from_e_2_to_w_t w_from_e_2,
  from_m_to_w_t   w_from_m,
  reg_t          *reg_file,
  reg_num_t      *w_destination,
  w_state_t      *w_state){
#pragma HLS INLINE off
  bit_t     save_input;
  //bit_t     is_ad_op;
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
    stage_job(w_state, reg_file);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    printf("wb       ");
    printf("%04d\n", (int)w_state->pc);
    if (!w_state->decoded_instruction.is_branch &&
        !w_state->decoded_instruction.is_jalr)
      emulate(reg_file,
              w_state->decoded_instruction,
              w_state->target_pc);
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
    emulate(reg_file,
            w_state->decoded_instruction,
            w_state->target_pc);
#endif
#endif
#endif
  }
}
