#ifndef __MULTICYCLE_PIPELINE_IP
#define __MULTICYCLE_PIPELINE_IP

//https://forums.xilinx.com/t5/High-Level-Synthesis-HLS/Vivado-HLS-2019-1-C-RTL-CoSimulation-Compilation-error-for-the/td-p/1075588
/*
	1. Uninstall libgmp-dev
	2. Find the path containing gmp.h and include the header file of this path #include "/opt/Xilinx/Vivado_HLS/2019.1/include/gmp.h"
	3. Add the type _gmp_const type definition suggested in error to the header file of the system gmp.h.
 */

#include "/tools/Xilinx/Vitis_HLS/2022.2/include/gmp.h"
#if __GMP_HAVE_CONST
#define __gmp_const const
#define __gmp_signed signed
#else
#define __gmp_const
#define __gmp_signed
#endif
#include "/tools/Xilinx/Vitis_HLS/2022.2/include/mpfr.h"

#include "ap_int.h"
#include "debug_multicycle_pipeline_ip.h"

//AD_ADATION:
#define DIFFERENTIATE_BITWISE_OPS
//AD_ADATION_END:

//#define LOG_MEM_SIZE 14 //original: 11 //2^11=2048 Byte, 2KB, das ist nat. nicht viel ;) 2^13=8KB = 2K Instructions
//size in bytes
//#define MEM_SIZE     (1<<LOG_MEM_SIZE)

#define LOG_CODE_MEM_SIZE 11//13 //original: 11 //2^11=2048 Byte, 2KB, das ist nat. nicht viel ;) 2^13=8KB = 2K Instructions "cached"
//size in bytes
#define CODE_MEM_SIZE     (1<<LOG_CODE_MEM_SIZE)
#define LOG_DATA_MEM_SIZE 13//14 //original: 11 //has to be at least one more than LOG_CODE_MEM_SIZE for stack //14 = 4K Data "cached"
//size in bytes
#define DATA_MEM_SIZE     (1<<LOG_DATA_MEM_SIZE)

#define LOG_REG_FILE_SIZE  5
#define NB_REGISTER       (1<<LOG_REG_FILE_SIZE)


//AD_ADATION:
#define MAX_LOG_AD_DERIV_CNT  1 //1	|2	|3	|4	|5	|6	|7	|8							//3 Bit max -> 2^3-1 = 7 derivatives;
														//MAX_SIZE = 12!! equal to s_immediate
typedef ap_uint<MAX_LOG_AD_DERIV_CNT> max_der_t;
#define AD_DERIV_CNT		  1 //1	|3	|7  |15	|31	|63	|127 |255					//max 2^MAX_LOG_AD_DERIV_CNT
//Hack to use pragma with defines (in main to partition memory): https://support.xilinx.com/s/article/46111?language=en_US
#define PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) PRAGMA_SUB(x)
//AD_ADATION_END:

#define RET            0x8067 //
#define EXIT		   0x2b	  //TODO: this is a temporary custom OPCODE (custom-1 0101011 and fnc3 = 000) to EXIT processing
#define NOP            0x13

#define RA             1

#define LOAD           0b00000
#define LOAD_FP        0b00001
#define CUSTOM_0	   0b00010
//AD_ADATION:
#define AD_STORE       0b00010
//END_AD_ADATION:
#define MISC_MEM       0b00011
#define OP_IMM         0b00100
#define AUIPC          0b00101
#define OP_IMM_32      0b00110
#define RV48_0         0b00111
#define STORE          0b01000
#define STORE_FP       0b01001
#define CUSTOM_1       0b01010
#define AMO            0b01011
#define OP             0b01100
#define LUI            0b01101
#define OP_32          0b01110
#define RV_64          0b01111
#define MADD           0b10000
#define MSUB           0b10001
#define NMSUB          0b10010
#define NMADD          0b10011
#define OP_FP          0b10100
#define RESERVED_0     0b10101
#define CUSTOM_2_RV128 0b10110
#define RV48_1         0b10111
#define BRANCH         0b11000
#define JALR           0b11001
#define RESERVED_1     0b11010
#define JAL            0b11011
#define SYSTEM         0b11100
#define RESERVED_2     0b11101
#define CUSTOM_3_RV128 0b11110
#define RV80           0b11111

#define UNDEFINED_TYPE 0
#define R_TYPE         1
#define I_TYPE         2
#define S_TYPE         3
#define B_TYPE         4
#define U_TYPE         5
#define J_TYPE         6
#define OTHER_TYPE     7
//AD_ADATION:
#define AD_S_TYPE      8
//END_AD_ADATION:

#define BEQ            0
#define BNE            1
#define BLT            4
#define BGE            5
#define BLTU           6
#define BGEU           7

#define ADD            0
#define SUB            0
#define SLL            1
#define SLT            2
#define SLTU           3
#define XOR            4
#define SRL            5
#define SRA            5
#define OR             6
#define AND            7

#define ADDI           0
#define SLLI           1
#define SLTI           2
#define SLTIU          3
#define XORI           4
#define SRLI           5
#define SRAI           5
#define ORI            6
#define ANDI           7

#define ECALL          0
#define EBREAK         1

#define LB             0
#define LH             1
#define LW             2
#define LBU            4
#define LHU            5

#define SB             0
#define SH             1
#define SW             2

//AD_ADAPTION:
typedef ap_fixed<64,32,AP_TRN_ZERO,AP_WRAP> fixp64;
typedef ap_ufixed<64,32,AP_TRN_ZERO,AP_WRAP> ufixp64;
typedef ap_fixed<40,32,AP_TRN_ZERO,AP_WRAP> fixp40;
typedef ap_ufixed<40,32,AP_TRN_ZERO,AP_WRAP> ufixp40;
//typedef ap_fixed<8,1,AP_TRN_ZERO,AP_WRAP> fixp8;
typedef ap_ufixed<8,1,AP_TRN_ZERO,AP_WRAP> ufixp8;

//#define ln2 ((ufixp8)0.6931471805599453094172321214581)
#define ln2 0.6931471805599453094172321214581


//static const fixp38 fixln2 = ((fixp38) 0.6931471805599453094172321214581);
//static const float fln2 = ((fixp38) 0.6931471805599453094172321214581);
//AD_ADATION_END:
typedef unsigned int                 instruction_t;
typedef ap_uint<LOG_CODE_MEM_SIZE>   code_address_t;
typedef ap_uint<LOG_DATA_MEM_SIZE>   data_address_t;
typedef ap_uint<4>                   type_t;
typedef ap_int<20>                   immediate_t;
typedef ap_int<12>                   i_immediate_t;
typedef ap_int<12>                   s_immediate_t;
typedef ap_int<12>                   b_immediate_t;
typedef ap_int<20>                   u_immediate_t;
typedef ap_int<20>                   j_immediate_t;
typedef ap_uint<12>                  i_uimmediate_t;
typedef ap_uint<12>                  s_uimmediate_t;
typedef ap_uint<12>                  b_uimmediate_t;
typedef ap_uint<20>                  u_uimmediate_t;
typedef ap_uint<20>                  j_uimmediate_t;
typedef ap_uint<5>                   opcode_t;
typedef ap_uint<LOG_REG_FILE_SIZE+1> reg_num_p1_t;
typedef ap_uint<LOG_REG_FILE_SIZE>   reg_num_t;
typedef ap_uint<3>                   func3_t;
typedef ap_uint<7>                   func7_t;
typedef ap_uint<1>                   bit_t;
typedef ap_uint<32>                  counter_t;

//AD_ADATION:
typedef struct reg_s{
	int gp_reg_file[NB_REGISTER];
//#pragma HLS ARRAY_PARTITION variable=reg_file dim=1 complete //function scope only
	int ad_reg_file[NB_REGISTER][AD_DERIV_CNT];
//#pragma HLS ARRAY_PARTITION variable=reg_file dim=2 complete
	bit_t is_reg_computed[NB_REGISTER];
//#pragma HLS ARRAY_PARTITION variable=is_reg_computed dim=1 complete
} reg_t;
//AD_ADATION_END:

//AD_ADATION:
typedef struct data_mem_s{
	  char data_mem_0[DATA_MEM_SIZE/sizeof(int)] = {0};
	  char data_mem_1[DATA_MEM_SIZE/sizeof(int)] = {0};
	  char data_mem_2[DATA_MEM_SIZE/sizeof(int)] = {0};
	  char data_mem_3[DATA_MEM_SIZE/sizeof(int)] = {0};
	  char ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT] = {0};
	  char ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT] = {0};
	  char ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT] = {0};
	  char ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT] = {0};
} data_mem_t;
//AD_ADATION_END:

typedef struct decoded_instruction_s{
  bit_t       inst_31;
  ap_uint<6>  inst_30_25;
  ap_uint<4>  inst_24_21;
  bit_t       inst_20;
  ap_uint<8>  inst_19_12;
  ap_uint<4>  inst_11_8;
  bit_t       inst_7;
  opcode_t    inst_6_2;
  opcode_t    opcode;
  ap_uint<2>  inst_1_0;
  reg_num_t   rd;
  func3_t     func3;
  reg_num_t   rs1;
  reg_num_t   rs2;
  func7_t     func7;
  type_t      type;
  immediate_t imm;
  bit_t       is_rs1_reg;
  bit_t       is_rs2_reg;
  bit_t       is_r_type;
  bit_t       is_load;
  bit_t       is_store;
  //AD_ADATION:
  bit_t		  is_ad_store;
  bit_t		  is_ad_op;		//needed for writeback - if false, no writeback to ad-registers
  //END_AD_ADATION:
  bit_t       is_branch;
  bit_t       is_jal;
  bit_t       is_jalr;
  bit_t       is_ret;
  bit_t       is_exit;
  bit_t       is_lui;
  bit_t       is_op_imm;
  bit_t       is_system;
  bit_t       is_jump;
  bit_t       is_ctrl;
  bit_t       has_no_dest;
  ap_uint<2>  opch;
  ap_uint<3>  opcl;
} decoded_instruction_t;

typedef struct from_f_to_d_s{
  bit_t          is_valid;
  code_address_t pc;
  instruction_t  instruction;
} from_f_to_d_t;

typedef struct f_state_s{
  bit_t          is_full;
  code_address_t pc;
#ifndef __SYNTHESIS__
#ifdef DEBUG_DISASSEMBLE
  instruction_t  instruction;
#endif
#endif
} f_state_t;

typedef struct from_d_to_f_s{
  bit_t          is_valid;
  code_address_t target_pc;
} from_d_to_f_t;

typedef struct from_d_to_i_s{
  bit_t                 is_valid;
  code_address_t        pc;
#ifndef __SYNTHESIS__
  instruction_t         instruction;
  code_address_t        target_pc;
#endif
  decoded_instruction_t decoded_instruction;
} from_d_to_i_t;

typedef struct d_state_s{
  bit_t          is_full;
  code_address_t pc;
  instruction_t  instruction;
} d_state_t;

typedef struct from_i_to_e_1_s{
  bit_t                 is_valid;
  int                   rv1;
  int                   rv2;
  int                   ad_rv1[AD_DERIV_CNT];
  int                   ad_rv2[AD_DERIV_CNT];
  code_address_t        pc;
#ifndef __SYNTHESIS__
  instruction_t         instruction;
  code_address_t        target_pc;
#endif
  decoded_instruction_t decoded_instruction;
} from_i_to_e_1_t;

typedef struct i_state_s{
  bit_t                 is_full;
  code_address_t        pc;
#ifndef __SYNTHESIS__
  instruction_t         instruction;
  code_address_t        target_pc;
#endif
  decoded_instruction_t decoded_instruction;
} i_state_t;

typedef struct from_e_1_to_f_s{
  bit_t          is_valid;
  code_address_t target_pc;
} from_e_1_to_f_t;

typedef struct from_e_1_to_e_2_s{
  bit_t                 is_valid;
  reg_num_t             rd;
  bit_t                 has_no_dest;
  bit_t                 is_r_type;
  bit_t                 is_op_imm;
  bit_t                 is_ad_op;
  bit_t                 is_ret;
  bit_t       			is_exit;
  int                   result1;
  int                   result2;
  int                   ad_result1[AD_DERIV_CNT];
  int                   ad_result2[AD_DERIV_CNT];
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
  code_address_t        target_pc;
#endif
} from_e_1_to_e_2_t;

typedef struct from_e_1_to_m_s{
  bit_t                 is_valid;
  data_address_t        address;
  char                  value_0;
  char                  value_1;
  char                  value_2;
  char                  value_3;
  char                  ad_value_0[AD_DERIV_CNT];
  char                  ad_value_1[AD_DERIV_CNT];
  char                  ad_value_2[AD_DERIV_CNT];
  char                  ad_value_3[AD_DERIV_CNT];
  reg_num_t             rd;
  reg_num_t             rs2;
  int					rv1;
  bit_t                 has_no_dest;
  bit_t                 is_load;
  bit_t                 is_store;
  //AD_ADATION:
  bit_t					is_ad_store;
  bit_t                 is_ad_op;
  max_der_t	  	  	  	ad_channel;
  //END_AD_ADATION:
  func3_t               func3;
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
#endif
} from_e_1_to_m_t;

typedef struct e_1_state_s{
  bit_t                 is_full;
  int                   rv1;
  int                   rv2;
  int                   ad_rv1[AD_DERIV_CNT];
  int                   ad_rv2[AD_DERIV_CNT];
  code_address_t        pc;
  decoded_instruction_t decoded_instruction;
#ifndef __SYNTHESIS__
  instruction_t         instruction;
  code_address_t        target_pc;
#endif
} e_1_state_t;

typedef struct from_e_2_to_w_s{
  bit_t                 is_valid;
  int                   result;
  int                   ad_result[AD_DERIV_CNT];
  reg_num_t             rd;
  bit_t                 is_ad_op;
  bit_t                 has_no_dest;
  bit_t                 is_ret;
  bit_t                 is_exit;
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
  code_address_t        target_pc;
#endif
} from_e_2_to_w_t;

typedef struct e_2_state_s{
  bit_t                 is_full;
  reg_num_t             rd;
  bit_t                 has_no_dest;
  bit_t                 is_r_type;
  bit_t                 is_op_imm;
  bit_t                 is_ad_op;
  bit_t                 is_ret;
  bit_t                 is_exit;
  int                   result1;
  int                   result2;
  int                   ad_result1[AD_DERIV_CNT];
  int                   ad_result2[AD_DERIV_CNT];
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
  code_address_t        target_pc;
#endif
} e_2_state_t;

typedef struct from_m_to_w_s{
  bit_t                 is_valid;
  int                   result;
  char                  value_0;
  char                  value_1;
  char                  value_2;
  char                  value_3;
  char                  ad_value_0[AD_DERIV_CNT];
  char                  ad_value_1[AD_DERIV_CNT];
  char                  ad_value_2[AD_DERIV_CNT];
  char                  ad_value_3[AD_DERIV_CNT];
  ap_uint<2>            a01;
  bit_t                 a1;
  func3_t               msize;
  reg_num_t             rd;
  bit_t                 has_no_dest;
  bit_t                 is_ad_op;
  bit_t					is_ad_store;
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
#endif
} from_m_to_w_t;

typedef struct m_state_s{
  data_address_t        address;
  char                  value_0;
  char                  value_1;
  char                  value_2;
  char                  value_3;
  char                  ad_value_0[AD_DERIV_CNT];
  char                  ad_value_1[AD_DERIV_CNT];
  char                  ad_value_2[AD_DERIV_CNT];
  char                  ad_value_3[AD_DERIV_CNT];
  reg_num_t             rd;
  reg_num_t             rs2;
  int					rv1;
  bit_t                 has_no_dest;
  bit_t                 is_load;
  bit_t                 is_store;
  //AD_ADATION:
  bit_t					is_ad_store;
  bit_t                 is_ad_op;
  max_der_t	  	  	  	ad_channel;
  //END_AD_ADATION:
  func3_t               func3;
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
#endif
} m_state_t;

typedef struct w_state_s{
  int                   value;
  int                   ad_value[AD_DERIV_CNT];
  char                  ad_value_0[AD_DERIV_CNT];
  char                  ad_value_1[AD_DERIV_CNT];
  char                  ad_value_2[AD_DERIV_CNT];
  char                  ad_value_3[AD_DERIV_CNT];
  reg_num_t             rd;
  bit_t                 has_no_dest;
  bit_t                 is_ad_op;
  bit_t					is_ad_store;
  bit_t                 is_ret;
  bit_t                 is_exit;
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
  code_address_t        target_pc;
#endif
} w_state_t;

void ad_multicycle_pipeline_ip(
  code_address_t start_pc,
  instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  //data_mem_t    *data_mem,
  char    g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
  char    g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
  char    g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
  char    g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
  char    g_ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    g_ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    g_ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    g_ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  unsigned int  *nb_instruction,
  unsigned int  *nb_cycle);

//TODO: static or not? and why?
void copy_array_8bit(
  char a[],
  char b[],
  max_der_t length
  );

//TODO: static or not? and why?
void copy_value_8bit(
  char a[],
  char *b,
  max_der_t length
  );

//TODO: static or not? and why?
void copy_array_32bit(
  int a[],
  int b[],
  max_der_t length
  );

//TODO: static or not? and why?
void copy_value_32bit(
  int a[],
  int *b,
  max_der_t length
  );


#endif
