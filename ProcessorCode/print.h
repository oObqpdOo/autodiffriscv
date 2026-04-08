#ifndef __PRINT
#define __PRINT

#ifndef __SYNTHESIS__
#include "adrv32imf_mp_ip.h"

//register names are printed as x0, x1, x2 ...
//to print symbolic register names (zero, ra, sp ...)
//uncomment next line
//#define SYMB_REG

#ifdef FPU
//register names are printed as f1, f2, f3 ...
//to print symbolic register names (ft0, ft1, ft2 ...)
//uncomment next line
//#define SYMB_FREG
#endif

void print_reg_name(reg_num_t r);
void print_op_imm(func3_t func3, func7_t func7);
void print_msize(func3_t func3);
void print_branch(func3_t func3);
void print_reg(
#ifdef AD
		reg_t *reg_file_s
#else
		int *reg_file
#endif
		);

#ifdef FPU
void print_op(func3_t func3, func7_t func7, func5_t func5, opcode_t opcode, reg_num_t  rs2);
void print_freg_name(reg_num_t r);
void print_freg(
#ifdef AD
		reg_t *reg_file_s
#else
		float *freg_file
#endif
		);
void print_fcsr(int fcsr);
#else
void print_op(func3_t func3, func7_t func7);
#endif
#endif
#endif
