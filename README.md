# ad-rv32imf

## Disclaimer: This is a temporary, anonymous repository for submission to ASAP 2026. 
### The real repository will be revealed upon acceptance. 

Many thanks to Bernard Goossens for providing early prototypes and for the excellent description of the underlying microarchitecture in the book:   
   
Guide to Computer Processor Architecture, Bernard Goossens,  
Springer, ISBN: 978-3-031-18023-1, Published: 25 January 2023

## Installation of RISC-V gcc toolchain and requirements

```
sudo apt-get install autoconf automake autotools-dev curl python3 python3-pip libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev libtinfo5 libncurses5 libncurses5-dev libncursesw5-dev device-tree-compiler git pkg-config libstdc++6:i386 libgtk2.0-0:i386 dpkg-dev:i386  
cd  
git clone https://github.com/riscv/riscv-gnu-toolchain  
cd $HOME/riscv-gnu-toolchain  
./configure --prefix=/opt/riscv --enable-multilib --with-arch=rv32imf  
sudo make  
PATH=$PATH:/opt/riscv/bin  

cd  
git clone https://github.com/riscv/riscv-isa-sim  
export RISCV=/opt/riscv  
cd $HOME/riscv-isa-sim  
mkdir build  
cd build  
../configure --prefix=$RISCV --with-isa=rv32imf  
make  
sudo make install   

cd  
git clone https://github.com/riscv/riscv-pk  
export RISCV=/opt/riscv  
cd $HOME/riscv-pk  
mkdir build  
cd build  
../configure --prefix=$RISCV --host=riscv32-unknown-elf --with-arch=rv32imfd\_zifencei\_zicsr --with-abi=ilp32f  
make  
sudo make install  
```
==================================================

## Installation of Vitis toolchain and project setup

### Recommended installation: Vitis 2024.2 on Ubuntu 20.04.06 LTS!  

Before installing Vitis, install the following dependencies on Ubuntu:

```
sudo apt-get update  
sudo apt install libncurses5 libtinfo5 libncurses5-dev libncursesw5-dev  
sudo apt-get install libtinfo-dev libncurses-dev gitk libdpkg-perl git-gui libstdc++6 libusb-dev libgtk2.0-0 libc6-dev-i386 dpkg-devn libegl-mesa0 libegl1-mesa libgbm1 python3-apport fxload build-essential libgl1-mesa-glx libgl1-mesa-dri libgl1-mesa-dev opencl-clhpp-headers  
sudo add-apt-repository --remove ppa:xorg-edgers/ppa    
sudo apt install net-tools   
sudo apt-get install -y unzip   
sudo apt install gcc g++   
```
set up XILINX_VITIS and XILINX_VIVADO variables and XILINX_XRT by entering into ```.bashrc```

```  
nano ~/.bashrc
```   
```
source <Vitis_install_path>/Vitis/2024.2/settings64.sh   
source /opt/xilinx/xrt/setup.sh   
```
```
source ~/.bashrc   
```

### install cable_drivers    
```
sudo ./tools/Xilinx/Vivado/2024.2/data/xicom/cable_drivers/lin64/install_script/install_drivers  
```
  
### install board files (for Pynq or MiniZed FPGA only)
```
sudo cp -r ~/Downloads/pynq-z2 /tools/Xilinx/Vivado/2024.2/data/boards/  
sudo chmod 775 /tools/Xilinx/Vivado/2024.2/data/boards/pynq-z2  
sudo cp -r ~/Downloads/pynq-z2 /tools/Xilinx/Vitis/2024.2/data/boards/  
sudo chmod 775 /tools/Xilinx/Vitis/2024.2/data/boards/pynq-z2  
``` 

### install board files (for Alveo U50)

follow the instructions at [Xilinx (link)](https://www.amd.com/de/support/downloads/alveo-downloads.html/accelerators/alveo/u50.html#alveotabs-item-vitis-tab)

## wrapping up changes to ```.bashrc```

```  
nano ~/.bashrc
```   
```
source /tools/Xilinx/Vitis/2024.2/settings64.sh
source /tools/Xilinx/Vivado/2024.2/settings64.sh
export XILINX_VITIS="/tools/Xilinx/Vitis/2024.2"

#############################################################  

#export PATH=/path/to/riscv-gnu-toolchain/bin:$PATH
#export RISCV=/path/to/riscv-isa-sim
#export PATH=/path/to/riscv-isa-sim/build:$PATH
#export PATH=/path/to/riscv-pk/build:$PATH

export PATH=$PATH:/opt/riscv/bin
export PATH=$PATH:~/riscv-pk/bin
export RISCV=/opt/riscv

. "$HOME/.cargo/env"

source /opt/xilinx/xrt/setup.sh
source $XILINX_VITIS/settings64.sh

export PLATFORM_REPO_PATHS=$ALVEO_PLATFORM_INSTALLATION_DIRECTORY
export LIBRARY_PATH=/usr/lib/x86_64-linux-gnu

```
```
source ~/.bashrc   
```

# Project Setup with Vitis
  
start Vitis 2024.2 via:
```
vitis
```

(If using older 2023.1.1 Vitis Unified IDE: start Vitis with ```vitis -new``` (for Alveo FPGA) or ```vitis``` (for Pynq FPGA), not recommended)

### create HLS component 
- add content of ```/ProcessorCode/``` folder to ```src``` and set top-level function to ```adrv32imf_mp_ip```
- add content of ```/testbench/``` as testbench

### create Application Component  
- add content of ```/ApplicationCode/``` as ```src```

### create System Project
- create System Project for Alveo U50 with previously configured HLS component and Application Component

### (Optional: additional information for using Kria KR260 FPGA)

Running on Kria KR260 in Vitis 2023.2 in Vivado Flow ->  Configure Run Configuration:  
- Target Setup set checkmarks at:   
- Use FSBL flow for initialization  
- Reset Entire System  
- Program FPGA  
- Initialize Using FSBL  

### (Optional: Xilinx Licensing) 

/tools/Xilinx/Vivado/2024.1/bin/vlm  
Manage Licenses -> Load Licenses  


