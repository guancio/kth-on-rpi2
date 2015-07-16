
# get current targets
include ../target


# declare base libraries
DEPENDS   += ../library/uc ../library/util ../library/mem 
LIB_FILES += uc util mem

# add our own files
SRC_DIRS += hypervisor lib hw/common hypervisor/guest_config hypervisor/hypercalls hypervisor/linux
INC_DIRS += hypervisor include lib hw/common hypervisor/guest_config 
ASM_DIRS += hypervisor/hypercalls hypervisor/linux


# load additional configuration
include config/hw/$(PLATFORM).cfg
include  $(foreach s,$(SOFTWARE), config/sw/$(s).cfg)
include ../templates/platform/$(PLATFORM).cfg
include ../templates/cpu/$(PLATFORM_CPU).cfg

# load the remaining config files
include ../templates/make/toolchain.inc
include ../templates/make/generic.inc

# some defaults
EXE ?= sth_$(PLATFORM)
HAL_PHYS_START ?= 0x00000000
HAL_PHYS_SIZE  ?= 0x01000000
HAL_VIRT_START  = 0xF0000000

SIMULATOR ?= @echo ERROR! NO SIMULATOR DEFINED FOR

# utils
UTILSHOME=../bin
GHELPER= $(UTILSHOME)/guesthelper.exe 

# add it to flags
USER_DEFS += HAL_PHYS_START=$(HAL_PHYS_START)
USER_DEFS += HAL_PHYS_SIZE=$(HAL_PHYS_SIZE)
USER_DEFS += HAL_VIRT_START=$(HAL_VIRT_START)

# USER_DEFS += GUESTS_POSITIONS="$(shell $(GHELPER) p $(GUESTS))"

FLAG_DEFS += $(foreach d, $(USER_DEFS), -D$(d))

CFLAGS += $(FLAG_DEFS)
ASFLAGS += $(FLAG_DEFS)


#Guest rules, names are hard-coded by .lds and other makefiles!
GUEST_ELFS=$(shell $(GHELPER) e $(GUESTS))
GUEST_BINS=$(shell $(GHELPER) b $(GUESTS))

GUEST_ASM = build/guest_data.S
GUEST_OBJ = build/guest_data.o


	
# build rules	
all: build/$(EXE).img build/$(EXE).bin build/$(EXE).fw build/$(EXE).fw.img

build/$(EXE).img: build/$(EXE).bin
	$(MKIMAGE) -A arm -O linux -T kernel -C none -a $(HAL_PHYS_START) -e $(HAL_PHYS_START) -n build/$(EXE) -d build/$(EXE).bin build/$(EXE).img

#TODO: Change target for this function back to "build/$(EXE).fw.img" from current one.
build/$(EXE).fw.img: build/$(EXE).fw
	$(MKIMAGE) -A arm -O linux -T kernel -C none -a $(HAL_PHYS_START) -e $(HAL_PHYS_START) -n build/$(EXE) -d build/$(EXE).fw /tftpboot/uImage

build/$(EXE).bin: build/$(EXE).elf
	$(OBJCOPY) build/$(EXE).elf -O binary build/$(EXE).bin
	$(HEXDUMP) build/$(EXE).bin > build/$(EXE).bin.hex		


build/$(EXE).elf: $(OBJ_FILES_TARGET) build $(LSCRIPT) Makefile  $(GUESTST_BIN)
	$(LD) -o $@ $(OBJ_FILES_TARGET) -T $(LSCRIPT) $(LFLAGS) -Map build/$(EXE).map
	# some extra stuff for debugging
	$(OBJDUMP) -M reg-names-raw -D -x -t -r build/$(EXE).elf > build/$(EXE).asm	
	$(STRIP) build/$(EXE).elf -o build/$(EXE).strip.elf
	$(OBJDUMP) -M reg-names-raw -d -x -t -r build/$(EXE).strip.elf > build/$(EXE).strip.asm
	
about:
	@echo PLATFORM=$(PLATFORM)
	@echo SOFTWARE=$(SOFTWARE)
	@echo TARGET_CPU=$(TARGET_CPU)
	@echo CFLAGS=$(CFLAGS)
	@echo LFLAGS=$(LFLAGS)
	@echo OBJ_FILE=$(OBJ_FILES_TARGET)
	
##


sim: build/$(EXE).elf  build/$(EXE).fw
	$(SIMULATOR) build/$(EXE).fw:$(HAL_PHYS_START) build/$(EXE).elf # $(GUEST_ELFS)
	-cat *.log


dbg: build/$(EXE).elf  build/$(EXE).fw
	$(SIMULATOR) --debug build/$(EXE).fw:$(HAL_PHYS_START) build/$(EXE).elf # $(GUEST_ELFS) 


qemu: build/$(EXE).fw.img $(QEMU_REQS)
	@echo ________________________________________
	@echo QEMU is started in dbg mode run
	@echo 'make ddd' in another console to 
	@echo connect to this gdb instance at :1234
	@echo !!! NOTE: terminate with 'Ctrl-A X' !!!!
	@echo ________________________________________
	$(QEMU) -no-reboot -s -S -kernel build/$(EXE).fw.img $(QEMU_ADDS)

## 
gdb: build/$(EXE).elf build/$(EXE).fw  
	@echo NOTE: you should have run 'make dbg' or 'make qemu' before this!
	@sleep 1
	$(TOOLCHAIN)gdb -ex "target remote :1234" build/$(EXE).elf 

ddd: build/$(EXE).elf build/$(EXE).fw
	@echo NOTE: you should have run 'make dbg' or 'make qemu' before this!
	@sleep 1	# give debugger time to start!
	ddd --debugger $(TOOLCHAIN)gdb --eval-command="target remote :1234" build/$(EXE).elf
#	ddd --debugger $(TOOLCHAIN)gdb  build/$(EXE).elf


# This unreadable snippet will create a new .S file containing all guest
# binaries. The corresponding .o file will be linked at the end of 
# out binary
$(GUEST_ASM): $(GUEST_BINS) Makefile ../target
	@../bin/guesthelper.exe a $(GUESTS) > $(GUEST_ASM)

build/$(EXE).fw: build/$(EXE).bin $(GUEST_OBJ)	
	$(OBJCOPY) $(GUEST_OBJ) -O binary $(GUEST_OBJ).bin
	cat build/$(EXE).bin $(GUEST_OBJ).bin > build/$(EXE).fw
	$(HEXDUMP) build/$(EXE).fw > build/$(EXE).fw.hex
	$(OBJDUMP) -M reg-names-raw -d -x -t -r $(GUEST_OBJ) > $(GUEST_OBJ).asm
	@echo "NOTE: created firmware by joining build/$(EXE).bin and $(GUEST_OBJ).bin"
	@ ls -l build/$(EXE).fw build/$(EXE).bin $(GUEST_OBJ).bin

##

# force re-build when makefile or target changes

Makefile: ../target
$(OBJ_FILES_TARGET): Makefile


##

show: build/$(EXE).elf
	less  build/$(EXE).asm
	
clean:
	-rm build/$(EXE)*
	-rm $(OBJ_FILES_TARGET)
	-rm $(GUEST_ASM)
	-rm $(GUEST_OBJ)
	-rm $(GUEST_OBJ).*
#	-make -C test clean

build:
	mkdir build
	
#
#.PHONY: test
#test:
#	@echo Compiling and running tests
#	@env SRC_DIRS="$(SRC_DIRS)" INC_DIRS="$(INC_DIRS)" make -B -C test
