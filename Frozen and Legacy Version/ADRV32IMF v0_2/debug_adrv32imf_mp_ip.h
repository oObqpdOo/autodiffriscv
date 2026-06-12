#ifndef __DEBUG_MULTICYCLE_PIPELINE_IP
#define __DEBUG_MULTICYCLE_PIPELINE_IP

//#include "adrv32imf_mp_ip.h"

//#define DEBUG_ADHOC             //Temporary Debugging print used for fast tests and debug prints

//register names are printed as x0, x1, x2 ...
//to print symbolic register names (zero, ra, sp ...)
//uncomment next line
#define SYMB_REG

#ifdef FPU
//register names are printed as f1, f2, f3 ...
//to print symbolic register names (ft0, ft1, ft2 ...)
//uncomment next line
#define SYMB_FREG
#endif

//==================================
//comment the next line to turn off
//pipeline debugging prints

//#define DEBUG_PIPELINE

//==================================
//comment the next line to turn off
//fetch debugging prints

//#define DEBUG_FETCH

//==================================
//comment the next line to turn off
//disassembling debugging prints

//#define DEBUG_DISASSEMBLE

//==================================
//comment the next line to turn off
//emulation debugging prints

//#define DEBUG_EMULATE

//==================================
//comment the next lines to turn off
//register locking debugging prints
//#define DEBUG_LOCK_REG
//#define DEBUG_LOCK_FREG
//#define DEBUG_LOCK_REG_MINIMAL

#ifdef DEBUG_LOCK_REG
#define DEBUG_LOCK_REG_MINIMAL
#endif
//==================================
//comment the next line to turn off
//register file dump debugging prints

//#define DEBUG_REG_FILE

//==================================


//EXPERIMENTAL - NOT RECOMMENDED FOR PRACTICAL USE
//==================================
//comment the next line to take Prof. Goossens
//Version, but it includes bugs in build_word and compute AUIPC
//#define USE_OLD_BUG
//==================================
//comment the next line to turn off
//spontaneos debugging prints
//#define DEBUG_ON_THE_FLY
//==================================
//comment the next line to turn off
//FPU flag debugging prints
//#define DEBUG_FPU_FLAGS
//==================================

//#ifdef FPU
//==================================
//comment the next line to turn off
//floating point register debugging prints

//#define DEBUG_FREG_FILE

//==================================
//comment the next line to turn off
//fcsr register debugging prints
//#define DEBUG_FCSR_REG
//==================================
//#endif

//#ifdef AD
//==================================
//comment the next line to turn off
//AD debugging prints
//#define DEBUG_AD
//==================================
//comment the next line to turn off
//custom exit command debugging prints
//#define DEBUG_EXIT
//==================================
//comment the next line to turn off
//custom compute command debugging prints
//#define DEBUG_OPS
//==================================
//comment the next line to turn off
//custom compute command debugging prints for M Extension
//#define DEBUG_MUL_OPS
//==================================
//comment the next line to turn off
//custom compute command debugging prints for F Extension
//#define DEBUG_FPU_OPS
//==================================
//comment the next line to turn off
//custom writeback stage command debugging prints
//#define DEBUG_WB
//==================================
//comment the next line to turn off
//custom writeback stage command debugging prints
//#define DEBUG_MEM
//==================================
//comment the next line to turn off
//custom execute2 stage command debugging prints
//#define DEBUG_EXE2
//==================================
//#endif

#endif
