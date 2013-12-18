# -*- Makefile -*-

all: build build/lib$(TARGET)_host.a
             
build/lib$(TARGET)_host.a: $(OBJ_FILES_HOST)
	ar r $@ $^
	objdump -D -x -t -r $@ > build/lib$(TARGET)_host.asm	
	
       
        
