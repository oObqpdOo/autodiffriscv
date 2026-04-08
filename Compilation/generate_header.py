import sys
import os
import random

class AD_Code_Generator:  
    def __init__(self, a):
        self.a = a
    
    def generate_ad_c_files(self):
        os.system('make clean')
        stream = os.popen('make')
        output = stream.read()
        print(output)
        os.system('cp ./build/src/ad_c_code.c.o.dis ./ad_c_code.dis')
        os.system('cp ./build/exec.out.dis ./full_ad_c_code.dis')
        stream = os.popen('/opt/riscv/bin/spike /opt/riscv/riscv32-unknown-elf/bin/pk build/exec.out')
        output = stream.read()
        print(output)
        with open('control_out.txt', 'w') as f:    
            for i in output:
                f.write(i)
            f.close()
    
    def generate_ad_proc_files(self):
        os.system('make clean')
        stream = os.popen('make no_std')
        #stream = os.popen('make std')
        output = stream.read()
        print(output)
        os.system('cp ./build/src/ad_proc_code.c.o.dis ./ad_proc_code.dis')
        os.system('cp ./build/exec.out.dis ./full_ad_proc_code.dis')
        os.system('cp ./build/exec.out.full ./all_sections_ad_proc_code.dis')
            
    def generate_all_files(self):
        self.generate_ad_proc_files()
        self.generaterunnable_ad_proc_disassembly()
        
    def clean_all_files(self):
        os.system('make clean')
        os.system('rm *.dis')        
        os.system('rm *.txt')
        
    def change_endian(self, in_str):
        ba = bytearray.fromhex(in_str)
        ba.reverse()
        s = ''.join(format(x, '02x') for x in ba)
        return s
        
    def run_simulation(self):
        stream = os.popen('/opt/riscv/bin/spike /opt/riscv/riscv32-unknown-elf/bin/pk build/exec.out')
        output = stream.read()
        print(output)
        with open('control_out.txt', 'w') as f:    
            for i in output:
                f.write(i)
            f.close()
            
    def generaterunnable_ad_proc_disassembly(self):
        bytecode = []
        start_pc = "0x00000000"
        #try:
        with open('all_sections_ad_proc_code.dis', 'r') as f:    
            text = f.read().split("\n")
            for i in text:
                if not i == '':
                    if not ("./build/exec.out:" in i) and not ("Contents of section" in i) and (i.startswith(" ")):
                        i = i.split(" ")
                        bytecode.append("0x" + self.change_endian(i[2].replace(' ','')) + ",")
                        bytecode.append("0x" + self.change_endian(i[3].replace(' ','')) + ",")
                        bytecode.append("0x" + self.change_endian(i[4].replace(' ','')) + ",")                        
                        bytecode.append("0x" + self.change_endian(i[5].replace(' ','')) + ",")
            f.close()
        #except: 
        #    print("File ad_proc_code doesn't exist")
        with open('full_ad_proc_code.dis', 'r') as f:    
            lines = f.readlines()
            for line in lines:
                if line.find("<main>:") != -1:
                    #print(line)
                    start_pc = "0x" + line.replace(" <main>:", "").replace("\n", "")
            f.close()
        with open('runnable_ad_proc_code.h', 'w') as f:
            #f.write("#ifndef __TEST_MEM\n#define __TEST_MEM\n\n#include \"../../ProcessorCode/adrv32imf_mp_ip.h\"\n\ninstruction_t code_mem[CODE_MEM_SIZE/sizeof(int)] = {\n")
            f.write("#ifndef __TEST_MEM\n#define __TEST_MEM\n\n#include \"../../Vitis_HLS/ProcessorCode/adrv32imf_mp_ip.h\"\n\ninstruction_t code_mem[CODE_MEM_SIZE/sizeof(int)] = {\n")
            for i in bytecode:
                if not i == "0x," and i.startswith("0x"):
                    f.write(i)
                    f.write('\n')
            f.write("\n};\n")
            f.write("#define CUSTOM_START_PC\nunsigned int __start_pc = "+ start_pc + ";")
            f.write("\n\n#endif \n //ARRAY_SIZE: "+ str(len(bytecode)))
            f.close()
    
gen = AD_Code_Generator(0) #instructions, variables, derivatives, %shifts, muls?, float?, only32bit?
gen.clean_all_files()
gen.generate_all_files()
#gen.generaterunnable_ad_proc_disassembly()
#gen.run_simulation()

