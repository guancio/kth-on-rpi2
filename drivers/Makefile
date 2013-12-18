TARGET=drivers
SRC_DIRS += src 
INC_DIRS += include

DEPENDS += ../library/uc


# drivers are platform dependent:
include ../target
#include config/$(PLATFORM).cfg
include ../templates/platform/$(PLATFORM).cfg
include ../templates/cpu/$(PLATFORM_CPU).cfg

# get sources:

SRC_DIRS += src/$(PLATFORM)

# include templates
include ../templates/make/toolchain.inc
include ../templates/make/generic.inc
include ../templates/make/library.inc
