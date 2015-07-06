RPi2 accessories
======================

This folder contains test programs and a guide to set up OpenOCD for
JTAG debugging, communicate over the UART and remote boot via U-Boot.

For optimal ease of reading, open the guide in a .pdf reader which 
can display hyperlinks.

Dependencies you will need to use everything in this folder:
- CMake (for compiling the kernels): sudo apt-get install cmake
- ARM cross-compiler (for compiling the kernels): sudo apt-get install gcc-arm-none-eabi
- mkimage (for creating U-Boot images): sudo apt-get install mkimage
- A LaTeX distribution (for compiling the Guide): Your choice, although we recommend simply using TexMaker: sudo apt-get install texmaker
- The Raspberry Pi 2 firmware (to get things working on the RPi2): You can
download it to your current folder with: git clone https://github.com/raspberrypi/firmware

For the convenience of not swapping the SD card physically:
- U-Boot (the remote bootloader tool): You can download it to the current folder 
with: git clone http://git.denx.de/u-boot.git
- ARM cross-compiler for U-Boot (to compile U-Boot): apt-get install gcc-arm-linux-gnueabihf
- U-Boot tools (for configuring U-Boot): apt-get install u-boot-tools
- xinetd (to set up the server to remote boot from): sudo apt-get install xinetd
- tftpd (TFTP server): sudo apt-get install tftpd
- tftp (TFTP client): sudo apt-get install tftp

For serial communication:
- screen: sudo apt-get install screen

For JTAG debugging:
- OpenOCD dependencies (install these before installing OpenOCD): sudo apt-get install make libtool pkg-config autoconf automake texinfo
- GNU Debugger: sudo apt-get install gdb
- libFTDI driver: sudo apt-get install libusb-1.0-0
- OpenOCD: We recommend

		git clone git://git.code.sf.net/p/openocd/code
		./configure --enable-ftdi
		make
		make install


- OpenOCD: We recommend cloning as done below:

		git clone git://git.code.sf.net/p/openocd/code
		./configure --enable-ftdi
		make
		make install

All the test kernels are built using the 
commands:

    ./configure.sh
    make

in their respective directories. This will create an ELF in the current
directory, and an image file called "zImage" in the folder /tftpboot/ (at the
bottom of the directory tree) unless you are compiling a standalone kernel.
Make sure you have prepared the tftpboot folder in advance, or change the target
directory at the bottom of CMakeLists.txt.

The test kernels are all meant for use with U-Boot, unless they are suffixed 
with "-standalone", which means that the .img file created when compiling should simply replace kernel.img or
kernel7.img on a SD card with just the RPi2 firmware.

boot.scr is a U-Boot boot script which should first be made an image by
executing the command

    mkimage -A arm -O linux -T script -C none -n Boot Script -d boot.scr boot.scr.uimg

in this folder, and then placed in the root folder of the SD card, if you want to use U-Boot.
