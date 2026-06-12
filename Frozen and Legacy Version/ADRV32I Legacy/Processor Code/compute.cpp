#include "ap_int.h"
#include "ad_multicycle_pipeline_ip.h"

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

int compute_op_result(
  int rv1,
  int right,
  decoded_instruction_t d_i){
#pragma HLS INLINE
  bit_t f7_6   = d_i.func7>>5;
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
    case XOR : result = rv1 ^ rv2;
               break;
    case SRL : if (f7_6) result = rv1 >> shift;//SRA
               else result = (unsigned int)rv1 >> shift;
               break;
    case OR  : result = rv1 | rv2;
               break;
    case AND : result = rv1 & rv2;
               break;
  }
  return result;
}

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

		//normal variant
		//int temp = (fixp40) b * ln2 * ad_shift[i];

		//int temp = int((float)0.6931471805599453094172321214581 * float(ad_shift[i]) * float(b));

		//AD_Rounded Version to ease up things:
		//int temp = (b << 2) * ad_shift[i];

		fixp64 temp = (fixp64) b * (fixp64) ln2 * (fixp64) ad_shift[i];
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
		/*int temp;
		if(sign == true)
			temp = (fixp40) b * ln2 * ad_shift[i];
		else
			temp = (ufixp40) b * ln2 * ad_shift[i];*/

		int temp;
		if(sign == true)
			temp = (fixp64) b * (ufixp64) ln2 * (fixp64) ad_shift[i];
		else
			temp = (ufixp64) b * (ufixp64) ln2 * (fixp64) ad_shift[i];

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
		result[i] = (ad_b[i] >> c) + (temp >> c);
	}
}


//TODO: CAREFULLY; WE PASS "decoded_instruction_t d_i" values directly, NOT a pointer, so changes wont reach execute1, we have to return the result!
//TODO: IS THERE A BETTER WAY??
bit_t compute_ad_op_result(
  int rv1,
  int right,
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
  switch(d_i.func3){
    case ADD : if (d_i.is_r_type && f7_6){

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
    case SLL :	*result = rv1 << shift;
    		  	//a = b << c = b * 2 ^ c
    			vsll(ad_result1, ad_rv1, temp_shift, rv1, shift);
    			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
    			//da = (db << c) + ((b * ln(2) * dc) << c)
    	       break;
    case SLT : *result = rv1 < rv2;
				//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
				#ifdef DIFFERENTIATE_BITWISE_OPS
    			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
					#pragma HLS UNROLL
				    ad_result1[i] = ad_rv1[i] < (int)d_i.imm;;
				}
    			d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
				#endif
    		   break;
    case SLTU: *result = (unsigned int)rv1 < (unsigned int)rv2;
				//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
				#ifdef DIFFERENTIATE_BITWISE_OPS
				for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
					#pragma HLS UNROLL
				    ad_result1[i] = (unsigned int)ad_rv1[i] < (unsigned int)d_i.imm;;
				}
				d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
				#endif
               break;
    case XOR : *result = rv1 ^ rv2;
				//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?

				#ifdef DIFFERENTIATE_BITWISE_OPS
    			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
					#pragma HLS UNROLL
				    ad_result1[i] = ad_rv1[i] < (int)d_i.imm;;
    			}
			   d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB
				#endif

               break;
    case SRL : if (f7_6){
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
    case OR  : *result = rv1 | rv2;
    		   	//TODO: IS THIS CORRECT? BIT MANIPULATIONS ON AD REGS?
				#ifdef DIFFERENTIATE_BITWISE_OPS
    			for(max_der_t i = 0; i < AD_DERIV_CNT; i++){
					#pragma HLS UNROLL
				    ad_result1[i] = ad_rv1[i] | (int)d_i.imm;;
    			}
				d_i.is_ad_op = true; 		//get ready to overwrite AD_Registers in WB*/
				#endif
               break;
    case AND : *result = rv1 & rv2;
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
  return d_i.is_ad_op;
}

int compute_result(
  int rv1,
  //int ad_rv1[AD_DERIV_CNT],
  code_address_t pc,
  //int ad_result2[AD_DERIV_CNT],
  decoded_instruction_t decoded_instruction){
#pragma HLS INLINE
  code_address_t pc4 = pc + 4;
  int result;
  switch(decoded_instruction.type){
    case R_TYPE:
      result = 0;//computed in compute_op_result
      break;
    case I_TYPE:
      if (decoded_instruction.is_jalr)
        result = (unsigned int)pc4;
      else if (decoded_instruction.is_load){
        result = rv1 + (int)decoded_instruction.imm;
        decoded_instruction.is_ad_op = true; 		//TODO: has to be set here?
      }
      else //if (decoded_instruction.is_op_imm)
        result = 0;//computed in compute_op_result
      //else
        //result = 0;//(d_i.opcode == SYSTEM)
      break;
    //TODO:result is just the address, nothing else to be changed here, AD-Registers will be loaded from memory in mem.c later
    case S_TYPE:
      result = rv1 + (int)decoded_instruction.imm;
      decoded_instruction.is_ad_op = true; 		//TODO: has to be set here?
      break;
    case B_TYPE:
      result = 0;//computed in compute_branch_result
      break;
    case U_TYPE:
      if (decoded_instruction.is_lui)
          result = ((unsigned int)decoded_instruction.imm) << 12;
      else//d_i.opcode == AUIPC
        result = (int)pc +
                 (int)(decoded_instruction.imm<<12);
      break;
    case J_TYPE:
      result = (unsigned int)pc4;
      break;
    case AD_S_TYPE:
      //Carefully: here we DON'T add an offset, the imm here is just the AD-Channel! BUT, we have to pass this to the memory
      result = rv1;// + (int)decoded_instruction.imm;
      decoded_instruction.is_ad_op = true; 		//TODO: has to be set here?
      break;
    default:
      result = 0;
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
