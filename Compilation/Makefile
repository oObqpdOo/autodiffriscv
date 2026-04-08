#https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
#https://github.com/johnwinans/riscv-toolchain-install-guide/

ARCH=/opt/riscv/bin/riscv32-unknown-elf
MARCH=rv32imf#d
MABI=ilp32f#ilp32d

CC=$(ARCH)-gcc
CXX=$(ARCH)-g++
AS=$(ARCH)-gcc
LD=$(ARCH)-ld
OBJCOPY=$(ARCH)-objcopy
OBJDUMP=$(ARCH)-objdump
SIZE=$(ARCH)-size
AR=$(ARCH)-ar
RANLIB=$(ARCH)-ranlib

TARGET_ARCH+=-march=$(MARCH)
TARGET_ARCH+=-mabi=$(MABI)

CFLAGS = -O3 $(TARGET_ARCH) -malign-data=natural -mpreferred-stack-boundary=4 -mno-strict-align -mtune=sifive-7-series #-g
CXXFLAGS = -c  $(TARGET_ARCH) -O3 -malign-data=natural -mpreferred-stack-boundary=4 -mno-strict-align -mtune=sifive-7-series #-malign-data=xlen -mpreferred-stack-boundary=4 -mstrict-align -mtune=sifive-3-series#-g
ASFLAGS = -Wall #-g

CFLAGS+=-Wall -Wextra -Wpedantic #-Werror
CFLAGS+=-g
CFLAGS+=-Wcast-align
CFLAGS+=-ffreestanding 
#CFLAGS+=-fno-pic
CFLAGS+=-fpic
#CFLAGS+=-fomit-frame-pointer
#https://stackoverflow.com/questions/71376269/how-to-not-link-impure-data-when-call-math-h
#https://stackoverflow.com/questions/56550457/what-are-the-possible-side-effects-of-using-gccs-fno-math-errno
CFLAGS+=-fno-math-errno
CXXFLAGS+=$(CFLAGS)

LDLIBS=-lc -lgcc -lm
LDFLAGS=
#LDFLAGS+=-nostdlib
#LDFLAGS+=-Wl,-T,vanilla.ld

ASFLAGS+=$(CFLAGS)
ASFLAGS+=-Wa,-alh=$@.lst,-L

OBJDUMPFLAGS=

TARGET_EXEC ?= exec.out

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
CPPFLAGS ?= $(INC_FLAGS) -MMD -MP 

CSOURCES=$(shell find $(SRC_DIRS) -name *.c -or -name *.cpp)
ASSEMBLY = $(CSOURCES:%=$(BUILD_DIR)/%.asm)
COBJECTS = $(CSOURCES:%=$(BUILD_DIR)/%.o)
DISASMC = $(CSOURCES:%=$(BUILD_DIR)/%.dis)

ASMSOURCES := $(shell find $(SRC_DIRS) -name *.s)
ASMOBJECTS = $(ASMSOURCES:%=$(BUILD_DIR)/%.o)
HEX = $(ASMSOURCES:%=$(BUILD_DIR)/%.hex)
DISASM = $(ASMSOURCES:%=$(BUILD_DIR)/%.dis)

HEX_CLEAN = $(CSOURCES:%=$(BUILD_DIR)/%.hexc)

all: OBJDUMPFLAGS+=-Mnumeric,no-aliases
all: OBJDUMPFLAGS+=-S -d
all: $(BUILD_DIR)/$(TARGET_EXEC) $(HEX) $(ASSEMBLY)

no_std: LDFLAGS+=-nostdlib
no_std: LDFLAGS+=-Wl,-T,vanilla.ld 
no_std: OBJDUMPFLAGS+=-d -j .text -j .plt
no_std: $(BUILD_DIR)/$(TARGET_EXEC) $(HEX) $(ASSEMBLY) $(HEX_CLEAN_ASM) $(HEX_CLEAN_C)

std: LDFLAGS+=-Wl,-T,vanilla_std.ld,--entry=0x00000000
std: OBJDUMPFLAGS+=-d -j .text -j .plt
std: $(BUILD_DIR)/$(TARGET_EXEC) $(HEX) $(ASSEMBLY) $(HEX_CLEAN_ASM) $(HEX_CLEAN_C)

$(ASSEMBLY):
	#directly from C-code files compiled assembly
	for i in $(CSOURCES); do\
		$(CC) -S $$i -o $(BUILD_DIR)/$$i.asm;\
	done 
	#from C-Obj files disassembly
	for i in $(COBJECTS); do\
		od -A x4 -t x1 $$i > $$i.hex;\
		$(OBJDUMP) --syms -S -d $$i > $$i.dis;\
	done
$(HEX):
	for i in $(ASMOBJECTS); do\
		od -A x4 -t x1 $$i > $$i.hex;\
		$(OBJDUMP) --syms -S -d $$i > $$i.dis;\
	done
$(HEX_CLEAN):
	# Author: Stefan Remke
	for i in $(CSOURCES); do\
		hexdump -ve '1/1 " %.8x,"' exec.out;\
	done
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
	$(OBJDUMP) $(OBJDUMPFLAGS) $(BUILD_DIR)/$(TARGET_EXEC) >  $(BUILD_DIR)/$(TARGET_EXEC).dis
	$(OBJDUMP) -s -EB -j .data -j .rodata -j .text $(BUILD_DIR)/$(TARGET_EXEC) >  $(BUILD_DIR)/$(TARGET_EXEC).full #objdumps section .text .rodata AND .data to exec.out.full
	$(OBJDUMP) $(OBJDUMPFLAGS) -dr $< > $<.lst
	$(SIZE) -x -A $@
# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@
# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)


-include $(DEPS)

MKDIR_P ?= mkdir -p

