#include "debug_adrv32imf_mp_ip.h"
#include "adrv32imf_mp_ip.h"
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
#include "stdio.h"
#endif
#endif

void init_i_state(i_state_t *i_state){
  i_state->is_full     = 0;
}
static void get_input(
  from_d_to_i_t i_from_d,
  i_state_t    *i_state){
#pragma HLS INLINE
  i_state->decoded_instruction = i_from_d.decoded_instruction;
  i_state->pc                  = i_from_d.pc;
#ifndef __SYNTHESIS__
  i_state->instruction         = i_from_d.instruction;
  i_state->target_pc           = i_from_d.target_pc;
#endif
}
static void stage_job(
  i_state_t			i_state,
#ifdef AD
  reg_t				*reg_file_s,
#else
  int		       	*reg_file,
#ifdef FPU
  float       		*freg_file,
#endif
#endif
  float_int_t       *rv1,
  float_int_t       *rv2
#ifdef FPU
  ,float_int_t		*rv3
#ifdef AD
  ,float_int_t		ad_rv1[AD_DERIV_CNT],
  float_int_t		ad_rv2[AD_DERIV_CNT],
  float_int_t		ad_rv3[AD_DERIV_CNT]
#endif
#endif
  ){
#pragma HLS INLINE

#ifdef AD
  //TODO: pointer passing good option? or use original pointers directly
  int *reg_file = reg_file_s->reg_file; //name of the array is the pointer to the first element
  float *freg_file = reg_file_s->freg_file;
#endif

#ifdef FPU
  reg_num_t rs1 = i_state.decoded_instruction.rs1;
  reg_num_t rs2 = i_state.decoded_instruction.rs2;
  reg_num_t rs3 = i_state.decoded_instruction.rs3;
  if ((!i_state.decoded_instruction.is_float)  || i_state.decoded_instruction.is_store_fp ||
		  i_state.decoded_instruction.is_load_fp || i_state.decoded_instruction.is_system   ||
		  i_state.decoded_instruction.is_fmv_w_x || i_state.decoded_instruction.is_fcvt_s_w){
    rv1->i =  reg_file[rs1];
#ifdef AD
    copy_int_array_to_float_int_t(ad_rv1, reg_file_s->ad_reg_file[i_state.decoded_instruction.rs1], AD_DERIV_CNT); //pointer to array
#endif
  }
  else{
    rv1->f = freg_file[rs1];
#ifdef AD
    copy_float_array_to_float_int_t(ad_rv1, reg_file_s->ad_freg_file[i_state.decoded_instruction.rs1], AD_DERIV_CNT); //pointer to array
#endif
  }
  if ((i_state.decoded_instruction.is_float) && !(i_state.decoded_instruction.is_load_fp)){
    rv2->f = freg_file[rs2];
#ifdef AD
    copy_float_array_to_float_int_t(ad_rv2, reg_file_s->ad_freg_file[i_state.decoded_instruction.rs2], AD_DERIV_CNT); //pointer to array
#endif
  }
  else{
#ifdef AD
	copy_int_array_to_float_int_t(ad_rv2, reg_file_s->ad_reg_file[i_state.decoded_instruction.rs2], AD_DERIV_CNT); //pointer to array
#endif
    rv2->i =  reg_file[rs2];
  }
  rv3->f   = freg_file[rs3];
#ifdef AD
  copy_float_array_to_float_int_t(ad_rv3, reg_file_s->ad_freg_file[i_state.decoded_instruction.rs3], AD_DERIV_CNT); //pointer to array
#endif
#else
  rv1->i = reg_file[i_state.decoded_instruction.rs1];
  rv2->i = reg_file[i_state.decoded_instruction.rs2];
#endif
}
static void set_output(
  float_int_t      rv1,
  float_int_t      rv2,
#ifdef FPU
  float_int_t	   rv3,
#ifdef AD
  float_int_t	   ad_rv_1[AD_DERIV_CNT],
  float_int_t	   ad_rv_2[AD_DERIV_CNT],
  float_int_t	   ad_rv_3[AD_DERIV_CNT],
#endif
  int 			   fcsr,
#endif
  i_state_t        i_state,
  from_i_to_e_1_t *i_to_e_1){
#pragma HLS INLINE
#ifdef AD
//TODO: understand why the following creates extreme neg. slack due to dependencies?
#pragma HLS ARRAY_PARTITION variable=ad_rv_1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=ad_rv_2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=ad_rv_3 dim=1 complete
#endif
  i_to_e_1->rv1                 = rv1;
  i_to_e_1->rv2                 = rv2;
#ifdef FPU
  i_to_e_1->rv3                 = rv3;
#ifdef AD
  copy_array_float_int_t(i_to_e_1->ad_rv1, ad_rv_1, AD_DERIV_CNT);
  copy_array_float_int_t(i_to_e_1->ad_rv2, ad_rv_2, AD_DERIV_CNT);
  copy_array_float_int_t(i_to_e_1->ad_rv3, ad_rv_3, AD_DERIV_CNT);
#endif
  i_to_e_1->fcsr				= fcsr;
#endif
  i_to_e_1->decoded_instruction = i_state.decoded_instruction;
  i_to_e_1->pc                  = i_state.pc;
#ifndef __SYNTHESIS__
  i_to_e_1->instruction         = i_state.instruction;
  i_to_e_1->target_pc           = i_state.target_pc;
#endif
}
void issue(
  from_d_to_i_t    i_from_d,
  bit_t            e_1_state_is_full,
  reg_num_t       *i_destination,
#ifdef FPU
  bit_t			  *i_type,
#endif
#ifdef AD
  reg_t			  *reg_file_s,
#else
  int             *reg_file,
#ifdef FPU
  float 		  *freg_file,
  int			  *fcsr,
#endif
  bit_t           *is_reg_computed,
#ifdef FPU
  bit_t           *is_freg_computed,
  bit_t			  *is_fcsr_computed,
#endif
#endif
  i_state_t       *i_state,
  from_i_to_e_1_t *i_to_e_1,
  bit_t           *i_state_is_full){
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("\n=====ISSUE======\n");
#endif
#endif

  bit_t save_input, wait_for_source, is_locked_1, is_locked_2, is_locked_d;
#ifdef FPU
  bit_t is_locked_3;
  bit_t is_locked_fcsr;
#endif
  float_int_t rv1, rv2;
#ifdef FPU
  float_int_t rv3;
#ifdef AD
//TODO: LOCAL copies of register values for this particular stage! - can't just pass pointer here!!!
float_int_t ad_rv1[AD_DERIV_CNT];
float_int_t	ad_rv2[AD_DERIV_CNT];
float_int_t	ad_rv3[AD_DERIV_CNT];
//TODO: understand why the following creates extreme neg. slack due to dependencies?
//#pragma HLS ARRAY_PARTITION variable=ad_rv1 dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=ad_rv2 dim=1 complete
//#pragma HLS ARRAY_PARTITION variable=ad_rv3 dim=1 complete
#endif
#endif
  bit_t valid_or_full;
  valid_or_full = i_from_d.is_valid ||  i_state->is_full;
  save_input    = i_from_d.is_valid && !i_state->is_full;
#ifdef AD
  //TODO: pointer passing good option? or use original pointers directly
  bit_t *is_reg_computed = reg_file_s->is_reg_computed; //name of the array is the pointer to the first element
  bit_t *is_freg_computed = reg_file_s->is_freg_computed;
  bit_t *is_fcsr_computed = &reg_file_s->is_fcsr_computed;
#endif
#ifdef FPU
  bit_t is_fmv_fcmp_fcvt  = ( i_from_d.decoded_instruction.is_fmv_x_w ||
							  i_from_d.decoded_instruction.is_fcmp ||
							  i_from_d.decoded_instruction.is_fcvt_w_s);
  bit_t has_destination   =  !i_from_d.decoded_instruction.has_no_dest;  
  //TODO: are that really all instuctions writing to and reading from F-REGs?
  bit_t read_from_freg =  i_from_d.decoded_instruction.is_float &&
		  	  	  	  	 !i_from_d.decoded_instruction.is_fmv_w_x &&
						 !i_from_d.decoded_instruction.is_fcvt_s_w &&
		  	  	  	  	 !i_from_d.decoded_instruction.is_load_fp;
  bit_t is_load_store_fp = i_from_d.decoded_instruction.is_load_fp ||
		  	  	  	  	   i_from_d.decoded_instruction.is_store_fp;
  bit_t write_to_freg =   i_from_d.decoded_instruction.is_float && !is_fmv_fcmp_fcvt && !i_from_d.decoded_instruction.is_store_fp;
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_FREG
     printf("read_from_freg %d\n", read_from_freg);
     printf("write_to_freg %d\n", write_to_freg);
     printf("i_from_d.decoded_instruction.is_fcvt_s_w %d\n", i_from_d.decoded_instruction.is_fcvt_s_w);
     printf("i_from_d.decoded_instruction.is_fcvt_w_s %d\n", i_from_d.decoded_instruction.is_fcvt_w_s);
#endif
#endif
  is_locked_1 =
    i_from_d.decoded_instruction.is_rs1_reg &&
    ((is_reg_computed[i_from_d.decoded_instruction.rs1] && !read_from_freg) ||
     (is_freg_computed[i_from_d.decoded_instruction.rs1] && read_from_freg) ||
	 (is_freg_computed[i_from_d.decoded_instruction.rs1] && is_load_store_fp)); //load_store has always fp-reg as rs1 and normal reg as rs2 (address)
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG
     printf("is_locked_1 %d, lock_reg[%d]=%d, lock_freg[%d]=%d\n", is_locked_1, i_from_d.decoded_instruction.rs1, is_reg_computed[i_from_d.decoded_instruction.rs1], i_from_d.decoded_instruction.rs1, is_freg_computed[i_from_d.decoded_instruction.rs1]);
#endif
#endif
  is_locked_2 =
    i_from_d.decoded_instruction.is_rs2_reg &&
    ((is_reg_computed[i_from_d.decoded_instruction.rs2] && !read_from_freg) ||
	 (is_freg_computed[i_from_d.decoded_instruction.rs2] && read_from_freg)	||
	 (is_reg_computed[i_from_d.decoded_instruction.rs1] && is_load_store_fp)); //load_store has always fp-reg as rs1 and normal reg as rs2 (address));
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG
  printf("is_locked_2 %d, lock_reg[%d]=%d, lock_freg[%d]=%d\n", is_locked_2, i_from_d.decoded_instruction.rs2, is_reg_computed[i_from_d.decoded_instruction.rs2], i_from_d.decoded_instruction.rs2, is_freg_computed[i_from_d.decoded_instruction.rs2]);
#endif
#endif
  is_locked_d =
    (!i_from_d.decoded_instruction.has_no_dest) &&
    ((is_reg_computed[i_from_d.decoded_instruction.rd] && !write_to_freg) ||
	 (is_freg_computed[i_from_d.decoded_instruction.rd] && write_to_freg));
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG
  printf("is_locked_d %d, lock_reg[%d]=%d, lock_freg[%d]=%d\n", is_locked_d, i_from_d.decoded_instruction.rd, is_reg_computed[i_from_d.decoded_instruction.rd], i_from_d.decoded_instruction.rd, is_freg_computed[i_from_d.decoded_instruction.rd]);
#endif
#endif
  is_locked_3 =
    i_from_d.decoded_instruction.is_rs3_reg &&
	is_freg_computed[i_from_d.decoded_instruction.rs3];
  //TODO: this STILL wont fully work, it doesn't wait long enough!!!
  //should only wait if FCSR is READ, written is not that important, as writing happens every cycle
  is_locked_fcsr = 	(*is_fcsr_computed) &&
		  	  	  	(i_from_d.decoded_instruction.is_system) &&
					((i_from_d.decoded_instruction.func3 == CSRRW) ||
					 (i_from_d.decoded_instruction.func3 == CSRRS));
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG
  printf("is_locked_3 %d, lock_reg[%d]=%d, lock_freg[%d]=%d\n", is_locked_3, i_from_d.decoded_instruction.rs3, is_reg_computed[i_from_d.decoded_instruction.rs3], i_from_d.decoded_instruction.rs3, is_freg_computed[i_from_d.decoded_instruction.rs3]);
#endif
#endif

  /*
  if(i_from_d.decoded_instruction.is_system && (i_from_d.decoded_instruction.func3 == CSRRW) ||
    (i_from_d.decoded_instruction.is_float && !i_from_d.decoded_instruction.is_store_fp && !i_from_d.decoded_instruction.is_load_fp) ||
    (i_from_d.decoded_instruction.is_op_fp && (i_from_d.decoded_instruction.func5 == FDIV)) ||
    (i_from_d.decoded_instruction.is_execpt_inst)){
      bit_t is_locked_fcsr = 1;
    }
  */

#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_FREG
	printf("LOCKED FCSR:  ");
	printf("%d\n", (int)is_locked_fcsr);
#endif
#endif
  wait_for_source = is_locked_1 || is_locked_2 || is_locked_3 || is_locked_d || is_locked_fcsr;
#else
  is_locked_1 =
    i_from_d.decoded_instruction.is_rs1_reg &&
    is_reg_computed[i_from_d.decoded_instruction.rs1];
  is_locked_2 =
    i_from_d.decoded_instruction.is_rs2_reg &&
    is_reg_computed[i_from_d.decoded_instruction.rs2];
  is_locked_d =
  (!i_from_d.decoded_instruction.has_no_dest) &&
    is_reg_computed[i_from_d.decoded_instruction.rd];
  wait_for_source = is_locked_1 || is_locked_2 || is_locked_d;
#endif
  i_to_e_1->is_valid = !e_1_state_is_full && valid_or_full && !wait_for_source;
  if (i_to_e_1->is_valid){
	  *i_destination = i_from_d.decoded_instruction.rd;
  }
  else
    *i_destination = 0; //WZR
//TODO: Do I have to meet any other condition here? is_fp_op oder is_float
#ifdef FPU
  if (has_destination && i_to_e_1->is_valid){ //|| (!has_destination && (wb_i.is_float && !is_fmv_fcmp_fcvt && !wb_i.is_store_fp))){
    if (is_fmv_fcmp_fcvt                                     ||
       (!i_from_d.decoded_instruction.is_branch && !i_from_d.decoded_instruction.is_store && !i_from_d.decoded_instruction.is_float) ||
       ( i_from_d.decoded_instruction.is_system &&  i_from_d.decoded_instruction.func3 == CSRRS)){
       *i_type = I_TYPE_NO_FP;
    }
    if (write_to_freg){ //(i_from_d.decoded_instruction.is_float && !is_fmv_fcmp_fcvt && !i_from_d.decoded_instruction.is_store_fp){
    	*i_type = I_TYPE_FP;
    }
    if (i_from_d.decoded_instruction.is_system && (i_from_d.decoded_instruction.func3 == CSRRW)){
       *i_type = I_TYPE_NO_FP;
    }
  }
  //if(i_from_d.decoded_instruction.is_float && i_to_e_1->is_valid)
  //  *i_type = I_TYPE_FP;
  //else if (!i_from_d.decoded_instruction.is_float && i_to_e_1->is_valid)
  //  *i_type = I_TYPE_NO_FP;
  else{
	  *i_type = I_TYPE_NO_FP; //TODO: ??? -> results in zero register being locked, because *i_destination = 0 then; xD useless or harmful?
  }
#endif

#ifndef __SYNTHESIS__
#ifdef DEBUG_LOCK_REG_MINIMAL
  printf("i_from_d.instruction: %08x\n", i_from_d.instruction);
  printf("i_from_d.decoded_instruction.has_no_dest: %d\n", i_from_d.decoded_instruction.has_no_dest);
  printf("has_destination: %d\n", has_destination);  
  printf("*i_destination : %d\n", *i_destination );
  printf("i_from_d.decoded_instruction.rd: %d\n", i_from_d.decoded_instruction.rd);

  if(wait_for_source){
	printf("WAIT FOR SOURCE REGISTERS!\n");
	if(is_locked_1){
	     printf("is_locked_1 %d, lock_reg[%d]=%d, lock_freg[%d]=%d\n", is_locked_1, i_from_d.decoded_instruction.rs1, is_reg_computed[i_from_d.decoded_instruction.rs1], i_from_d.decoded_instruction.rs1, is_freg_computed[i_from_d.decoded_instruction.rs1]);
	}
	if(is_locked_2){
		  printf("is_locked_2 %d, lock_reg[%d]=%d, lock_freg[%d]=%d\n", is_locked_2, i_from_d.decoded_instruction.rs2, is_reg_computed[i_from_d.decoded_instruction.rs2], i_from_d.decoded_instruction.rs2, is_freg_computed[i_from_d.decoded_instruction.rs2]);
	}
	if(is_locked_d){
		  printf("is_locked_d %d, lock_reg[%d]=%d, lock_freg[%d]=%d\n", is_locked_d, i_from_d.decoded_instruction.rd, is_reg_computed[i_from_d.decoded_instruction.rd], i_from_d.decoded_instruction.rd, is_freg_computed[i_from_d.decoded_instruction.rd]);
	}
	if(is_locked_fcsr)
		printf("WAIT FOR FCSR\n");
  }
#endif
#endif

  if (save_input){

#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("ISSUE: get_input()\n");
#endif
#endif
    get_input(i_from_d, i_state);

  }
  if (i_to_e_1->is_valid){
#ifdef AD
	stage_job( *i_state,
			   reg_file_s,
			   &rv1,
			   &rv2,
			   &rv3,
			   ad_rv1,
			   ad_rv2,
			   ad_rv3
			   );
#else
	stage_job(*i_state, reg_file,
			#ifdef FPU
    		freg_file,
			#endif
			&rv1, &rv2
    		#ifdef FPU
    		,&rv3
    		#endif
			);
#endif
    i_state->decoded_instruction.is_null_rv2           = (rv2.i == 0);
    i_state->decoded_instruction.is_first_negative_rv2 = (rv2.i == -1);
    i_state->decoded_instruction.is_last_negative_rv1  = (rv1.i == 0x80000000);
#ifndef __SYNTHESIS__
#ifdef DEBUG_PIPELINE
    if (!wait_for_source){
      printf("issued   ");
      printf("%04d\n", (int)i_state->pc);
    }
#endif
#endif
    set_output(rv1,
               rv2,
#ifdef FPU
			   rv3,
#ifdef AD
			   ad_rv1,
			   ad_rv2,
			   ad_rv3,
			   reg_file_s->fcsr, //pass value
#else
			  *fcsr,		   //pass value, not pointer
#endif
#endif
              *i_state,
               i_to_e_1);
#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
	printf("ISSUE: set_output()\n");
#endif
#endif
               
  }

  *i_state_is_full = valid_or_full && !i_to_e_1->is_valid;
  i_state->is_full = *i_state_is_full;

#ifndef __SYNTHESIS__
#ifdef DEBUG_ADHOC
    //printf("f_state->is_full: %d\n",(int)f_state->is_full); //always 0
    //printf("d_state_is_full: %d\n",(int)d_state_is_full); //always 0
  	//==============NEXT 4 ALL THE SAME=================================
	//printf("valid_input: %d\n",(int)valid_input);
	//printf("valid_or_full: %d\n",(int)valid_or_full);
	//printf("save_input: %d\n",(int)save_input);
	printf("i_state->is_full : %d\n",(int)i_state->is_full );
    printf("i_to_e_1->is_valid : %d\n",(int)i_to_e_1->is_valid );
#endif
#endif
}