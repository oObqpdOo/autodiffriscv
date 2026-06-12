#ifndef __MULTICYCLE_PIPELINE_IP
#define __MULTICYCLE_PIPELINE_IP

#include "ap_int.h"
#include "debug_adrv32imf_mp_ip.h"

#define USE_DEFAULTS

//===========================================================================
//DEFAULT TOGGLES: DEFAULT
//===========================================================================

#ifdef USE_DEFAULTS
#define FPU
#define AD
#define LOCK_FCSR
#define EXTERNAL_FPU_COMP
#define USE_HLS_MATH
#define UNIFIED_MEMORY
#define LOCAL_MEMORY

//#define USE_LESS_CYCLES_FOR_EXEC
#if !defined (USE_LESS_CYCLES_FOR_EXEC) && !defined (USE_MORE_CYCLES_FOR_EXEC)
#define LOG_MUL_LATENCY         1
#define MUL_LATENCY             ((1<<LOG_MUL_LATENCY) - 1)  //2-1 =  1 ( 1 FPGA cycles at II=1)
#define LOG_DIV_LATENCY         6
#define DIV_LATENCY             ((1<<LOG_DIV_LATENCY) - 28) //64-29 = 36 (36 FPGA cycles at II=1)
#ifdef FPU
#define LOG_FPU_LATENCY         4
#define FPU_LATENCY             ((1<<LOG_FPU_LATENCY) - 5)  //16-5 = 11 (11 FPGA cycles at II=1) //orig: 16-4=12 (12 FPGA cycles at II=1)
#endif
#endif

#define FETCH_EVERY_CYCLE
#define REVERSE_MAIN_LOOP

//#define RUNNING_CONDITION_EXIT
//#define LIMIT_CYCLES
//#define MAX_CYCLES 20000

#define FIX_II_2
//#define PRAGMA_USE_BRAM_ALVEO
#define PRAGMA_USE_LATENCY

#define ARRAY_PARTITION_FACTOR  1
#define LOG_CODE_MEM_SIZE       16    //16 = 2^16=64KB=16k Instructions
#define LOG_DATA_MEM_SIZE       17    //17 = 2^17=128KB
#define MAX_LOG_AD_DERIV_CNT    4     //1 | 2 | 3 | 4  ( | 5  | 6  | 7   | 8   )
#define AD_DERIV_CNT		    15    //1 | 3 | 7 | 15 ( | 31 | 63 | 127 | 255 )

#endif

//===========================================================================
//IMPORTANT TOGGLES: GENERAL
//===========================================================================
//Uncomment next line to achieve II=2 on Alveo U50
//ATTENTION: This is crucial to get II=2 on ALVEO
//TODO: Check if it messes with inter dependencies
//#define FIX_II_2
//===========================================================================
//IMPORTANT TOGGLES: FETCH EVERY CYCLE or every two cycles
//===========================================================================
//uncomment the next line to fetch a new instruction each processor clock cycle
//fetching every cycle does not help when executing FPU instructions, as we
//have to wait 7 cycles anyway. At the same time, fpga clock frequency is reduced
//when fetching every cycle due to more complex fetch stage!
//#define FETCH_EVERY_CYCLE
//===========================================================================
//IMPORTANT TOGGLES: REVERSE MAIN LOOP ORDER 
//===========================================================================
//uncomment the next line to reverse order of pipeline stages in main loop
//this is absolutely requires to be able to fetch every cycle, however it slows down!
//the fpga clock frequency if fetching every 2 cycles!! SO, if FETCH_EVERY_CYCLE
//if FETCH_EVERY_CYCLE is NOT defined, REVERSE_MAIN_LOOP should also NOT be defined!
//#define REVERSE_MAIN_LOOP
//#ifdef FETCH_EVERY_CYCLE
//#define REVERSE_MAIN_LOOP
//#endif
//==================================
// FPGA SELECTION
//==================================
//comment the next lines to set interfaces for ALVEO instead of MINIZED or KRIA KR260
//BE CAREFUL: only works WITH FPU
//#define MINIZED_INSTEAD_OF_ALVEO
//#define KRIA_INSTEAD_OF_ALVEO
//==================================
// COPY CODE MEM TO DATA MEM?
//==================================
//uncomment the next line to copy CodeMem into DataMem before executing something; this gets executed in testbench
//BE CAREFUL: ONLY TESTED WITH AD/ DOES SOMETHING WHEN AD IS ENABLED - UNCLEAR IF WORKING IF AD IS DISABLED
//on per default!
//#define UNIFIED_MEMORY
//==================================
// USE BOTH, GLOBAL AND LOCAL MEMORY?
//==================================
//uncomment the next line to also generate a local memory portion in the non-AD code
//off per default! actually makes the non-AD-Code less performant
//#define LOCAL_MEMORY
//==================================
// USE ROUNGIN MODES (AUTHOR: STEFAN REMKE)
//==================================
//uncomment the next line to use FPU rounding modes (also in AD unit)
//off per default! Requires FPU and AD
//#define ROUNDING_MODES
//==================================
// LIMIT CYCLES OF EXECUTION
//==================================
//comment the next line to limit
//execution to fixed number of clock cycles
//#define LIMIT_CYCLES
//#define MAX_CYCLES 200 //128	//8192
//==================================
// EXTEND OR SHORTEN MULTICYCLE PIPELINE COUNTER
//==================================
//uncomment the next line to use increase cycle counter limits
//impacts FPU, MUL and DIV
//#define USE_MORE_CYCLES_FOR_EXEC
//#define USE_LESS_CYCLES_FOR_EXEC
//===========================================================================
// IMPORTANT TOGGLES: FPU
//===========================================================================
//uncomment the next line to use only rv32im instead of rv32imf
//#define FPU
//==================================
//uncomment the next line to add experimental attempt to lock FCSR
//TODO: This is STILL incomplete
//#define LOCK_FCSR
//==================================
//TODO: Re-evaluate what this does...
//Assumption: uncomment the next line to turn on
//FPU separate cycle counter for FPU
//!! works only if AD is DISABLED, otherwise uses external anyway!!
//DEFINED by DEFAUL (I think)
//#define EXTERNAL_FPU_COMP
//==================================
//comment the next line to turn on
//old running condition check based on RET
//!!! ATTENTION DOES NOT WORK WITH ALVEO U50!!
//#define RUNNING_CONDITION_EXIT
//===========================================================================
//IMPORTANT TOGGLES: AD
//===========================================================================
//comment the next line to turn off AD-ADAPTIONS
//BE CAREFUL: only works WITH FPU
//#define AD
//===========================================================================
//IMPORTANT TOGGLES: PRAGMAS
//===========================================================================
//TODO: test if this is required to achieve more than p=3!
//uncomment the next line to try to put the whole local data memory in BRAM
//#define PRAGMA_USE_BRAM_ALVEO
//define the next to automatically 
//#ifdef PRAGMA_USE_BRAM_ALVEO
//#define ARRAY_PARTITION_FACTOR 1
//#endif
//All three of the following defines were commented out in the 
// reference build from 06.11.
//==================================
//uncomment the next line to add #pragma LATENCY max=1
//#define PRAGMA_USE_LATENCY
//==================================
//uncomment the next line to change the #pragma INLINE off to #pragma INLINE 
//within execute1 stage, e.g., for 
//ATTENTION! this ruins II=2 for RV32IM -> thus theses function may NOT be inlined
//#define PRAGMA_USE_INLINE_IN_MPP_EXECUTE
//==================================
//uncomment the next line to add an additional #pragma DATAFLOW to
//top-level function and the main processor loop
//ATTENTION! Only works with for loops and not with do .. while loops as in this code
//#define PRAGMA_USE_DATAFLOW
//===========================================================================
//IMPORTANT TOGGLES: EXPERIMENTAL
//===========================================================================
//THESE ARE TEMPORARY! SOLUTIONS TO TEST/ INCREASE PERFORMANCE
//==================================
//Try and simulate with __SYNTHESIS__ to see if important code is missing during simulation
//#define __SYNTHESIS__
//==================================
//uncomment the next line to use HLS intern library for FMADD etc
//could conflicts e.g. with #define MAX 1 and other defines from Bernard etc.
//!! TODO: if NOT using USE_HLS_MATH, QNAN check in all fmadd alternatives is not executed so far!!
//#define USE_HLS_MATH
//==================================
//uncomment the next line to use HLS bind_op pragma for div, mul and fpu instructions
//ON per DEFAULT
//#define HLS_USE_BIND_OP
//==================================
//uncomment the next line to use a trick, where within compute_div_1() instead of integer division
//float division is executed and typecast to speed up everything, especiall II
//ATTENTION: is only implemented for AD Version!!
//ATTENTION: Only effective, if only ONE! direction is used
//#define USE_FLOAT_DIV
//==================================
//uncomment the next line to use a simplified version for the computation of the derivatives of shift operations in integer
//arithmetics; if commented, it uses more logic ressources
//#define USE_INACCURATE_AD_FOR_SHIFTS
//==================================
//uncomment the next line to use new_cycle copying mechanism for is_full signals
//TODO: THIS does ONLY seem to work if NOT fetching every cycle (no matter which order of main loop!)
//#define USE_NEW_CYCLE_FOR_IS_FULL_SIGNALS


//===========================================================================
//IMPORTANT TOGGLES: AUTODEFINE!
//===========================================================================

//===========================================================================
//BE CAREFUL: AD only works WITH FPU - if you defined AD but NOT FPU, will be defined here anyways
#ifdef AD
#define FPU
#endif
//===========================================================================

//===========================================================================
//BE CAREFUL: AD only works WITH FPU - if you defined AD but NOT FPU, will be defined here anyways
#ifdef ROUNDING_MODES
#define FPU
#define AD
#endif

#ifdef PRAGMA_USE_BRAM_ALVEO
#ifndef ARRAY_PARTITION_FACTOR
#define ARRAY_PARTITION_FACTOR 1
#endif
#endif

#ifdef FETCH_EVERY_CYCLE
#define REVERSE_MAIN_LOOP
#endif

//===========================================================================

#ifdef AD
#define DIFFERENTIATE_BITWISE_OPS
#ifndef MAX_LOG_AD_DERIV_CNT
#define MAX_LOG_AD_DERIV_CNT  4 //1	|2 |3 |4 |5 |6 |7 |8
#endif
								//3 Bit max -> 2^3-1 = 7 derivatives
								//MAX_SIZE = 12!! equal to s_immediate wegen: typedef ap_int<12>
typedef ap_uint<MAX_LOG_AD_DERIV_CNT> max_der_t;
//TODO: can be computed automatically as well?
#ifndef MAX_LOG_AD_DERIV_CNT
#define AD_DERIV_CNT		  15 //1 |3 |7 |15 | 31 |63 |127 |255
#endif
								//max 2^MAX_LOG_AD_DERIV_CNT
								//Be careful - if you change MAX_LOG_AD_DERIV_CNT change AD_DERIV_CNT accordingly!!!
//Hack to use pragma with defines (in main to partition memory): https://support.xilinx.com/s/article/46111?language=en_US
#define PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) PRAGMA_SUB(x)
#endif


//==================================
//uncomment the next line to use smaller FPGAs with less Ressources
//Alternatively, if none are uncommented, ALVEO U50DD is Target
//If Minized or PYNQ-Z2 is target, then automatically use S_AXI interfaces
//as in Goossens Book
//#define KRIA_INSTEAD_OF_ALVEO
//#define MINIZED_INSTEAD_OF_ALVEO

//==================================
//uncomment the next line to use S_AXI Ports
//Alternatively, if not uncommented, ALVEO U50DD is Target
//and M_AXI Interface is used
//#define USE_S_AXI_INTERFACE

//#ifdef MINIZED_INSTEAD_OF_ALVEO
#if defined(MINIZED_INSTEAD_OF_ALVEO) || defined (KRIA_INSTEAD_OF_ALVEO)
#define USE_S_AXI_INTERFACE
#endif

//==========================================================
// MINIZED
//==========================================================
#ifdef MINIZED_INSTEAD_OF_ALVEO //MINIZED VERY RESTICTED! 
#ifndef LOG_CODE_MEM_SIZE
#define LOG_CODE_MEM_SIZE    16 //original: 11 //2^11=2048 Byte, 2KB, das ist nat. nicht viel ;) 2^13=8KB = 2K Instructions "cached" | 2^14=16KB = 4K Instructions "cached"
#endif
//size in bytes
#define CODE_MEM_SIZE       (1<<LOG_CODE_MEM_SIZE)
#ifndef LOG_DATA_MEM_SIZE
#define LOG_DATA_MEM_SIZE    17 //original: 11 //has to be at least one more than LOG_CODE_MEM_SIZE for stack //14 = 4K Data "cached" //15 = 8K * 4Byte Data "cached"
#endif
//size in bytes
#define DATA_MEM_SIZE       (1<<LOG_DATA_MEM_SIZE)
//==========================================================
// KRIA KR260
//==========================================================
#elif defined(KRIA_INSTEAD_OF_ALVEO) //MINIZED VERY RESTICTED!
#ifndef LOG_CODE_MEM_SIZE
#define LOG_CODE_MEM_SIZE    13 //2^15=32KB = 8K Instructions "cached" //2^13=8KB = 2K Instructions
#endif
//size in bytes
#define CODE_MEM_SIZE       (1<<LOG_CODE_MEM_SIZE)
#ifndef LOG_DATA_MEM_SIZE
#define LOG_DATA_MEM_SIZE    14 //16 = 64K Data "cached" = 16K * 4Byte Data "cached" //14 = 16K DataMem = 4K Floats(e.g.)
#endif
//size in bytes
#define DATA_MEM_SIZE       (1<<LOG_DATA_MEM_SIZE)
//==========================================================
// ALVEO - DEFAULT
//==========================================================
#else
#ifndef LOG_CODE_MEM_SIZE
#define LOG_CODE_MEM_SIZE    16//17 //original: 11 //2^11=2048 Byte, 2KB, das ist nat. nicht viel ;) 2^17=128KB = 32K Instructions "cached"
#endif
//size in bytes
#define CODE_MEM_SIZE       (1<<LOG_CODE_MEM_SIZE)
#ifndef LOG_DATA_MEM_SIZE
#define LOG_DATA_MEM_SIZE    17//18 //original: 11 //has to be at least one more than LOG_CODE_MEM_SIZE for stack //18 = 256K Data "cached" = 64K * 4Byte Data "cached"
#endif
//size in bytes
#define DATA_MEM_SIZE       (1<<LOG_DATA_MEM_SIZE)
#endif
#define LOG_REG_FILE_SIZE    5
#define NB_REGISTER         (1<<LOG_REG_FILE_SIZE)

#ifdef USE_MORE_CYCLES_FOR_EXEC
//==========================================================================================
//CYCLE COUNTER: MINE WITH EXTRA LARGE TIME BUFFER
//==========================================================================================
#define LOG_MUL_LATENCY      4
#define MUL_LATENCY         ((1<<LOG_MUL_LATENCY) - 4)//16 - 4 = 12 ( 24 FPGA cycles at II=2?)

#define LOG_DIV_LATENCY      6
#define DIV_LATENCY         ((1<<LOG_DIV_LATENCY) - 28)//64 - 28 = 100 (200 FPGA cycles at II=2?)
/*
#define LOG_DIV_LATENCY      7
#define DIV_LATENCY         ((1<<LOG_DIV_LATENCY) - 28)//128 - 28 = 100 (200 FPGA cycles at II=2?)
*/
#ifdef FPU
#define LOG_FPU_LATENCY      6
#define FPU_LATENCY         ((1<<LOG_FPU_LATENCY) - 28)//64 - 28 = 100 (200 FPGA cycles at II=2?)
#endif

#elif defined(USE_LESS_CYCLES_FOR_EXEC)

//==========================================================================================
//CYCLE COUNTER: MINE WITH EXTRA SMALL TIME BUFFER
//==========================================================================================

#define LOG_MUL_LATENCY      3
#define MUL_LATENCY         ((1<<LOG_MUL_LATENCY) - 6)//8 - 6 = 2 (2 FPGA cycles at II=1)

#define LOG_DIV_LATENCY      6
#define DIV_LATENCY         ((1<<LOG_DIV_LATENCY) - 20)//64 - 20 = 44 (40 FPGA cycles at II=1)

#ifdef FPU
#define LOG_FPU_LATENCY      4
#define FPU_LATENCY         ((1<<LOG_FPU_LATENCY) - 4)// 16 - 4 = 12 (12 FPGA cycles at II=1)
#endif

#endif


//==========================================================================================
//BERNARDS VERSION
//==========================================================================================
//#define LOG_MUL_LATENCY      3
//#define MUL_LATENCY         ((1<<LOG_MUL_LATENCY) -  4)// 8- 4 =  4 ( 8 FPGA cycles at II=2?)
//#define LOG_DIV_LATENCY      5
//#define DIV_LATENCY         ((1<<LOG_DIV_LATENCY) - 12)//32-12 = 20 (40 FPGA cycles at II=2?)
//TODO: HOW MANY DO I NEED??? EXTENSIVE TESTING REQUIRED
//#ifdef FPU
//#define LOG_FPU_LATENCY     5
//#define FPU_LATENCY         ((1<<LOG_FPU_LATENCY) - 12)//32-8 = 20 (40 FPGA cycles at II=2?)
//#define LOG_FPU_LATENCY   3
//#define FPU_LATENCY       ((1<<LOG_FPU_LATENCY) - 4)	//8-4 = 4 (8 FPGA cycles at II=2?)
//#endif
//#endif



#if defined(LOCAL_MEMORY) && !defined (AD)
typedef struct data_mem_s{
	  char data_mem_0[DATA_MEM_SIZE/sizeof(int)] = {0};
	  char data_mem_1[DATA_MEM_SIZE/sizeof(int)] = {0};
	  char data_mem_2[DATA_MEM_SIZE/sizeof(int)] = {0};
	  char data_mem_3[DATA_MEM_SIZE/sizeof(int)] = {0};
} data_mem_t;
#endif

//==========================================================================================
//TODO: REMOVE ABOVE!!! It slows things down immensly
//==========================================================================================

#define RET            0x8067
#define EXIT		   0x2b	  //TODO: this is a temporary custom OPCODE (custom-1 0101011 and fnc3 = 000) to EXIT processing
#define NOP            0x13

#define RA             1

#define LOAD           0b00000
#define LOAD_FP        0b00001
#ifdef AD
#define AD_STORE       0b00010	//Using custom Opcode for this
#else
#define CUSTOM_0       0b00010
#endif
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

//TODO: This other_type = 0 now could be a problem
#define UNDEFINED_TYPE 0
#ifdef FPU
#define OTHER_TYPE     0
#else
#define OTHER_TYPE     7
#endif
#define R_TYPE         1
#define I_TYPE         2
#define S_TYPE         3
#define B_TYPE         4
#define U_TYPE         5
#define J_TYPE         6
#ifdef FPU
#define R4_TYPE        7
#endif
#ifdef AD
#define AD_S_TYPE      8 // because of this decoded_instruction.type needs 4 bit now
#endif

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

#define MUL            0
#define MULH           1
#define MULHSU         2
#define MULHU          3
#define DIV            4
#define DIVU           5
#define REM            6
#define REMU           7

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

#ifdef FPU

#define I_TYPE_NO_FP   0
#define I_TYPE_FP	   1
//#define I_TYPE_FCSR  2

#define WB_TYPE_NO_FP  0
#define WB_TYPE_FP	   1
//#define WB_TYPE_FCSR 2

#define RNE            0
#define RTZ            1
#define RDN            2
#define RUP            3
#define RMM            4
#define RINV1          5
#define RINV2          6
#define DYN            7

#define FLE            0
#define FLT            1
#ifdef USE_HLS_MATH
#define MMAX            1
#else
#define MAX            1
#endif
#define JN             1
#define FCLASS         1
#define CSRRW          1
#define JX             2
#define FEQ            2
#define CSRRS          2

#define FADD           0x00
#define FSUB           0x01
#define FMUL           0x02
#define FDIV           0x03
#define FSGNJ          0x04
#define FMIN_MAX       0x05
#define FSQRT          0x0B
#define FCMP           0x14
#define FCVT_W_S       0x18
#define FCVT_S_W       0x1A
#define FMV_X_W        0x1C
#define FMV_W_X        0x1E

#define NX             (1<<0)
#define UF             (1<<1)
#define OF             (1<<2)
#define DZ             (1<<3)
#define NV             (1<<4)

#define MINUS_INFINITY 0xFF800000
#define PLUS_INFINITY  0x7f800000
#define NEGATIVE_ZERO  0x80000000
#define POSITIVE_ZERO  0x00000000
#define MIN_NEG_RANGE  0x80000000
#define MAX_POS_RANGE  0x7FFFFFFF
#define EMAX           0x000000FE
#define EMIN           0x000000FF
#define QNAN           0x7FC00000
#endif

#ifdef AD
typedef ap_fixed<64,32,AP_TRN_ZERO,AP_WRAP> fixp64;
typedef ap_ufixed<64,32,AP_TRN_ZERO,AP_WRAP> ufixp64;
typedef ap_fixed<40,32,AP_TRN_ZERO,AP_WRAP> fixp40;
typedef ap_ufixed<40,32,AP_TRN_ZERO,AP_WRAP> ufixp40;
//typedef ap_fixed<8,1,AP_TRN_ZERO,AP_WRAP> fixp8;
typedef ap_ufixed<8,1,AP_TRN_ZERO,AP_WRAP> ufixp8;
//#define ln2 ((ufixp8)0.6931471805599453094172321214581)
#ifdef USE_HLS_MATH
#define my_ln2 0.6931471805599453094172321214581
#else
#define ln2 0.6931471805599453094172321214581
#endif
//static const fixp38 fixln2 = ((fixp38) 0.6931471805599453094172321214581);
//static const float fln2 = ((fixp38) 0.6931471805599453094172321214581);
#endif

typedef unsigned int                 instruction_t;
typedef ap_uint<LOG_CODE_MEM_SIZE>   code_address_t;
typedef ap_uint<LOG_DATA_MEM_SIZE>   data_address_t;
#ifdef AD
typedef ap_uint<4>                   type_t; //we need a new AD_S_TYPE
#else
typedef ap_uint<3>                   type_t;
#endif
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
#ifdef FPU
typedef ap_uint<5>                   func5_t;
typedef ap_uint<3>                   frm_t;
#endif
typedef ap_uint<7>                   func7_t;
typedef ap_uint<1>                   bit_t;
typedef ap_uint<64>                  counter_t;
typedef ap_uint<LOG_MUL_LATENCY>     mul_latency_t;
typedef ap_uint<LOG_DIV_LATENCY>     div_latency_t;
#ifdef FPU
typedef ap_uint<LOG_FPU_LATENCY>     fpu_latency_t;
#endif
#ifdef AD
typedef ap_uint<64>                  counter_t;
#endif

#ifdef AD
//TODO: huge problem - what with fp regs ? need fp-ad-regs as well?
typedef struct reg_s{
	int   reg_file[NB_REGISTER];
	float freg_file[NB_REGISTER];
//#pragma HLS ARRAY_PARTITION variable=reg_file dim=1 complete //function scope only
	int   ad_reg_file[NB_REGISTER][AD_DERIV_CNT];
	float ad_freg_file[NB_REGISTER][AD_DERIV_CNT];
//#pragma HLS ARRAY_PARTITION variable=reg_file dim=2 complete
	bit_t is_reg_computed[NB_REGISTER];
	bit_t is_freg_computed[NB_REGISTER];
//#pragma HLS ARRAY_PARTITION variable=is_reg_computed dim=1 complete
	int   fcsr;
	bit_t is_fcsr_computed;
    int   fflag;
} reg_t;

#ifdef LOCAL_MEMORY
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
#endif

#endif

typedef union float_int_u{
  unsigned int i;
#ifdef FPU
  float        f;
  struct part_s{
    unsigned int mantissa: 23;
    unsigned int exponent:  8;
    unsigned int sign    :  1;
  } part __attribute__ ((__packed__));
#endif
} float_int_t;

typedef struct decoded_control_s{
  bit_t is_branch;
  bit_t is_jalr;
  bit_t is_jal;
  //bit_t is_ret; //just a JALR
} decoded_control_t;

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
#ifdef FPU
  reg_num_t   rs3;
#endif
  func7_t     func7;
#ifdef FPU
  func5_t       func5;
  ap_uint<2>    func5l;
  ap_uint<3>    func5h;
#endif
  type_t      type;
  immediate_t imm;
  bit_t       is_rs1_reg;
  bit_t       is_rs2_reg;
#ifdef FPU
  bit_t		  is_rs3_reg;
#endif
  bit_t       is_r_type;
  bit_t       is_mul;
  bit_t       is_div;
  bit_t       is_load;
  bit_t       is_store;
#ifdef AD
  bit_t		  is_ad_store;
  bit_t		  is_ad_op;			//needed for writeback - if false, no writeback to ad-registers
  bit_t		  is_ad_load_move;	//needed for locking - if true, rs2 has to be locked to prevent RaW
#endif
  bit_t       is_branch;
#ifdef FPU
  bit_t       is_load_fp;
  bit_t       is_store_fp;
  bit_t       is_madd;
  bit_t       is_msub;
  bit_t       is_nmsub;
  bit_t       is_nmadd;
  bit_t       is_fused;
  bit_t       is_op_fp;
  bit_t       is_fsgnj;
  bit_t       is_fmin_max;
  bit_t       is_fsqrt;
  bit_t       is_fcmp;
  bit_t       is_fcvt_w_s;
  bit_t       is_fcvt_s_w;
  bit_t       is_fmv_x_w;
  bit_t       is_fmv_w_x;
  bit_t       is_float;
  bit_t       is_execpt_inst;
#endif
  bit_t       is_jal;
  bit_t       is_jalr;
  bit_t       is_ret;
  bit_t       is_lui;
  bit_t       is_op_imm;
  bit_t       is_system;
  bit_t       is_mem;
  bit_t       is_bjalr;
  bit_t       is_jump;
  bit_t       is_ctrl;
  bit_t       has_no_dest;
  ap_uint<2>  opch;
  ap_uint<3>  opcl;
  bit_t       is_null_rv2;
  bit_t       is_first_negative_rv2;
  bit_t       is_last_negative_rv1;
  bit_t       is_exit;
} decoded_instruction_t;


//TODO: struct in struct is working? performant?
#ifdef FPU
typedef struct fpu_inst_for_wb_s{
  //reg_num_t   rd;
  func3_t     func3;
  func5_t     func5;
  type_t      type;
  bit_t       is_load_fp;
  bit_t       is_store_fp;
  bit_t       is_store;
  bit_t       is_branch;
  bit_t       is_system;
  bit_t       is_op_fp;
  bit_t       is_fcmp;
  bit_t       is_fcvt_w_s;
  bit_t       is_fmv_x_w;
  bit_t       is_float;
  bit_t       is_execpt_inst;
  bit_t       has_no_dest;
  int 		  fcsr;
  float_int_t rv1;
  float_int_t rv2;
} fpu_inst_for_wb_t;
#endif

typedef struct from_f_to_f_s{
  bit_t          is_valid;
  code_address_t next_pc;
} from_f_to_f_t;

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
  float_int_t           rv1;
  float_int_t           rv2;
#ifdef AD
  float_int_t           ad_rv1[AD_DERIV_CNT];
  float_int_t           ad_rv2[AD_DERIV_CNT];
#endif
#ifdef FPU
  float_int_t			rv3;
#ifdef AD
  float_int_t           ad_rv3[AD_DERIV_CNT];
#endif
  int					fcsr;
#endif
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
#ifdef AD
  bit_t					is_ad_op;
#endif
  bit_t                 is_ret;
  float_int_t           result1;
  float_int_t           result2;
#ifdef AD
  float_int_t           ad_result1[AD_DERIV_CNT];
  float_int_t           ad_result2[AD_DERIV_CNT];
#endif
  bit_t       			is_exit;
#ifdef FPU
  fpu_inst_for_wb_t		wb_info_fpu; //this contains FCSR status register
#endif
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
#ifdef AD
  char                  ad_value_0[AD_DERIV_CNT];
  char                  ad_value_1[AD_DERIV_CNT];
  char                  ad_value_2[AD_DERIV_CNT];
  char                  ad_value_3[AD_DERIV_CNT];
#endif
  reg_num_t             rd;
#ifdef AD
  reg_num_t             rs2;
  int					rv1;
#endif
  bit_t                 has_no_dest;
  bit_t                 is_load;
  bit_t                 is_store;
#ifdef AD
  bit_t					is_ad_store;
  bit_t					is_ad_op;
  max_der_t				ad_channel;
#endif
  func3_t               func3;
#ifdef FPU
  fpu_inst_for_wb_t		wb_info_fpu;
#endif
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
#endif
} from_e_1_to_m_t;

typedef struct e_1_state_s{
  bit_t                 is_full;
  float_int_t           rv1;
  float_int_t           rv2;
#ifdef AD
  float_int_t           ad_rv1[AD_DERIV_CNT];
  float_int_t           ad_rv2[AD_DERIV_CNT];
#endif
#ifdef FPU
  float_int_t			rv3;
#ifdef AD
  float_int_t           ad_rv3[AD_DERIV_CNT];
#endif
  int					fcsr;
#endif
  code_address_t        pc;
  decoded_instruction_t decoded_instruction;
#ifndef __SYNTHESIS__
  instruction_t         instruction;
  code_address_t        target_pc;
#endif
} e_1_state_t;

typedef struct from_e_2_to_w_s{
  bit_t                 is_valid;
  float_int_t           result;
#ifdef AD
  float_int_t			ad_result[AD_DERIV_CNT];
#endif
  reg_num_t             rd;
#ifdef AD
  bit_t					is_ad_op;
#endif
  bit_t                 has_no_dest;
  bit_t                 is_ret;
  bit_t       			is_exit;
#ifdef FPU
  fpu_inst_for_wb_t		wb_info_fpu;
#endif
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
#ifdef AD
  bit_t					is_ad_op;
#endif
  bit_t                 is_ret;
  float_int_t           result1;
  float_int_t           result2;
#ifdef AD
  float_int_t           ad_result1[AD_DERIV_CNT];
  float_int_t           ad_result2[AD_DERIV_CNT];
#endif
  bit_t       			is_exit;
#ifdef FPU
  fpu_inst_for_wb_t		wb_info_fpu;
#endif
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
#ifdef AD
  char                  ad_value_0[AD_DERIV_CNT];
  char                  ad_value_1[AD_DERIV_CNT];
  char                  ad_value_2[AD_DERIV_CNT];
  char                  ad_value_3[AD_DERIV_CNT];
#endif
  ap_uint<2>            a01;
  bit_t                 a1;
  func3_t               msize;
  reg_num_t             rd;
  bit_t                 has_no_dest;
#ifdef AD
  bit_t					is_ad_op;
  bit_t					is_ad_store;
#endif
#ifdef FPU
  fpu_inst_for_wb_t		wb_info_fpu;
#endif
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
#endif
} from_m_to_w_t;

typedef struct m_state_s{
  bit_t                 is_full;
  data_address_t        address;
  char                  value_0;
  char                  value_1;
  char                  value_2;
  char                  value_3;
#ifdef AD
  char                  ad_value_0[AD_DERIV_CNT];
  char                  ad_value_1[AD_DERIV_CNT];
  char                  ad_value_2[AD_DERIV_CNT];
  char                  ad_value_3[AD_DERIV_CNT];
#endif
  reg_num_t             rd;
#ifdef AD
  reg_num_t             rs2;
  int					rv1;
#endif
  bit_t                 has_no_dest;
  bit_t                 is_load;
  bit_t                 is_store;
#ifdef AD
  bit_t					is_ad_store;
  bit_t					is_ad_op;
  max_der_t				ad_channel;
#endif
  func3_t               func3;
#ifdef FPU
  fpu_inst_for_wb_t		wb_info_fpu;
#endif
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
#endif
} m_state_t;

typedef struct w_state_s{
  float_int_t           value;
#ifdef AD
  float_int_t			ad_value[AD_DERIV_CNT];
  char                  ad_value_0[AD_DERIV_CNT];
  char                  ad_value_1[AD_DERIV_CNT];
  char                  ad_value_2[AD_DERIV_CNT];
  char                  ad_value_3[AD_DERIV_CNT];
#endif
  reg_num_t             rd;
  bit_t                 has_no_dest;
#ifdef AD
  bit_t                 is_ad_op;
  bit_t					is_ad_store;
#endif
  bit_t                 is_ret;
  bit_t                 is_exit;
#ifdef FPU
  fpu_inst_for_wb_t		wb_info_fpu;
#endif
#ifndef __SYNTHESIS__
  code_address_t        pc;
  instruction_t         instruction;
  decoded_instruction_t decoded_instruction;
  code_address_t        target_pc;
#endif
} w_state_t;


#ifdef __cplusplus
extern "C" {
#endif
void adrv32imf_mp_ip(
  //code_address_t start_pc,
  unsigned int  *start_pc_ptr,
#ifdef AD
  //instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  unsigned int   g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  char           g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
  char    		 g_ad_data_mem_0[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    		 g_ad_data_mem_1[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    		 g_ad_data_mem_2[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
  char    		 g_ad_data_mem_3[DATA_MEM_SIZE/sizeof(int)][AD_DERIV_CNT],
#else
#ifdef LOCAL_MEMORY
  //instruction_t  g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  unsigned int   g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  char           g_data_mem_0[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_1[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_2[DATA_MEM_SIZE/sizeof(int)],
  char           g_data_mem_3[DATA_MEM_SIZE/sizeof(int)],
#else
  //instruction_t  code_mem  [CODE_MEM_SIZE/sizeof(int)],
  unsigned int   g_code_mem  [CODE_MEM_SIZE/sizeof(int)],
  char           data_mem_0[DATA_MEM_SIZE/sizeof(int)],
  char           data_mem_1[DATA_MEM_SIZE/sizeof(int)],
  char           data_mem_2[DATA_MEM_SIZE/sizeof(int)],
  char           data_mem_3[DATA_MEM_SIZE/sizeof(int)],
#endif
#endif
  unsigned long int  *nb_instruction,
  unsigned long int  *nb_cycle);
#ifdef __cplusplus
}
#endif

#ifdef AD
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

//TODO: static or not? and why?
void copy_value_float(
  float a[],
  float *b,
  max_der_t length
  );


//TODO: static or not? and why?
void copy_array_float_int_t(
  float_int_t a[],
  float_int_t *b,
  max_der_t length
  );

//TODO: static or not? and why?
void copy_value_float_int_t(
  float_int_t a[],
  float_int_t *b,
  max_der_t length
  );

//TODO: static or not? and why?
void copy_int_array_to_float_int_t(
  float_int_t a[],
  int b[],
  max_der_t length);

//TODO: static or not? and why?
void copy_float_array_to_float_int_t(
  float_int_t a[],
  float b[],
  max_der_t length);

void copy_float_int_t_array_to_float(
  float a[],
  float_int_t b[],
  max_der_t length);

void copy_float_int_t_array_to_int(
  int a[],
  float_int_t b[],
  max_der_t length);

#endif

#endif
