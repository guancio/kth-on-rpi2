# -*- Makefile -*-
#
# this is a makefile for native executables
#


# needed for winodws ...
CC = gcc -m32

# INPUT AND OUTPUT
SRC ?= src
EXE ?= unnamed

EXE_DIR = ../../bin
SRC_FILES = $(foreach dir,$(SRC), $(wildcard $(dir)/*.c) )
EXE_FILE = $(EXE_DIR)/$(EXE).exe
#
all: $(EXE_FILE)

run: $(EXE_FILE)
	$(EXE_FILE)

$(EXE_FILE): $(EXE_DIR) $(SRC_FILES)
	$(CC) $(CFLAGS) $(SRC_FILES) -o $(EXE_FILE)

##
$(EXE_DIR):
	mkdir $(EXE_DIR)

clean:
	rm -rf $(EXE_DIR)

