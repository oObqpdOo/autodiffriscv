#include "ap_int.h"
#include "adrv32imf_mp_ip.h"

#ifdef USE_HLS_MATH
#include "hls_math.h" //for fmadd
#else
#include "math.h"
#endif

//THE FOLLOWING TRIED TO USE ONLY FLOAT_INT_T, to decrease II, but it didn't work :(
#ifdef FLOAT_INT_T_IN_COMPUTE
static void vaddsub(float_int_t result[AD_DERIV_CNT], float_int_t a[AD_DERIV_CNT], int b[AD_DERIV_CNT], bit_t b_sub){
#pragma HLS INLINE
//#pragma HLS ARRAY_PARTITION variable=result dim=1 complete
	max_der_t i;
//#pragma HLS PIPELINE II=1
//#pragma HLS LATENCY max=1
	for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
		if(b_sub == true){
			result[i].i = a[i].i - b[i];
		}
		else
			result[i].i = a[i].i + b[i];
#ifdef DEBUG_OPS
#ifndef __SYNTHESIS__
		printf("vaddsub: a = %d; b = %d; res = %d\n", a[i].i, b[i].i, result[i].i);
#endif
#endif
	}
}

//vsll(ad_result1, ad_rv1, temp_shift, rv1, shift);
static void vsll(float_int_t result[AD_DERIV_CNT], float_int_t ad_b [AD_DERIV_CNT], int ad_shift[AD_DERIV_CNT], float_int_t b, ap_uint<5> c){
#pragma HLS INLINE
	max_der_t i;
	for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
//#pragma HLS PIPELINE II=6
//#pragma HLS LATENCY max=6
		//a = b << c = b * 2 ^ c
		//da = (db << c) + ((b * ln(2) * dc) << c)

		//normal variant
		//int temp = (fixp40) b * ln2 * ad_shift[i];

		//int temp = int((float)0.6931471805599453094172321214581 * float(ad_shift[i]) * float(b));

		//AD_Rounded Version to ease up things:
		//int temp = (b << 2) * ad_shift[i];

		fixp64 temp = (fixp64) b.i * (fixp64) ln2 * (fixp64) ad_shift[i];
		//int temp = (float) ln2 * ad_shift[i] * b;
		result[i].i = (ad_b[i].i << c) + (temp << c);
	}
}

//vsra(ad_result1, ad_rv1, temp_shift, rv1, shift, true);
static void vsra(float_int_t result[AD_DERIV_CNT], float_int_t ad_b [AD_DERIV_CNT], int ad_shift[AD_DERIV_CNT], float_int_t b, ap_uint<5> c, bit_t sign){
#pragma HLS INLINE
//#pragma HLS ARRAY_PARTITION variable=result dim=1 complete
	max_der_t i;
	for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
//#pragma HLS PIPELINE II=1
//#pragma HLS LATENCY max=1
		//a = b >> c = b / 2 ^ c
		//da = (db >> c) - ((b * ln(2) * dc) >> c)
		/*int temp;
		if(sign == true)
			temp = (fixp40) b * ln2 * ad_shift[i];
		else
			temp = (ufixp40) b * ln2 * ad_shift[i];*/

		int temp;
		if(sign == true)
			temp = (fixp64) b.i * (ufixp64) ln2 * (fixp64) ad_shift[i];
		else
			temp = (ufixp64) b.i * (ufixp64) ln2 * (fixp64) ad_shift[i];

		/*float temp = (float) ln2 * ad_shift[i];
		if(sign == true)
			temp = temp * (int) b;
		else
			temp = temp * (unsigned int) b;*/
		//AD_Rounded Version to ease up things:
		/*if(sign == true)
			temp = ((unsigned int) b << 1) * ad_shift[i];
		else
			temp = (b << 1) * ad_shift[i];*/
		result[i].i = (ad_b[i].i >> c) - (temp >> c);
	}
}


//TODO: CAREFULLY; WE PASS "decoded_instruction_t d_i" values directly, NOT a pointer, so changes wont reach execute1, we have to return the result!
//TODO: IS THERE A BETTER WAY??
bit_t compute_ad_op_result(
  float_int_t rv1,
  float_int_t right,
  int fcsr,
  float_int_t ad_rv1[AD_DERIV_CNT],
  float_int_t ad_rv2[AD_DERIV_CNT],
  float_int_t* result,
  float_int_t ad_result1[AD_DERIV_CNT],
  decoded_instruction_t d_i){
#pragma HLS INLINE
  bit_t f7_6   = d_i.func7>>5;
  ap_uint<5> shift;
  int rv2;
  int temp_rv2[AD_DERIV_CNT];
  int temp_shift[AD_DERIV_CNT];
  int null_v = 0;
  if (d_i.is_r_type){
    rv2   = right.i;
    copy_float_int_t_array_to_int(temp_rv2, ad_rv2, AD_DERIV_CNT); //is this instantaneos?
    shift = rv2;
    copy_float_int_t_array_to_int(temp_shift, ad_rv2, AD_DERIV_CNT);
#ifdef DEBUG_OPS
#ifndef __SYNTHESIS__
	printf("Execute: IT'S A NORMAL REGISTER OP\n");
#endif
#endif
  }
  else{//I_TYPE
	//TODO: CHECK IF (int *) &shift typecast is OK
    rv2   = (int)d_i.imm;
    //if immediate: e.g. addi a1, 1 -> a1 = a1 + 1;  derivative a1 = derivative a1 * 1 + 0
    copy_value_32bit(temp_rv2, &null_v, AD_DERIV_CNT);
    shift = ((ap_uint<5>)d_i.inst_24_21)<<1 |
             (ap_uint<5>)d_i.inst_20;
    copy_value_32bit(temp_shift, (int *) &null_v, AD_DERIV_CNT);
#ifdef DEBUG_OPS
#ifndef __SYNTHESIS__
	printf("Execute: IT'S AN IMMEDIATE OP\n");
#endif
#endif
  }
#ifdef DEBUG_AD
#ifndef __SYNTHESIS__
  printf("AD_DEBUG: HI, COMPUTE-STAGE IS_AD_OP = %d\n", (unsigned int)d_i.is_ad_op);
  printf("rv_1 = %x; rv_2 = %x;\n", rv1, rv2);
  for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
	printf("ad_rv_1 = %x; ad_rv_2 = %x;\n", ad_rv1[i].i, ad_rv2[i].i);
  }
#endif
#endif
  if(!d_i.is_system){
    switch(d_i.func3){
	  case ADD :
		  if (d_i.is_r_type && f7_6){
#ifdef DEBUG_OPS
#ifndef __SYNTHESIS__
			printf("IT'S A BUGGY SUB\n");
#endif
#endif
			(*result).i = rv1.i - rv2;//SUB
			vaddsub(ad_result1, ad_rv1, temp_rv2, true);
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
		   }
		   else{
			 (*result).i = rv1.i + rv2;
			 vaddsub(ad_result1, ad_rv1, temp_rv2, false);
			 d_i.is_ad_op = true;		//get ready to overwrite AD_Registers in WB
		   }
		   break;
	  case SLL :
			(*result).i = rv1.i << shift;
			//a = b << c = b * 2 ^ c
			vsll(ad_result1, ad_rv1, temp_shift, rv1, shift);
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
			//da = (db << c) + ((b * ln(2) * dc) << c)
			break;
	  case SLT :
			(*result).i = rv1.i < rv2;
			//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
			#pragma HLS UNROLL
			ad_result1[i].i = ad_rv1[i].i < (int)d_i.imm;;
			}
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
#endif
			break;
	  case SLTU:
			(*result).i = (unsigned int)rv1.i < (unsigned int)rv2;
			//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
				#pragma HLS UNROLL
				ad_result1[i].i = (unsigned int)ad_rv1[i].i < (unsigned int)d_i.imm;;
			}
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
#endif
		   break;
	  case XOR :
		  (*result).i = rv1.i ^ rv2;
		 //TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
     	 for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
			#pragma HLS UNROLL
			ad_result1[i].i = ad_rv1[i].i < (int)d_i.imm;;
		 }
		 d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
#endif
		 break;
	  case SRL :
		   if (f7_6){
				(*result).i = rv1.i >> shift;//SRA
				vsra(ad_result1, ad_rv1, temp_shift, rv1, shift, true);
		   }
		   else{
				(*result).i = (unsigned int)rv1.i >> shift;
				//No Idea yet ... is this used for calculations?
				//pass false for sign and use "rv1" value as unsigned (cast to (Ufixp64) instead of (fixp64))
				vsra(ad_result1, ad_rv1, temp_shift, rv1, shift, false);
		   }
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
		   //left out so far
		   break;
	  case OR  :
		  	(*result).i = rv1.i | rv2;
			//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
				#pragma HLS UNROLL
				ad_result1[i].i = ad_rv1[i].i | (int)d_i.imm;;
			}
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
#endif
		   break;
	  case AND :
		    (*result).i = rv1.i & rv2;
			//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
				#pragma HLS UNROLL
				ad_result1[i].i = ad_rv1[i].i & (int)d_i.imm;;
			}
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
#endif
		   break;
    }
  }
  else{
    if (d_i.func3 == CSRRW) *result = rv1;
    if (d_i.func3 == CSRRS) (*result).i = fcsr;
  }
  return d_i.is_ad_op;
}
#endif


#ifdef AD
static void vaddsub(int result[AD_DERIV_CNT], int a[AD_DERIV_CNT], int b[AD_DERIV_CNT], bit_t b_sub){
#pragma HLS INLINE
//#pragma HLS ARRAY_PARTITION variable=result dim=1 complete
	max_der_t i;
//#pragma HLS PIPELINE II=1
//#pragma HLS LATENCY max=1
	for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
		if(b_sub == true){
			result[i] = a[i] - b[i];
		}
		else
			result[i] = a[i] + b[i];
#ifdef DEBUG_OPS
#ifndef __SYNTHESIS__
		printf("vaddsub: a = %d; b = %d; res = %d\n", a[i], b[i], result[i]);
#endif
#endif
	}
}

static void vsll(int result[AD_DERIV_CNT], int ad_b [AD_DERIV_CNT], int ad_shift[AD_DERIV_CNT], int b, ap_uint<5> c){
#pragma HLS INLINE
	max_der_t i;
	for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
//#pragma HLS PIPELINE II=6
//#pragma HLS LATENCY max=6
		//a = b << c = b * 2 ^ c
		//da = (db << c) + ((b * ln(2) * dc) << c)

		//normal variant (with fixpoint arithmetics)
		//int temp = (fixp40) b * ln2 * ad_shift[i];
		//More precise version
		//int temp = int((float)0.6931471805599453094172321214581 * float(ad_shift[i]) * float(b));
		//AD_Rounded Version to ease up things:
		//int temp = (b << 2) * ad_shift[i];
		//===========================================================
		//USING EITHER ROUNDED OR FIXPOINT 64 BIT ARITHMETICS
#ifdef USE_INACCURATE_AD_FOR_SHIFTS
		int temp = b * ad_shift[i];
#else
#ifdef USE_HLS_MATH
		fixp64 temp = (fixp64) b * (fixp64) my_ln2 * (fixp64) ad_shift[i];
#else
		fixp64 temp = (fixp64) b * (fixp64) ln2 * (fixp64) ad_shift[i];
#endif
#endif
		//int temp = (float) ln2 * ad_shift[i] * b;
		result[i] = (ad_b[i] << c) + (temp << c);
	}
}

static void vsra(int result[AD_DERIV_CNT], int ad_b [AD_DERIV_CNT], int ad_shift[AD_DERIV_CNT], int b, ap_uint<5> c, bit_t sign){
#pragma HLS INLINE
//#pragma HLS ARRAY_PARTITION variable=result dim=1 complete
	max_der_t i;
	for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
//#pragma HLS PIPELINE II=1
//#pragma HLS LATENCY max=1
		//a = b >> c = b / 2 ^ c
		//da = (db >> c) - ((b * ln(2) * dc) >> c)
		//USING FIXED POINT 40 BIT ARITHMETICS
		/*int temp;
		if(sign == true)
			temp = (fixp40) b * ln2 * ad_shift[i];
		else
			temp = (ufixp40) b * ln2 * ad_shift[i];*/
		//USING FLOATING POINT ARITHMETICS
		/*float temp = (float) ln2 * ad_shift[i];
		if(sign == true)
			temp = temp * (int) b;
		else
			temp = temp * (unsigned int) b;*/
		//USING ROUNDED Version to ease up things:
		/*if(sign == true)
			temp = ((unsigned int) b << 1) * ad_shift[i];
		else
			temp = (b << 1) * ad_shift[i];*/
		//===========================================================
		//USING EITHER ROUNDED OR FIXPOINT 64 BIT ARITHMETICS
		int temp;
		if(sign == true){
#ifdef USE_INACCURATE_AD_FOR_SHIFTS
			temp = ((unsigned int) b) * ad_shift[i];
#else
#ifdef USE_HLS_MATH
			temp = (fixp64) b * (ufixp64) my_ln2 * (fixp64) ad_shift[i];
#else
			temp = (fixp64) b * (ufixp64) ln2 * (fixp64) ad_shift[i];
#endif
#endif
		}
		else{
#ifdef USE_INACCURATE_AD_FOR_SHIFTS
			temp = (b) * ad_shift[i];

#else
#ifdef USE_HLS_MATH
			temp = (ufixp64) b * (ufixp64) my_ln2 * (fixp64) ad_shift[i];
#else
			temp = (ufixp64) b * (ufixp64) ln2 * (fixp64) ad_shift[i];
#endif
#endif
		}
		result[i] = (ad_b[i] >> c) - (temp >> c);
	}
}


//TODO: CAREFULLY; WE PASS "decoded_instruction_t d_i" values directly, NOT a pointer, so changes wont reach execute1, we have to return the result!
//TODO: IS THERE A BETTER WAY??
bit_t compute_ad_op_result(
  int rv1,
  int right,
  int fcsr,
  int ad_rv1[AD_DERIV_CNT],
  int ad_rv2[AD_DERIV_CNT],
  int* result,
  int ad_result1[AD_DERIV_CNT],
  decoded_instruction_t d_i){
#pragma HLS INLINE
  bit_t f7_6   = d_i.func7>>5;
  ap_uint<5> shift;
  int rv2;
  int temp_rv2[AD_DERIV_CNT];
  int temp_shift[AD_DERIV_CNT];
  int null_v = 0;
  if (d_i.is_r_type){
    rv2   = right;
    copy_array_32bit(temp_rv2, ad_rv2, AD_DERIV_CNT); //is this instantaneos?
    shift = rv2;
    copy_array_32bit(temp_shift, ad_rv2, AD_DERIV_CNT);
#ifdef DEBUG_OPS
#ifndef __SYNTHESIS__
	printf("Execute: IT'S A NORMAL REGISTER OP\n");
#endif
#endif
  }
  else{//I_TYPE
	//TODO: CHECK IF (int *) &shift typecast is OK
    rv2   = (int)d_i.imm;
    //if immediate: e.g. addi a1, 1 -> a1 = a1 + 1;  derivative a1 = derivative a1 * 1 + 0
    copy_value_32bit(temp_rv2, &null_v, AD_DERIV_CNT);
    shift = ((ap_uint<5>)d_i.inst_24_21)<<1 |
             (ap_uint<5>)d_i.inst_20;
    copy_value_32bit(temp_shift, (int *) &null_v, AD_DERIV_CNT);
#ifdef DEBUG_OPS
#ifndef __SYNTHESIS__
	printf("Execute: IT'S AN IMMEDIATE OP\n");
#endif
#endif
  }
  #ifdef DEBUG_AD
  #ifndef __SYNTHESIS__
  printf("AD_DEBUG: HI, COMPUTE-STAGE IS_AD_OP = %d\n", (unsigned int)d_i.is_ad_op);
  printf("rv_1 = %x; rv_2 = %x;\n", rv1, rv2);
  for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
	printf("ad_rv_1 = %x; ad_rv_2 = %x;\n", ad_rv1[i], ad_rv2[i]);
  }
  #endif
  #endif
  if(!d_i.is_system){
    switch(d_i.func3){
	  case ADD :
		  if (d_i.is_r_type && f7_6){
#ifdef DEBUG_OPS
#ifndef __SYNTHESIS__
				printf("IT'S A BUGGY SUB\n");
#endif
#endif
			 *result = rv1 - rv2;//SUB
			 vaddsub(ad_result1, ad_rv1, temp_rv2, true);
			 d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
		   }
		   else{
			 *result = rv1 + rv2;
			 vaddsub(ad_result1, ad_rv1, temp_rv2, false);
			 d_i.is_ad_op = true;		//get ready to overwrite AD_Registers in WB
		   }
		   break;
	  case SLL :
			*result = rv1 << shift;
			//a = b << c = b * 2 ^ c
			vsll(ad_result1, ad_rv1, temp_shift, rv1, shift);
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
			//da = (db << c) + ((b * ln(2) * dc) << c)
			break;
	  case SLT :
			*result = rv1 < rv2;
			//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
			#pragma HLS UNROLL
			ad_result1[i] = ad_rv1[i] < (int)d_i.imm;;
			}
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
#endif
			break;
	  case SLTU:
			*result = (unsigned int)rv1 < (unsigned int)rv2;
			//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
				#pragma HLS UNROLL
				ad_result1[i] = (unsigned int)ad_rv1[i] < (unsigned int)d_i.imm;;
			}
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
#endif
		   break;
	  case XOR :
		  *result = rv1 ^ rv2;
		 //TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
     	 for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
			#pragma HLS UNROLL
			ad_result1[i] = ad_rv1[i] < (int)d_i.imm;;
		 }
		 d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
#endif
		 break;
	  case SRL :
		   if (f7_6){
				*result = rv1 >> shift;//SRA
				vsra(ad_result1, ad_rv1, temp_shift, rv1, shift, true);
		   }
		   else{
				*result = (unsigned int)rv1 >> shift;
				//No Idea yet ... is this used for calculations?
				//pass false for sign and use "rv1" value as unsigned (cast to (Ufixp64) instead of (fixp64))
				vsra(ad_result1, ad_rv1, temp_shift, rv1, shift, false);
		   }
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
		   //left out so far
		   break;
	  case OR  :
		  	*result = rv1 | rv2;
			//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
				#pragma HLS UNROLL
				ad_result1[i] = ad_rv1[i] | (int)d_i.imm;;
			}
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
#endif
		   break;
	  case AND :
		    *result = rv1 & rv2;
			//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
#ifdef DIFFERENTIATE_BITWISE_OPS
			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
				#pragma HLS UNROLL
				ad_result1[i] = ad_rv1[i] & (int)d_i.imm;;
			}
			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
#endif
		   break;
    }
  }
  else{
    if (d_i.func3 == CSRRW) *result = rv1;
    if (d_i.func3 == CSRRS) *result = fcsr;
  }
  return d_i.is_ad_op;
}
#else //no AD
#ifdef FPU
int compute_op_result(
  int       		    rv1,
  int		            right,
  int                   fcsr,
  decoded_instruction_t d_i){
#else //no FPU
int compute_op_result(
  int rv1,
  int right,
  decoded_instruction_t d_i){
#endif
#pragma HLS INLINE
  bit_t f7_6 = d_i.func7>>5;
  ap_uint<5> shift;
  int result, rv2;
  if (d_i.is_r_type){
    rv2   = right;
    shift = rv2;
  }
  else{//I_TYPE
    rv2   = (int)d_i.imm;
    shift = ((ap_uint<5>)d_i.inst_24_21)<<1 |
             (ap_uint<5>)d_i.inst_20;
  }
#ifdef FPU
  if(!d_i.is_system){
#endif
  switch(d_i.func3){
    case ADD : if (d_i.is_r_type && f7_6)
                 result = rv1 - rv2;//SUB
               else
                 result = rv1 + rv2;
               break;
    case SLL : result = rv1 << shift;
               break;
    case SLT : result = rv1 < rv2;
               break;
    case SLTU: result = (unsigned int)rv1 < (unsigned int)rv2;
               break;
    case XOR : result = rv1 ^ rv2 ;
               break;
    case SRL : if (f7_6)
                 result = rv1 >> shift;//SRA
               else
                 result = (unsigned int)rv1 >> shift;
               break;
    case OR  : result = rv1 | rv2;
               break;
    case AND : result = rv1 & rv2;
               break;
  }
#ifdef FPU
  }
  else{
    if (d_i.func3 == CSRRW) result = rv1;
    if (d_i.func3 == CSRRS) result = fcsr;
  }
#endif
  return result;
}
#endif //AD

#ifdef FPU
#ifdef AD
float_int_t compute_fp_ad_op_result(
  float_int_t                  rv1,
  float_int_t                  rv2,
  float_int_t                  rv3,
  float_int_t 				   ad_rv1[AD_DERIV_CNT],
  float_int_t 				   ad_rv2[AD_DERIV_CNT],
  float_int_t 				   ad_rv3[AD_DERIV_CNT],
  //float_int_t				   *result,
  float_int_t				   ad_result[AD_DERIV_CNT],
  decoded_instruction_t        d_i,
  bit_t					       *is_ad_op 
 ){
#pragma HLS INLINE

  float_int_t local_result;
  float_int_t local_ad_result[AD_DERIV_CNT];
#pragma HLS ARRAY_PARTITION variable=local_ad_result type=complete  
#pragma HLS ARRAY_RESHAPE variable=local_ad_result type=complete  
  bit_t local_is_ad_op;

//todo: trying to relax these contraints a little bit does NOT enable pynq-z2 with 2 directions and 1k instr. 2k data memory
//but! FOR ALVEO WE CAN TRY FOR MAXIMUM PERFORMANCE
#ifdef HLS_USE_BIND_OP
#pragma HLS BIND_OP variable=local_result op=fadd   impl=primitivedsp latency=2  //latency=2
#pragma HLS BIND_OP variable=local_result op=fsub   impl=primitivedsp latency=2  //latency=2
#pragma HLS BIND_OP variable=local_result op=fdiv   impl=fabric       latency=4  //latency=4
#pragma HLS BIND_OP variable=local_result op=fexp   impl=meddsp       latency=2  //latency=2
#pragma HLS BIND_OP variable=local_result op=flog   impl=meddsp       latency=2  //latency=2
#pragma HLS BIND_OP variable=local_result op=fmul   impl=primitivedsp latency=2  //latency=2
#pragma HLS BIND_OP variable=local_result op=fsqrt  impl=fabric       latency=2  //latency=2
#pragma HLS BIND_OP variable=local_result op=frsqrt impl=fulldsp      latency=2  //latency=2
#pragma HLS BIND_OP variable=local_result op=frecip impl=fulldsp      latency=2  //latency=2
#endif //HLS_USE_BIND_OP
  ap_uint<2>  opch;
  ap_uint<3>  opcl;
  ap_uint<2>  inst_3_2;
  reg_num_t   rs2;
  bit_t       rv1_is_infinity;
  bit_t       rv2_is_infinity;
  bit_t       rv1_is_zero;
  bit_t       rv2_is_zero;
  float temp_fconst_1;
  float temp_fconst_2;
  rv1_is_infinity = (rv1.i == PLUS_INFINITY) || (rv1.i == MINUS_INFINITY);
  rv2_is_infinity = (rv2.i == PLUS_INFINITY) || (rv2.i == MINUS_INFINITY);
  rv1_is_zero     = (rv1.i == POSITIVE_ZERO) || (rv1.i == NEGATIVE_ZERO);
  rv2_is_zero     = (rv2.i == POSITIVE_ZERO) || (rv2.i == NEGATIVE_ZERO);
  inst_3_2        = d_i.opcode;
  opch            = d_i.opcode >> 3;
  opcl            = d_i.opcode;
  rs2             = d_i.rs2;
  max_der_t i;
  if (d_i.is_op_fp){
    if ((!d_i.is_fcmp) && (rv1.i == QNAN) && (rv2.i == QNAN))
      local_result.i = QNAN;
    else{
      //RNE rounding mode only (other rounding modes not yet implemented)
      switch(d_i.func5h){
        case 0://FADD|FSUB|FMUL|FDIV
          switch(d_i.func5l){
//==========================================
//FADD
//==========================================
          case FADD:
              if ((rv1.i == PLUS_INFINITY && rv2.i == MINUS_INFINITY) ||
                  (rv2.i == PLUS_INFINITY && rv1.i == MINUS_INFINITY)){
                local_result.i = QNAN;
              }
              else{
                local_result.f = rv1.f + rv2.f ;
              }
              //================================
      		  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
			    if ((ad_rv1[i].i == PLUS_INFINITY && ad_rv2[i].i == MINUS_INFINITY) ||
				    (ad_rv2[i].i == PLUS_INFINITY && ad_rv1[i].i == MINUS_INFINITY)){
			  	  local_ad_result[i].i = QNAN;
			    }
			    else{
	      	      local_ad_result[i].f = ad_rv1[i].f + ad_rv2[i].f ;
			    }
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
      			printf("ad_fadd: a = %f; b = %f; res = %f\n", ad_rv1[i].f, ad_rv2[i].f, local_ad_result[i].f);
#endif
#endif
      		  }
              break;
//==========================================
//FSUB
//==========================================
            case FSUB:
              if (rv1_is_infinity && rv2_is_infinity){
                local_result.i = QNAN;
              }
              else{
                local_result.f = rv1.f - rv2.f;
              }
              //================================
      		  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
			    if (
			      (ad_rv1[i].i == PLUS_INFINITY && ad_rv2[i].i == MINUS_INFINITY) ||
				  (ad_rv2[i].i == PLUS_INFINITY && ad_rv1[i].i == MINUS_INFINITY) ||
				  (ad_rv2[i].i == MINUS_INFINITY && ad_rv1[i].i == MINUS_INFINITY) ||
				  (ad_rv2[i].i == PLUS_INFINITY && ad_rv1[i].i == PLUS_INFINITY)
				  ){
			  	  local_ad_result[i].i = QNAN;
			    }
			    else{
	      	      local_ad_result[i].f = ad_rv1[i].f - ad_rv2[i].f ;
			    }
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
      			printf("ad_fsub: a = %f; b = %f; res = %f\n", ad_rv1[i].f, ad_rv2[i].f, local_ad_result[i].f);
#endif
#endif
      		  }
              break;
//==========================================
//FMUL
//==========================================
            case FMUL:
              if ((rv1_is_infinity && rv2_is_infinity) ||
                  (rv1_is_infinity && rv2_is_zero)     ||
                  (rv2_is_infinity && rv1_is_zero)){
                local_result.i = QNAN;
              }
              else{
                local_result.f = rv1.f * rv2.f;
              }
              //TODO: AD with exception handling - check carefully if exception handling is OK
              //================================
      		  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
			    if (
			      //one of the summands is QNAN
			      //(rv1_is_infinity && rv2_is_infinity) but for both summands
			      ((ad_rv1[i].i == PLUS_INFINITY || ad_rv1[i].i == MINUS_INFINITY) && (rv2.i == PLUS_INFINITY || rv2.i == MINUS_INFINITY)) ||
				  ((ad_rv2[i].i == PLUS_INFINITY || ad_rv2[i].i == MINUS_INFINITY) && (rv1.i == PLUS_INFINITY || rv1.i == MINUS_INFINITY)) ||
			      //(rv1_is_infinity && rv2_is_zero) but for both summands
				  ((ad_rv1[i].i == PLUS_INFINITY || ad_rv1[i].i == MINUS_INFINITY) && (rv2.i == POSITIVE_ZERO || rv2.i == NEGATIVE_ZERO)) ||
				  ((ad_rv2[i].i == PLUS_INFINITY || ad_rv2[i].i == MINUS_INFINITY) && (rv1.i == POSITIVE_ZERO || rv1.i == NEGATIVE_ZERO))
				  //(rv2_is_infinity && rv1_is_zero) is the same as above
				  ){
			  	  local_ad_result[i].i = QNAN;
			    }
			    else{
	      	      local_ad_result[i].f = ad_rv1[i].f * rv2.f + ad_rv2[i].f * rv1.f;
			    }
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
      			printf("ad_fsub: a = %f; b = %f; res = %f\n", ad_rv1[i].f, ad_rv2[i].f, local_ad_result[i].f);
#endif
#endif
      		  }
              break;
//==========================================
//FDIV
//==========================================
            case FDIV:
    		  //TODO: what if just rv2 is ZERO???
              if((rv1_is_infinity && rv2_is_infinity) ||
                 (rv1_is_zero    && rv2_is_zero)){
                local_result.i = QNAN;
              }
              else{
                local_result.f = rv1.f / rv2.f;
              }
              //TODO: AD with exception handling
              //================================
	          //TODO: rename temp_fconst_2 to temp_fconst_1 and 1 to 2
              temp_fconst_1 = (rv1.f / (rv2.f * rv2.f));
              temp_fconst_2 = (1 / rv2.f);
      		  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
			    if (
			      //TODO: horrible exception handling... is this correct???
			      //one of the summands is QNAN
				  //======
				  //INF * INF but for both summands
			      // because: 1 / 0 = INF)
			      //INF * INF + x * INF
			      ((ad_rv1[i].i == PLUS_INFINITY || ad_rv1[i].i == MINUS_INFINITY) && (rv2.f == POSITIVE_ZERO || rv2.f == NEGATIVE_ZERO)) ||
				  //x * 0 + INF * INF
				  ((ad_rv2[i].i == PLUS_INFINITY || ad_rv2[i].i == MINUS_INFINITY) && (rv2.f == POSITIVE_ZERO || rv2.f == NEGATIVE_ZERO)) ||
			      //======
				  //INF * 0  but for both summands
				  // because: x / INF  = 0.0
			      //INF * 0 + x * 0
				  ((ad_rv1[i].i == PLUS_INFINITY || ad_rv1[i].i == MINUS_INFINITY) && (rv2.i == PLUS_INFINITY || rv2.i == MINUS_INFINITY)) ||
				  // x * 0 + INF * 0
				  ((ad_rv2[i].i == PLUS_INFINITY || ad_rv2[i].i == MINUS_INFINITY) && (rv2.i == PLUS_INFINITY || rv2.i == MINUS_INFINITY)) ||
				  //======
				  //0 * INF  but for both summands
				  //0 * INF + x * INF
				  ((ad_rv1[i].i == POSITIVE_ZERO || ad_rv1[i].i == NEGATIVE_ZERO) && (rv2.f == POSITIVE_ZERO || rv2.f == NEGATIVE_ZERO)) ||
				  //x * INF + 0 * INF
				  ((ad_rv2[i].i == POSITIVE_ZERO || ad_rv2[i].i == NEGATIVE_ZERO) && (rv2.f == POSITIVE_ZERO || rv2.f == NEGATIVE_ZERO)) ||
				  //======
				  //(0 / 0) but for both summands
				  ((rv1.i == POSITIVE_ZERO || rv1.i == NEGATIVE_ZERO) && (rv2.i == POSITIVE_ZERO || rv2.i == NEGATIVE_ZERO)) ||
				  //======
				  //INF / INF
				  ((rv1.i == PLUS_INFINITY || rv1.i == MINUS_INFINITY) && (rv2.i == PLUS_INFINITY || rv2.i == MINUS_INFINITY))
				  ){
			  	  local_ad_result[i].i = QNAN;
			    }
			    else{
		          //TODO: rename temp_fconst_2 to temp_fconst_1 and 1 to 2
#ifdef ROUNDING_MODES
			      //TODO:
			      local_ad_result[i].f = (ad_rv1[i].f * temp_fconst_2) + (ad_rv2[i].f * temp_fconst_1);
#else
			      local_ad_result[i].f = (ad_rv1[i].f * temp_fconst_2) + (ad_rv2[i].f * temp_fconst_1);
#endif
			    }
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
      			printf("ad_fsub: a = %f; b = %f; res = %f\n", ad_rv1[i].f, ad_rv2[i].f, local_ad_result[i].f);
#endif
#endif
      		  }
              break;
          }
    	  local_is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
          break;
//==========================================
//(FSGN|FMIN_MAX)
//==========================================
        case 1://FSGN|FMIN_MAX
          if (d_i.func5l){//FMIN_MAX
            if (rv1.i == QNAN || rv2.i == QNAN)
              local_result.f = (rv1.i == QNAN)? rv2.f : rv1.f;
            else if (rv1.i == NEGATIVE_ZERO  && rv2.i == POSITIVE_ZERO)
              local_result.f = (d_i.func3 == 1)? rv2.f : rv1.f;
            else if (rv1.i == POSITIVE_ZERO  && rv2.i == NEGATIVE_ZERO)
              local_result.f = (d_i.func3 == 1)? rv1.f : rv2.f;
            else{
              if (d_i.func3 == 1)
                local_result.f = (rv1.f > rv2.f)? rv1.f : rv2.f;
              else
                local_result.f = (rv1.f < rv2.f)? rv1.f : rv2.f;
            }
      	    local_is_ad_op = false;
          }
          else{ //FSGN
            if (d_i.func3 == JN){
              local_result.i = (~rv2.part.sign)    << 31 |
                           rv1.part.exponent << 23 |
                           rv1.part.mantissa;
        	  local_is_ad_op = false;
            }
            else if (d_i.func3 == JX){
              local_result.i = ((rv1.part.sign) ^ (rv2.part.sign))   << 31 |
                                             rv1.part.exponent << 23 |
                                             rv1.part.mantissa;
        	  local_is_ad_op = false;
            }
            else{ //THIS WOULD BE USED FOR FMOV.S PSEUDO-INSTRUCTION!!!
              local_result.i = (rv2.part.sign)    << 31 |
                          rv1.part.exponent << 23 |
                          rv1.part.mantissa;
              //================================
              if(d_i.rs2 == d_i.rs1){ //ONLY APPLY AD WHEN IT IS A FMOV instruction!
            	for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
     		      local_ad_result[i].i = (ad_rv2[i].part.sign) << 31 | (ad_rv1[i].part.exponent) << 23 | (ad_rv1[i].part.mantissa);
    		    }
            	local_is_ad_op = true; //IMPORTANT FOR FMOV.S!!!
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
      			printf("ad_fsgnj.s: a = %f; b = %d; res = %f\n", ad_rv1[i].f, ad_rv2[i].i, local_ad_result[i].f);
#endif
#endif
      		  }
              else{
                local_is_ad_op = false; //NO AD WB OTHERWISE
              }
        	}
          }
          break;
//==========================================
//FSQRT
//==========================================
        case 2://FSQRT
    	  if (rv1.part.sign == 1 || rv1.i == QNAN){
    	    local_result.i = QNAN;
    	  }
    	  else{
#ifdef USE_HLS_MATH
    		  local_result.f = hls::sqrt(rv1.f);
#else
    		  local_result.f = sqrt(rv1.f);
#endif
    	  }
          //================================
#ifdef USE_HLS_MATH
    	  temp_fconst_1 = (1 / (2 * hls::sqrt(rv1.f)));
#else
    	  temp_fconst_1 = (1 / (2 * sqrt(rv1.f)));
#endif
  		  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
  			//TODO: This check is wrong!
		    //if (ad_rv1[i].part.sign == 1 || rv1.i == QNAN || ad_rv1[i].i == QNAN ){
  			//TODO: is the following check better?
  			if (rv1.part.sign == 1 		||
  				rv1.i == QNAN 			||
				ad_rv1[i].f == QNAN		||
				//Multiplication problems:
				// INF * INF - not possible (1/INF = 0)
				// 0 * INF - IS possible
				// INF * 0 - not possible (1/INF = 0)
				(rv1_is_infinity && (ad_rv1[i].f == PLUS_INFINITY || ad_rv1[i].f == MINUS_INFINITY))){
                local_ad_result[i].i = QNAN;
		    }
		    else{
		      local_ad_result[i].f = temp_fconst_1 * ad_rv1[i].f;
		    }
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
  			printf("ad_fsqrt: a = %f; b = %d; res = %f\n", ad_rv1[i].f, ad_rv2[i].i, local_ad_result[i].f);
#endif
#endif
  		  }
  		  local_is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
          break;
//==========================================
//(FCMP)
//==========================================
        case 5://FCMP
          if (rv1.i == QNAN || rv2.i == QNAN)
            local_result.i = 0;
          else{
            if (d_i.func3 == FLT)
              local_result.i = (rv1.f < rv2.f);
            else if (d_i.func3 == FLE)
              local_result.i = (rv1.f <= rv2.f);
            else
              local_result.i = (rv1.f == rv2.f);
          }
    	  local_is_ad_op = false; 		//get ready to overwrite AD_Registers in WB
          break;
//==========================================
//FCVT
//==========================================
        case 6://FCVT
          if (d_i.func5l == 0){//FCVT_W
            if (rs2 == 1){//FCVT_WU_S
              if (rv1.i == MINUS_INFINITY)
                local_result.i = 0;
              else if (rv1.i == QNAN || rv1.i == PLUS_INFINITY)
                local_result.i = MAX_POS_RANGE;
              else
                local_result.i = (unsigned)((int)(rv1.f));
              //================================
      		  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
			    if (ad_rv1[i].i == MINUS_INFINITY)
			      local_ad_result[i].i = 0;
			    else if (ad_rv1[i].i == QNAN || ad_rv1[i].i == PLUS_INFINITY)
			      local_ad_result[i].i = MAX_POS_RANGE;
			    else
			      local_ad_result[i].i = (unsigned)((int)(ad_rv1[i].f));
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
      			printf("ad_fcvt_wu_s: a = %f; b = %d; res = %d\n", ad_rv1[i].f, ad_rv2[i].i, local_ad_result[i].i);
#endif
#endif
      		  }
            }
       	    else{//FCVT_W_S
              if (rv1.i == MINUS_INFINITY)
                local_result.i = MIN_NEG_RANGE;
              else if (rv1.i == QNAN || rv1.i == PLUS_INFINITY)
                local_result.i = MAX_POS_RANGE;
              else
                local_result.i = (int)(rv1.f);
              //================================
      		  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
			    if (ad_rv1[i].i == MINUS_INFINITY)
			      local_ad_result[i].i = MIN_NEG_RANGE;
			    else if (ad_rv1[i].i == QNAN || ad_rv1[i].i == PLUS_INFINITY)
			      local_ad_result[i].i = MAX_POS_RANGE;
			    else
	              local_ad_result[i].i = (int)(ad_rv1[i].f);
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
      			printf("ad_fcvt_w_s: a = %f; b = %d; res = %d\n", ad_rv1[i].f, ad_rv2[i].i, local_ad_result[i].i);
#endif
#endif
      		  }
            }
          }
          else{//FCVT_S
            if (rs2 == 1){//FCVT_S_WU
              local_result.f = (float)((unsigned)(rv1.i));
              //================================
      		  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
      			local_ad_result[i].f = (float)((unsigned)(ad_rv1[i].i));
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
      			printf("ad_fcvt_s_wu: a = %d; b = %d; res = %f\n", ad_rv1[i].i, ad_rv2[i].i, local_ad_result[i].f);
#endif
#endif
      		  }
            }
            else{//FCVT_S_W
#ifdef USE_OLD_BUG
              local_result.f = (float)(rv1.i); //wont work for negative numbers!!!! will be assumed to be very high positive instead
#else
              //TODO: OR SHOULD I SET FLOAT_INT_T INT PARTITION TO INT INSTEAD OF UNSIGNED??????????
              local_result.f = (float)((int)rv1.i);
#endif
#ifndef __SYNTHESIS__
              printf("FCVT_S_W: int = %d, float = %f\n", (rv1.i), local_result.f);
#endif
              //================================
      		  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
      			local_ad_result[i].f = (float)((int)ad_rv1[i].i);
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
      			printf("ad_fcvt_s_w: a = %d; b = %d; res = %f\n", ad_rv1[i].i, ad_rv2[i].i, local_ad_result[i].f);
#endif
#endif
      		  }
            }
          }
    	  local_is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
          break;
//==========================================
//FMV|(FCLASS) //!!!!! THIS MOVES FROM FREG TO REG OR VICE VERSA; FROM FREG TO FREG DOES FSGNJ
//==========================================
        case 7://FCLASS|FMV
          if (d_i.func5l == 0){//FCLASS|FMV_X_W
            if (d_i.func3 == FCLASS){
    	      if (rv1.i == MINUS_INFINITY)
    	        local_result.i = 0;
    	      else if (rv1.part.sign == 1 && (0 < rv1.part.exponent)  && (rv1.part.exponent < EMAX))
                local_result.i = 1;
              else if (rv1.part.sign == 1 && (rv1.part.exponent == 0) && (rv1.part.mantissa != 0))
                local_result.i = 2;
              else if (rv1.i == NEGATIVE_ZERO)
                local_result.i = 3 ;
              else if (rv1.i == POSITIVE_ZERO)
                local_result.i = 4 ;
              else if (rv1.part.sign == 0 && (0 < rv1.part.exponent)  && (rv1.part.exponent < EMAX))
                local_result.i = 5;
              else if (rv1.part.sign == 1 && (rv1.part.exponent == 0) && (rv1.part.mantissa != 0))
                local_result.i = 6;
              else if (rv1.i == PLUS_INFINITY)
                local_result.i = 7;
              else if (rv1.i == QNAN)
                local_result.i = 9;
        	  local_is_ad_op = false;
            }//FMV_X_W
            else{
			  local_result.i = rv1.i;
			  //================================
			  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
				  local_ad_result[i].i = ad_rv1[i].i;
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
				  printf("ad_fmv_x_w: a = %d; b = %d; res = %d\n", ad_rv1[i].i, ad_rv2[i].i, local_ad_result[i].i);
#endif
#endif
			  }
        	  local_is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
            }
          }
          else{ //FMV_W_X
            local_result.f = rv1.f;
			//================================
			for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
	          local_ad_result[i].f = ad_rv1[i].f;
#ifdef DEBUG_FPU_OPS
#ifndef __SYNTHESIS__
			  printf("ad_fmv_w_x: a = %f; b = %d; res = %f\n", ad_rv1[i].f, ad_rv2[i].i, local_ad_result[i].f);
#endif
#endif
			}
			local_is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
          }
          break;
        default:
      	  local_is_ad_op = false; 		//get ready to overwrite AD_Registers in WB
          break;
      }
    }
  }
//==========================================
//FUSED
//==========================================
  if (d_i.is_fused){
  //TODO: IS THIS EXCEPTION HANDLING HERE ENOUGH?
    if (((rv1_is_infinity) && (rv2_is_zero)) ||
        ((rv2_is_infinity) && (rv1_is_zero)) ||
         (rv3.i == QNAN))
      local_result.i = QNAN; //this will and must set the INVALID flag NV
    else{
      //TODO: this is wrong! they should only round ONCE
      switch(inst_3_2){
        case 0://MADD
#ifdef USE_HLS_MATH
          local_result.f = hls::fma(rv1.f, rv2.f, rv3.f); //#include "hls_math.h"
#else
          local_result.f =  (rv1.f * rv2.f) + rv3.f;
#endif
          break;
        case 1://MSUB
#ifdef USE_HLS_MATH
          local_result.f = hls::fma(rv1.f, rv2.f, -rv3.f); //#include "hls_math.h"
#else
          local_result.f =  (rv1.f * rv2.f) - rv3.f;
#endif
          break;
        case 2://NMSUB
#ifdef USE_HLS_MATH
          local_result.f = hls::fma(-rv1.f, rv2.f, rv3.f); //#include "hls_math.h"
#else
          local_result.f = -(rv1.f * rv2.f) + rv3.f;
#endif
          break;
        case 3://NMADD
#ifdef USE_HLS_MATH
          local_result.f = hls::fma(-rv1.f, rv2.f, -rv3.f); //#include "hls_math.h"
#else
          local_result.f = -(rv1.f * rv2.f) - rv3.f;
#endif
          break;
      }
    }
//================================
	for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
	  float temp1;
	  if (((ad_rv2[i].f == MINUS_INFINITY || ad_rv2[i].f == PLUS_INFINITY) && (rv1_is_zero))     ||
	      ((ad_rv2[i].f == POSITIVE_ZERO  || ad_rv2[i].f == NEGATIVE_ZERO) && (rv1_is_infinity)) ||
		  ((ad_rv1[i].f == MINUS_INFINITY || ad_rv1[i].f == PLUS_INFINITY) && (rv2_is_zero))     ||
		  ((ad_rv1[i].f == POSITIVE_ZERO  || ad_rv1[i].f == NEGATIVE_ZERO) && (rv2_is_infinity)) ||
		   (rv3.i == QNAN)){
	    local_ad_result[i].f = QNAN;
	  }
	  else{
	    switch(inst_3_2){
		  case 0://MADD
#ifdef USE_HLS_MATH
		    temp1 = hls::fma(ad_rv2[i].f, rv1.f, ad_rv3[i].f);
		    if (temp1 == QNAN){
			  local_ad_result[i].f = QNAN;
		    }
		    else
		      local_ad_result[i].f = hls::fma(ad_rv1[i].f, rv2.f, temp1);
#else
	  	    local_ad_result[i].f =  (ad_rv1[i].f * rv2.f) + (ad_rv2[i].f * rv1.f) + ad_rv3[i].f;
#endif
		    break;
		  case 1://MSUB
#ifdef USE_HLS_MATH
		    temp1 = hls::fma(ad_rv2[i].f, rv1.f, -ad_rv3[i].f);
		    if (temp1 == QNAN){
			  local_ad_result[i].f = QNAN;
		    }
		    else
  		      local_ad_result[i].f = hls::fma(ad_rv1[i].f, rv2.f, temp1);
#else
		    local_ad_result[i].f =  (ad_rv1[i].f * rv2.f) + (ad_rv2[i].f * rv1.f) - ad_rv3[i].f;
#endif
		    break;
		  case 2://NMSUB
#ifdef USE_HLS_MATH
		    temp1 = hls::fma(ad_rv2[i].f, rv1.f, -ad_rv3[i].f);
		    if (temp1 == QNAN){
			  local_ad_result[i].f = QNAN;
		    }
		    else
		      local_ad_result[i].f = hls::fma(-ad_rv1[i].f, rv2.f, -temp1);
#else
		    local_ad_result[i].f =  (-ad_rv1[i].f * rv2.f) - (ad_rv2[i].f * rv1.f) - ad_rv3[i].f;
#endif
		    break;
		  case 3://NMADD
#ifdef USE_HLS_MATH
		    temp1 = hls::fma(ad_rv2[i].f, rv1.f, ad_rv3[i].f);
		    if (temp1 == QNAN){
			  local_ad_result[i].f = QNAN;
		    }
		    else
 		      local_ad_result[i].f = hls::fma(-ad_rv1[i].f, rv2.f, -temp1);
#else
		    local_ad_result[i].f =  (-ad_rv1[i].f * rv2.f) - (ad_rv2[i].f * rv1.f) + ad_rv3[i].f;
#endif
		    break;
	    }
	  }
	}
	//TODO: CREATE AD VERSION
    local_is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
  }//if is_fused
  //return d_i.is_ad_op;
  //*result = local_result;
  copy_array_float_int_t(ad_result, local_ad_result, AD_DERIV_CNT);
  //d_i.is_ad_op = local_is_ad_op;
  *is_ad_op = local_is_ad_op;
  return local_result;
}



#else //NO AD

float_int_t compute_fp_op_result(
  float_int_t                  rv1,
  float_int_t                  rv2,
  float_int_t                  rv3,
  decoded_instruction_t d_i){
#pragma HLS INLINE
  float_int_t result;
#pragma HLS BIND_OP variable=result op=fadd   impl=primitivedsp latency=2
#pragma HLS BIND_OP variable=result op=fsub   impl=primitivedsp latency=2
#pragma HLS BIND_OP variable=result op=fdiv   impl=fabric       latency=4
#pragma HLS BIND_OP variable=result op=fexp   impl=meddsp       latency=2
#pragma HLS BIND_OP variable=result op=flog   impl=meddsp       latency=2
#pragma HLS BIND_OP variable=result op=fmul   impl=primitivedsp latency=2
#pragma HLS BIND_OP variable=result op=fsqrt  impl=fabric       latency=2
#pragma HLS BIND_OP variable=result op=frsqrt impl=fulldsp      latency=2
#pragma HLS BIND_OP variable=result op=frecip impl=fulldsp      latency=2
  ap_uint<2>  opch;
  ap_uint<3>  opcl;
  ap_uint<2>  inst_3_2;
  reg_num_t   rs2;
  bit_t       rv1_is_infinity;
  bit_t       rv2_is_infinity;
  bit_t       rv1_is_zero;
  bit_t       rv2_is_zero;
  rv1_is_infinity = (rv1.i == PLUS_INFINITY) || (rv1.i == MINUS_INFINITY);
  rv2_is_infinity = (rv2.i == PLUS_INFINITY) || (rv2.i == MINUS_INFINITY);
  rv1_is_zero     = (rv1.i == POSITIVE_ZERO) || (rv1.i == NEGATIVE_ZERO);
  rv2_is_zero     = (rv2.i == POSITIVE_ZERO) || (rv2.i == NEGATIVE_ZERO);
  inst_3_2        = d_i.opcode;
  opch            = d_i.opcode >> 3;
  opcl            = d_i.opcode;
  rs2             = d_i.rs2;
  if (d_i.is_op_fp){
    if ((!d_i.is_fcmp) && (rv1.i == QNAN) && (rv2.i == QNAN))
      result.i = QNAN;
    else{
      //RNE rounding mode only (other rounding modes not yet implemented)
      switch(d_i.func5h){
        case 0://FADD|FSUB|FMUL|FDIV
          switch(d_i.func5l){
            case FADD:
              if ((rv1.i == PLUS_INFINITY && rv2.i == MINUS_INFINITY) ||
                  (rv2.i == PLUS_INFINITY && rv1.i == MINUS_INFINITY))
                result.i = QNAN;
              else
                result.f = rv1.f + rv2.f ;
             break;
            case FSUB:
              if (rv1_is_infinity && rv2_is_infinity)
                result.i = QNAN;
              else
                result.f = rv1.f - rv2.f;
              break;
            case FMUL:
              if ((rv1_is_infinity && rv2_is_infinity) ||
                  (rv1_is_infinity && rv2_is_zero)     ||
                  (rv2_is_infinity && rv1_is_zero))
                result.i = QNAN;
              else
                result.f = rv1.f * rv2.f;
              break;
            case FDIV:
              if((rv1_is_infinity && rv2_is_infinity) ||
                 (rv1_is_zero    && rv2_is_zero))
                result.i = QNAN;
              else
                result.f = rv1.f / rv2.f;
              break;
          }
          break;
        case 1://FSGN|FMIN_MAX
          if (d_i.func5l){//FMIN_MAX
            if (rv1.i == QNAN || rv2.i == QNAN)
              result.f = (rv1.i == QNAN)? rv2.f : rv1.f;
            else if (rv1.i == NEGATIVE_ZERO  && rv2.i == POSITIVE_ZERO)
              result.f = (d_i.func3 == 1)? rv2.f : rv1.f;
            else if (rv1.i == POSITIVE_ZERO  && rv2.i == NEGATIVE_ZERO)
              result.f = (d_i.func3 == 1)? rv1.f : rv2.f;
            else{
              if (d_i.func3 == 1)
                result.f = (rv1.f > rv2.f)? rv1.f : rv2.f;
              else
                result.f = (rv1.f < rv2.f)? rv1.f : rv2.f;
            }
          }
          else{ //FSGN
            if (d_i.func3 == JN)
              result.i = (~rv2.part.sign)    << 31 |
                           rv1.part.exponent << 23 |
                           rv1.part.mantissa;
            else if (d_i.func3 == JX)
              result.i = ((rv1.part.sign) ^ (rv2.part.sign))   << 31 |
                                             rv1.part.exponent << 23 |
                                             rv1.part.mantissa;
            else
              result.i = (rv2.part.sign)    << 31 |
                          rv1.part.exponent << 23 |
                          rv1.part.mantissa;
          }
          break;
        case 2://FSQRT
    	  if (rv1.part.sign == 1 || rv1.i == QNAN)
    	    result.i = QNAN;
    	  else
    	    result.f = sqrt(rv1.f);
          break;
        case 5://FCMP
          if (rv1.i == QNAN || rv2.i == QNAN)
            result.i = 0;
          else{
            if (d_i.func3 == FLT)
              result.i = (rv1.f < rv2.f);
            else if (d_i.func3 == FLE)
              result.i = (rv1.f <= rv2.f);
            else
              result.i = (rv1.f == rv2.f);
          }
          break;
        case 6://FCVT
          if (d_i.func5l == 0){//FCVT_W
            if (rs2 == 1){//FCVT_WU_S
              if (rv1.i == MINUS_INFINITY)
                result.i = 0;
              else if (rv1.i == QNAN || rv1.i == PLUS_INFINITY)
                result.i = MAX_POS_RANGE;
              else
                result.i = (unsigned)((int)(rv1.f));
            }
       	    else{//FCVT_W_S
              if (rv1.i == MINUS_INFINITY)
                result.i = MIN_NEG_RANGE;
              else if (rv1.i == QNAN || rv1.i == PLUS_INFINITY)
                result.i = MAX_POS_RANGE;
              else
                result.i = (int)(rv1.f);
            }
          }
          else{//FCVT_S
            if (rs2 == 1)//FCVT_S_WU
              result.f = (float)((unsigned)(rv1.i));
            else//FCVT_S_W
              result.f = (float)(rv1.i);
          }
          break;
        case 7://FCLASS|FMV
          if (d_i.func5l == 0){//FCLASS|FMV_X_W
            if (d_i.func3 == FCLASS){
    	      if (rv1.i == MINUS_INFINITY)
    	        result.i = 0;
    	      else if (rv1.part.sign == 1 && (0 < rv1.part.exponent)  && (rv1.part.exponent < EMAX))
                result.i = 1;
              else if (rv1.part.sign == 1 && (rv1.part.exponent == 0) && (rv1.part.mantissa != 0))
                result.i = 2;
              else if (rv1.i == NEGATIVE_ZERO)
                result.i = 3 ;
              else if (rv1.i == POSITIVE_ZERO)
                result.i = 4 ;
              else if (rv1.part.sign == 0 && (0 < rv1.part.exponent)  && (rv1.part.exponent < EMAX))
                result.i = 5;
              else if (rv1.part.sign == 1 && (rv1.part.exponent == 0) && (rv1.part.mantissa != 0))
                result.i = 6;
              else if (rv1.i == PLUS_INFINITY)
                result.i = 7;
              else if (rv1.i == QNAN)
                result.i = 9;
            }//FMV_X_W
            else result.i = rv1.i;
          }
          else//FMV_W_X
            result.f = rv1.f;
          break;
        default:
          break;
      }
    }
  }
  if (d_i.is_fused){
    if (((rv1_is_infinity) && (rv2_is_zero)) ||
        ((rv2_is_infinity) && (rv1_is_zero)) ||
         (rv3.i == QNAN))
      result.i = QNAN;
    else{
      switch(inst_3_2){
        case 0://MADD
          result.f =  (rv1.f * rv2.f) + rv3.f;
          break;
        case 1://MSUB
          result.f =  (rv1.f * rv2.f) - rv3.f;
          break;
        case 2://NMSUB
          result.f = -(rv1.f * rv2.f) + rv3.f;
          break;
        case 3://NMADD
          result.f = -(rv1.f * rv2.f) - rv3.f;
          break;
      }
    }
  }
  return result;
}
#endif //FPU
#endif //AD

bit_t compute_branch_result(
  int rv1,
  int rv2,
  func3_t func3){
#pragma HLS INLINE
  bit_t result;
  switch(func3){
    case BEQ : result = (rv1 == rv2);
               break;
    case BNE : result = (rv1 != rv2);
               break;
    case 2   :
    case 3   : result = 0;
               break;
    case BLT : result = (rv1 <  rv2);
               break;
    case BGE : result = (rv1 >= rv2);
               break;
    case BLTU: result =
               ((unsigned int)rv1 <  (unsigned int)rv2);
               break;
    case BGEU: result =
               ((unsigned int)rv1 >= (unsigned int)rv2);
               break;
  }
  return result;
}

#ifdef AD
int compute_ad_mul_result(
  int 					rv1,
  int 					rv2,
  float_int_t 			ad_rv1[AD_DERIV_CNT],
  float_int_t 			ad_rv2[AD_DERIV_CNT],
  //int					*result,
  float_int_t			ad_result[AD_DERIV_CNT],
  decoded_instruction_t d_i,
  bit_t					*is_ad_op
  ){
#pragma HLS INLINE
  bit_t f7_6          = d_i.func7>>5;
  bit_t f7_0          = d_i.func7;
  ap_uint<2> func3_10 = d_i.func3;

  //It is important to have local copies for result here I think!
  int local_result;
  int local_ad_result[AD_DERIV_CNT];
#pragma HLS ARRAY_PARTITION variable=local_ad_result type=complete  
#pragma HLS ARRAY_RESHAPE variable=local_ad_result type=complete  
  bit_t local_is_ad_op;

#ifdef HLS_USE_BIND_OP
//#pragma HLS BIND_OP variable=local_result op=mul impl=dsp //original
#pragma HLS BIND_OP variable=local_result op=mul impl=dsp latency=0
#endif //HLS_USE_BIND_OP

  long int          rv64,   rv1_64,  rv2_64;
  unsigned long int urv64, urv1_64, urv2_64;
  long int          adrv64,   adrv1_64, adrv2_64;
  unsigned long int adurv64, adurv1_64, adurv2_64;
  max_der_t i;

#ifdef HLS_USE_BIND_OP
//#pragma HLS BIND_OP variable=rv64   op=mul impl=dsp //original
#pragma HLS BIND_OP variable=rv64 op=mul impl=dsp latency=0
#endif //HLS_USE_BIND_OP

  switch(func3_10){
    case MUL    :
    	//================================
    	//NORMAL MUL:
    	local_result = rv1 * rv2;
    	//================================
    	//AD MUL:
		for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
			local_ad_result[i] = ad_rv1[i].i * rv2 + ad_rv2[i].i * rv1;
#ifdef DEBUG_MUL_OPS
#ifndef __SYNTHESIS__
			printf("ad_mul: a = %d; b = %d; res = %d\n", ad_rv1[i].i, ad_rv2[i].i, local_ad_result[i]);
#endif
#endif
		}
		//===============================
		//d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
		local_is_ad_op = true;
        break;
    case MULH   :
    	//================================
    	//NORMAL MULH:
    	rv1_64 = rv1;
    	rv2_64 = rv2;
    	rv64 = rv1_64 * rv2_64;
    	local_result = rv64 >> 32;
    	//================================
    	//AD MULH:
		for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
	    	adrv1_64 = ad_rv1[i].i;
	    	adrv2_64 = ad_rv2[i].i;
	    	rv64 = adrv1_64 * rv2_64 + adrv2_64 * rv1_64;
	    	local_ad_result[i] = rv64 >> 32;
#ifdef DEBUG_MUL_OPS
#ifndef __SYNTHESIS__
			printf("ad_mulh: a = %d; b = %d; res = %d\n", ad_rv1[i].i, ad_rv2[i].i, local_ad_result[i]);
#endif
#endif
		}
		//===============================
		//d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
		local_is_ad_op = true;
    	break;
    case MULHSU :
    	//================================
    	//NORMAL MULHSU:
        rv1_64  = rv1;
	    urv2_64 = (unsigned)rv2;
	    rv64 = rv1_64 * urv2_64;
	    local_result = rv64 >> 32;
    	//================================
    	//AD MULHSU:
		for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
	    	adrv1_64 = ad_rv1[i].i;
	    	adurv2_64 = (unsigned int)ad_rv2[i].i;
	    	rv64 = adrv1_64 * urv2_64 + adurv2_64 * rv1_64;
	    	local_ad_result[i] = rv64 >> 32;
#ifdef DEBUG_MUL_OPS
#ifndef __SYNTHESIS__
			printf("ad_mulhsu: a = %d; b = %d; res = %d\n", ad_rv1[i].i, ad_rv2[i].i, local_ad_result[i]);
#endif
#endif
		}
		//===============================
		//d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
		local_is_ad_op = true;
	    break;
    case MULHU  :
    	//================================
    	//NORMAL MULHU:
    	urv1_64 = (unsigned)rv1;
    	urv2_64 = (unsigned)rv2;
    	urv64 = urv1_64 * urv2_64;
    	local_result = urv64 >> 32;
    	//================================
    	//AD MULHU:
		for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
	    	adurv1_64 = (unsigned int)ad_rv1[i].i;
	    	adurv2_64 = (unsigned int)ad_rv2[i].i;
	    	rv64 = adurv1_64 * urv2_64 + adurv2_64 * urv1_64;
	    	local_ad_result[i] = rv64 >> 32;
#ifdef DEBUG_MUL_OPS
#ifndef __SYNTHESIS__
			printf("ad_mulhu: a = %d; b = %d; res = %d\n", ad_rv1[i].i, ad_rv2[i].i, local_ad_result[i]);
#endif
#endif
		}
		//===============================
		//d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
		local_is_ad_op = true;
		break;

  }

  //*result = local_result;
  copy_int_array_to_float_int_t(ad_result, local_ad_result, AD_DERIV_CNT);
  *is_ad_op = local_is_ad_op;
  //d_i.is_ad_op = local_is_ad_op;
  return local_result;
  //return true; //return true in either case - only possible in MUL, DIV has REM e.g.
}

int compute_ad_div_result(
  int 					rv1,
  int 					rv2,
  float_int_t 			ad_rv1[AD_DERIV_CNT],
  float_int_t 			ad_rv2[AD_DERIV_CNT],
  //int					*result,
  float_int_t			ad_result[AD_DERIV_CNT],
  decoded_instruction_t d_i,
  bit_t 				*is_ad_op){
#pragma HLS INLINE
  bit_t f7_6          = d_i.func7>>5;
  bit_t f7_0          = d_i.func7;
  ap_uint<2> func3_10 = d_i.func3;
  int temp_const_1;
  int temp_const_2;
  unsigned int temp_uconst_1;
  unsigned int temp_uconst_2;

  //It is important to have local copies for result here I think!
  int local_result;
  int local_ad_result[AD_DERIV_CNT];
#pragma HLS ARRAY_PARTITION variable=local_ad_result type=complete  
#pragma HLS ARRAY_RESHAPE variable=local_ad_result type=complete  
  bit_t local_is_ad_op;

#ifdef HLS_USE_BIND_OP
//#pragma HLS BIND_OP variable=local_result op=mul impl=dsp //original
//#pragma HLS BIND_OP variable=local_result op=mul impl=dsp latency=4
#ifdef USE_FLOAT_DIV
#pragma HLS BIND_OP variable=local_result op=fdiv latency=0
#pragma HLS BIND_OP variable=temp_const_1 op=fdiv latency=0
#pragma HLS BIND_OP variable=temp_const_2 op=fdiv latency=0
#pragma HLS BIND_OP variable=temp_uconst_1 op=fdiv latency=0
#pragma HLS BIND_OP variable=temp_uconst_1 op=fdiv latency=0
#else
#pragma HLS BIND_OP variable=local_result op=mul impl=dsp latency=0
#pragma HLS BIND_OP variable=temp_const_1 op=mul impl=dsp latency=0
#pragma HLS BIND_OP variable=temp_const_2 op=mul impl=dsp latency=0
#pragma HLS BIND_OP variable=temp_uconst_1 op=mul impl=dsp latency=0
#pragma HLS BIND_OP variable=temp_uconst_1 op=mul impl=dsp latency=0
#endif
#endif //HLS_USE_BIND_OP

  long int   rv64;

#ifdef HLS_USE_BIND_OP
//#pragma HLS BIND_OP variable=rv64   op=mul impl=dsp //original
//#pragma HLS BIND_OP variable=rv64 op=mul impl=dsp latency=0
#pragma HLS BIND_OP variable=rv64 op=mul latency=0
#endif

  max_der_t i;
  switch(func3_10){
    case MUL    : //DIV
	  //================================
	  //NORMAL DIV:
	  if (d_i.is_null_rv2){
		local_result = -1;
	  }
	  //https://five-embeddev.com/riscv-isa-manual/latest/m.html
	  //Signed division overflow occurs only when the most-negative integer is divided by  − 1.
	  //The quotient of a signed division with overflow is equal to the dividend, and the remainder is zero.
	  //Unsigned division overflow cannot occur.
	  else if ((d_i.is_last_negative_rv1) && (d_i.is_first_negative_rv2)){
		local_result = rv1;
	  }
	  else{
#ifdef USE_FLOAT_DIV
		local_result = (int)(hls::floor((float)(rv1) / (float)(rv2)));
#else
		local_result = rv1 / rv2;
#endif
	  }
	  //================================
	  //AD DIV:
#ifdef USE_FLOAT_DIV
  	  temp_const_1 = (int)(hls::floor((float)(1) / (float)(rv2)));
  	  temp_const_2 = (int)(hls::floor((float)(rv1) / (float)(rv2*rv2)));
#else
	  temp_const_1 = (1 / rv2);
	  temp_const_2 = (rv1 / (rv2 * rv2));
#endif
	  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
		if (d_i.is_null_rv2){
		  local_ad_result[i] = -1;
		}
		else if ((d_i.is_last_negative_rv1) && (d_i.is_first_negative_rv2)){
  		  //TODO: What the hell am I supposed to do here??
		  //From issue.cpp:
		  //i_state->decoded_instruction.is_first_negative_rv2 = (rv2.i == -1); // 0xFFFFFFFF
		  //i_state->decoded_instruction.is_last_negative_rv1  = (rv1.i == 0x80000000);
	      // 0x80000000 / 0xFFFFFFFF -> -2147483648 / -1
		  //https://five-embeddev.com/riscv-isa-manual/latest/m.html
		  //Signed division overflow occurs only when the most-negative integer is divided by  − 1.
		  //The quotient of a signed division with overflow is equal to the dividend, and the remainder is zero.
		  //Unsigned division overflow cannot occur.
			local_ad_result[i] = (ad_rv1[i].i * temp_const_1) - (ad_rv2[i].i * rv1);
		}
		else{
			local_ad_result[i] = (ad_rv1[i].i * temp_const_1) - (ad_rv2[i].i * temp_const_2);
#ifdef DEBUG_MUL_OPS
#ifndef __SYNTHESIS__
    	  printf("ad_div: a = %d; b = %d; res = %d\n", ad_rv1[i].i, ad_rv2[i].i, local_ad_result[i]);
#endif
#endif
		}
	  }
	  //===============================
	  //d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
	  local_is_ad_op = true;
	  break;
    case MULH   : // DIVU
  	  //================================
  	  //NORMAL DIVU:
	  if (d_i.is_null_rv2){
		local_result = 0xffffffff;
	  }
	  else{
#ifdef USE_FLOAT_DIV
		local_result = (unsigned int)(hls::floor((float)((unsigned int)rv1) / (float)((unsigned int)rv2)));
#else
		local_result = (unsigned int)rv1 / (unsigned int)rv2;
#endif
	  }
  	  //================================
	  //AD DIVU:
#ifdef USE_FLOAT_DIV
	  temp_uconst_1 = (unsigned int)(hls::floor((float)(1) / (float)((unsigned int)rv2)));
	  temp_uconst_2 = (unsigned int)(hls::floor((float)((unsigned int)rv1) / (float)((unsigned int)rv2 * (unsigned int)rv2)));
#else
	  temp_uconst_1 = (1 / (unsigned int)rv2);
	  temp_uconst_2 = ((unsigned int)rv1 / ((unsigned int)rv2 * (unsigned int)rv2));
#endif
	  for(i = 0; i < AD_DERIV_CNT; i++){
#pragma HLS UNROLL
		if (d_i.is_null_rv2){
			local_ad_result[i] = -1;
		}
		else{
			local_ad_result[i] = ((unsigned int)ad_rv1[i].i * temp_uconst_1) - ((unsigned int)ad_rv2[i].i * temp_uconst_2);
#ifdef DEBUG_MUL_OPS
#ifndef __SYNTHESIS__
    	  printf("ad_divu: a = %d; b = %d; res = %d\n", ad_rv1[i].i, ad_rv2[i].i, local_ad_result[i]);
#endif
#endif
		}
	  }
  	  //================================
      //d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
	  local_is_ad_op = true;
	  break;
    case MULHSU : //REM
	  //================================
	  //NORMAL REM:
	  if (d_i.is_null_rv2){
		local_result = rv1;
	  }
	  else if ((d_i.is_last_negative_rv1) && (d_i.is_first_negative_rv2)){
		local_result = 0;
	  }
	  else{
#ifdef USE_FLOAT_DIV
        //in that case?? how to do a float modulo?
        local_result = (int) (hls::floor(hls::fmod((float)rv1, (float)rv2)));
#else
		local_result = rv1 % rv2;
#endif
	  }
	  //================================
	  //AD REM:

	  //TODO: what to do here?? This is not differentiable

	  //d_i.is_ad_op = false; 		//is false by default anyway - no WB to ad-regs
	  local_is_ad_op = false;
	  //================================
	  break;
    case MULHU  : //REMU
  	  //================================
  	  //NORMAL REMU:
	  if (d_i.is_null_rv2){
		local_result = rv1;
	  }
	  else{
#ifdef USE_FLOAT_DIV
        local_result = (unsigned int) (hls::floor(hls::fmod((float)(unsigned int)rv1, (float)(unsigned int)rv2)));
#else
		local_result = (unsigned int)rv1 % (unsigned int)rv2;
#endif
	  }
	  //================================
	  //AD REMU:

	  //TODO: what to do here?? This is not differentiable

	  //d_i.is_ad_op = false; 		//is false by default anyway - no WB to ad-regs
	  local_is_ad_op = false;
	  //================================
	  break;
  }

  //*result = local_result;
  copy_int_array_to_float_int_t(ad_result, local_ad_result, AD_DERIV_CNT);
  //d_i.is_ad_op = local_is_ad_op;
  *is_ad_op = local_is_ad_op;
  return local_result;
}

#else // NO AD
int compute_mul_result(
  int                   rv1,
  int                   rv2,
  decoded_instruction_t d_i){
#pragma HLS INLINE
  bit_t f7_6          = d_i.func7>>5;
  bit_t f7_0          = d_i.func7;
  ap_uint<2> func3_10 = d_i.func3;
  int   result;
//#pragma HLS BIND_OP variable=result op=mul impl=dsp //original
#pragma HLS BIND_OP variable=result op=mul impl=dsp latency=0
  long int          rv64,   rv1_64,  rv2_64;
  unsigned long int urv64, urv1_64, urv2_64;
//#pragma HLS BIND_OP variable=rv64   op=mul impl=dsp //original
#pragma HLS BIND_OP variable=result op=mul impl=dsp latency=0
  switch(func3_10){
    case MUL    : result = rv1 * rv2;
                  break;
    case MULH   : rv1_64 = rv1;
                  rv2_64 = rv2;
                  rv64 = rv1_64 * rv2_64;
    	          result = rv64 >> 32;
                  break;
    case MULHSU : rv1_64  = rv1;
                  urv2_64 = (unsigned)rv2;
                  rv64 = rv1_64 * urv2_64;
                  result = rv64 >> 32;
                  break;
    case MULHU  : urv1_64 = (unsigned)rv1;
                  urv2_64 = (unsigned)rv2;
                  urv64 = urv1_64 * urv2_64;
                  result = urv64 >> 32;
  }
  return result;
}
int compute_div_result(
  int                   rv1,
  int                   rv2,
  decoded_instruction_t d_i){
#pragma HLS INLINE
  bit_t f7_6          = d_i.func7>>5;
  bit_t f7_0          = d_i.func7;
  ap_uint<2> func3_10 = d_i.func3;
  int result;
//#pragma HLS BIND_OP variable=result op=mul impl=dsp //original
#pragma HLS BIND_OP variable=result op=mul impl=dsp latency=4
  long int   rv64;
//#pragma HLS BIND_OP variable=rv64   op=mul impl=dsp //original
#pragma HLS BIND_OP variable=result op=mul impl=dsp latency=4
  switch(func3_10){
    case MUL    : //DIV
                  if (d_i.is_null_rv2)
                    result = -1;
                  else if ((d_i.is_last_negative_rv1) && (d_i.is_first_negative_rv2))
                    result = rv1;
                  else
                    result = rv1 / rv2;
                  break;
    case MULH   : //DIVU
                  if (d_i.is_null_rv2)
                    result = 0xffffffff;
                  else
                    result = (unsigned int)rv1 / (unsigned int)rv2;
                  break;
    case MULHSU : //REM
                  if (d_i.is_null_rv2)
                    result = rv1;
                  else if ((d_i.is_last_negative_rv1) && (d_i.is_first_negative_rv2))
                    result = 0;
    	          else
                    result = rv1 % rv2;
                  break;
    case MULHU  : //REMU
                  if (d_i.is_null_rv2)
                    result = rv1;
    	          else
                    result = (unsigned int)rv1 % (unsigned int)rv2;
                  break;
  }
  return result;
}
#endif //AD


int compute_result(
  int rv1,
  code_address_t pc,
  decoded_instruction_t decoded_instruction){
#pragma HLS INLINE
  code_address_t pc4 = pc + 4;
  int result;
  switch(decoded_instruction.type){
    case R_TYPE:
      result = 0;//computed in compute_op_result OR compute_fp_op_result
      break;
#ifdef FPU
    case R4_TYPE:
      result = 0;//computed in compute_fp_op_result
      break;
#endif
    case I_TYPE:
      if (decoded_instruction.is_jalr)
        result = (unsigned int)pc4;
      else if (decoded_instruction.is_load
#ifdef FPU
    		  || decoded_instruction.is_load_fp
#endif
    		  ){
        result = rv1 + (int)decoded_instruction.imm;
#ifdef AD
        decoded_instruction.is_ad_op = true; 		//TODO: has to be set here?
#endif
      }
      else //if (decoded_instruction.is_op_imm)
        result = 0;//computed in compute_op_result or compute_fp_op_result
      //else
        //result = 0;//(d_i.opcode == SYSTEM)
      break;
    case S_TYPE:
      result = rv1 + (int)decoded_instruction.imm;
#ifdef AD
      decoded_instruction.is_ad_op = true; 		//TODO: has to be set here?
#endif
      break;
    case B_TYPE:
      result = 0;//computed in compute_branch_result
      break;
    case U_TYPE:
    	//TODO: this was the bug I reported to Bernard, why is it different in this version? -> makes no difference in think
      if (decoded_instruction.is_lui){
#ifdef USE_OLD_BUG
    	result = (int)(decoded_instruction.imm<<12);
#else
    	result = ((unsigned int)decoded_instruction.imm) << 12;
#endif
      }
      else{ //d_i.opcode == AUIPC
#ifdef USE_OLD_BUG
    	  result = (int)pc + (int)(decoded_instruction.imm<<12);
#else
    	  result = (int)pc + (int)(decoded_instruction.imm<<12);
#endif
      }
      break;
    case J_TYPE:
      result = (unsigned int)pc4;
      break;
#ifdef AD
    case AD_S_TYPE:
      //Carefully: here we DON'T add an offset, the imm here is just the AD-Channel! BUT, we have to pass this to the memory
      result = rv1;// + (int)decoded_instruction.imm;
      decoded_instruction.is_ad_op = true; 		//TODO: has to be set here?
      break;
#endif
    default:
      result = 0;
      //TODO: why is in bernards code: result.i = 0; result.f = 0.0; it is the same right??
      break;
  }
  //TODO: is this correct???????
  //TODO:???????????????????????
  //TODO:???????!!!!!!!!????????
  //basically no matter what, leave ad_rv1 as it is? -> NO THAT CHANGES IT - I have to pass ad_Rd up unto this point as well
  //or pass flag "is_ad_op" up to the writeback -   decoded_instruction.is_ad_op = false; by default
  //copy_array(ad_result2, ad_r, AD_DERIV_CNT, sizeof(int));
  //decicion: don't copy anything, wont be written anyway anywhere
  return result;
}
code_address_t compute_next_pc(
  code_address_t pc,
  decoded_instruction_t decoded_instruction,
  int rv1){
#pragma HLS INLINE
  code_address_t next_pc;
  code_address_t j_b_target_pc = (pc +
           (code_address_t)
           (((int)decoded_instruction.imm<<1)));
  code_address_t i_target_pc = ((rv1 +
             (int)decoded_instruction.imm)&0xfffffffe);
  if (decoded_instruction.is_jalr)
    next_pc = i_target_pc;
  else
    next_pc = j_b_target_pc;
  return next_pc;
}
