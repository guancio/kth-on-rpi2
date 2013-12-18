# -*- Makefile -*-


NOVLNV=1
IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")


include $(IMPERAS_HOME)/bin/Makefile.include

ifndef IMPERAS_HOME
  IMPERAS_ERROR := $(error "IMPERAS_HOME not defined")
endif


SRC =$(wildcard src/*.c) $(wildcard ../ovp_common/*.c)
ASM =$(wildcard src/*.S)
OBJ =$(SRC:.c=.o) $(ASM:.S=.o)
EXE ?=platform

all: $(EXE).exe

##

$(EXE).exe: $(OBJ)
	$(V) echo "Linking Simulation platform $@"
	$(V) gcc -m32 -g -o $@ $^ $(SIM_CFLAGS) $(SIM_LDFLAGS)

%.o: %.c
	echo "Compiling Simulation platform $@"
	gcc -m32 -g -w -c -o $@ $< $(IMPERAS_VMIINC)


clean:
	-rm $(EXE).exe 
#	-rm $(EXE).$(SHRSUF)
	-rm $(OBJ)
