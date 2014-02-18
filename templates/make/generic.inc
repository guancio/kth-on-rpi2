# -*- Makefile -*-
# generic make


# defults
ASM_DIRS ?= $(SRC_DIRS)

##
## add dependencies
##
INC_DIRS += $(foreach dir,$(DEPENDS), $(dir)/include )
LIB_DIRS += $(foreach dir,$(DEPENDS), $(dir)/build )


##
## for host and target:
##

# get file lists from dirs defined by configuration
SRC_FILES += $(foreach dir,$(SRC_DIRS), $(wildcard $(dir)/*.c) )
ASM_FILES += $(foreach dir,$(ASM_DIRS), $(wildcard $(dir)/*.S) )

OBJ_FILES_NORM = $(SRC_FILES:.c=.o) $(ASM_FILES:.S=.o)

OBJ_FILES_HOST  = $(OBJ_FILES_NORM:.o=.ho)
OBJ_FILES_TARGET= $(OBJ_FILES_NORM:.o=.to)

# add include dirs to flags
INC_FLAGS += $(foreach dir,$(SRC_DIRS), -I$(dir))
INC_FLAGS += $(foreach dir,$(ASM_DIRS), -I$(dir))
INC_FLAGS += $(foreach dir,$(INC_DIRS), -I$(dir))

CFLAGS  += $(INC_FLAGS)
ASFLAGS += $(INC_FLAGS)

LFLAGS += $(foreach dir,$(LIB_DIRS), -L$(dir))
LFLAGS += $(foreach file,$(LIB_FILES), -l$(file))

CFLAGS += -g -Wall \
# CFLAGS += -Os -g3 -Wall \
#	-pedantic -Wall -Werror\
#	-O3 -fno-inline -finline-limit=1 --param max-unroll-times=0

ASFLAGS += -Os -g3 -c
LFLAGS += $(shell $(CC) -print-libgcc-file-name)


##
## target only
##
CFLAGS_TARGET = $(SRC_CFLAGS) -nostartfiles -nostdlib -ffreestanding -DCOMPILE_TARGET=1
ASFLAGS_TARGET += $(CFLAGS_TARGET)

##
## host only
##
CFLAGS_HOST = -DCOMPILE_HOST=1
ASFLAGS_HOST += $(CFLAGS_HOST)
