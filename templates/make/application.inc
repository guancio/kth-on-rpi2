# -*- Makefile -*-

OUTPUT=build/$(TARGET)

all: build $(OUTPUT).hex $(OUTPUT).bin
	ls -l $(OUTPUT)*
    
$(OUTPUT).hex: $(OUTPUT).strip.elf
	$(OBJCOPY) -O ihex $(OUTPUT).strip.elf $(OUTPUT).hex

$(OUTPUT).bin: $(OUTPUT).strip.elf
	$(OBJCOPY) -O binary $(OUTPUT).strip.elf $(OUTPUT).bin

$(OUTPUT).elf: $(OBJ_FILES_TARGET) Makefile
	$(LD) -T $(LDSCRIPT) -o $(OUTPUT).elf $(OBJ_FILES_TARGET) $(LFLAGS) $(LIBGCC) -Map=$(OUTPUT).map
	$(OBJDUMP) -M reg-names-raw -D -x -t -r $(OUTPUT).elf > $(OUTPUT).asm

$(OUTPUT).strip.elf: $(OUTPUT).elf
	$(STRIP) $(OUTPUT).elf -o $(OUTPUT).strip.elf
	$(OBJDUMP) -M reg-names-raw -D -x -t -r $(OUTPUT).strip.elf > $(OUTPUT).strip.asm	


##

show: all
	less $(OUTPUT).asm

##

build:
	mkdir build
clean:
	rm -f $(OBJ_FILES_TARGET)
	rm -rf build

        
        
