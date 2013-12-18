# -*- Makefile -*-

# config toolchain and other stuff
TOOLCHAIN ?= arm-none-eabi-
CC =$(TOOLCHAIN)gcc
LD =$(TOOLCHAIN)ld
AS =$(TOOLCHAIN)gcc
AR =$(TOOLCHAIN)ar
STRIP =$(TOOLCHAIN)strip
OBJCOPY =$(TOOLCHAIN)objcopy
OBJDUMP =$(TOOLCHAIN)objdump
GDB =$(TOOLCHAIN)gdb
HEXDUMP=od -A x -t x4
MKIMAGE =mkimage
ECHO=/bin/echo -e


# rules:

%.ho: %.c
	gcc -m32 -c $(CFLAGS) $(CFLAGS_HOST) $< -o $@
 
%.to: %.c      
	$(CC) -c $(CFLAGS) $(CFLAGS_TARGET) $< -o $@


%.to: %.S
	$(CC) -c $(ASFLAGS) $(ASFLAGS_TARGET) $< -o $@
