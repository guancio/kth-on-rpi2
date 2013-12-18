# -*- Makefile -*-
#
# this is a makefile for local test
#

# inject our own dependencies
 # (todo) 
 

# needed for winodws ...
CC = gcc -m32

# INPUT AND OUTPUT
TEST_FILES = $(wildcard src/*.c)
EXE_FILES = $(patsubst src/%.c, build/%.exe , $(TEST_FILES) )

# add dependencies
INC_DIRS += $(foreach dir,$(DEPENDS), $(dir)/include )
LIB_DIRS += $(foreach dir,$(DEPENDS), $(dir)/build )

# flags
CFLAGS += -DTEST=1 
CFLAGS += $(foreach dir,$(INC_DIRS), -I$(dir))

LFLAGS += $(foreach dir,$(LIB_DIRS), -L$(dir))
LFLAGS += $(foreach fil,$(LIB_FILES), -l$(fil)_host)

#
all: $(EXE_FILES)

run: $(EXE_FILES)
	for d in $(EXE_FILES); do $$d || break ; done
	@echo "TESTING MINIC: ALL DONE"

test: run

##
build/%.exe: src/%.o $(OBJ_FILES) build
	$(CC) $<  $(OBJ_FILES) $(LFLAGS)  -o $@

##
build:
	mkdir build

clean:
	rm -rf build
