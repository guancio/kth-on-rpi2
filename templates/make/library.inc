# -*- Makefile -*-

all: build build/lib$(TARGET).a
	ls -l build/lib$(TARGET)*

build/lib$(TARGET).a: $(OBJ_FILES_TARGET)
	$(AR) r $@ $^	
	$(OBJDUMP) -M reg-names-raw -D -x -t -r $@ > build/lib$(TARGET).asm	
	# these are just for control the real library size:
#	$(STRIP) build/lib$(TARGET).a -o build/lib$(TARGET).strip.a       
#	$(OBJDUMP) -M reg-names-raw -D -x -t -r build/lib$(TARGET).strip.a > build/lib$(TARGET).strip.asm
       
##
show: all
	less build/lib$(TARGET).asm
build:
	mkdir build
	
clean:
	rm -f $(OBJ_FILES_TARGET)
	rm -f $(OBJ_FILES_HOST)
	rm -rf build
