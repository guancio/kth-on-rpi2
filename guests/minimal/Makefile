TARGET=minimal
SRC_DIRS += src 

#INC_DIRS += ../../library/include include

DEPENDS = ../../library/uc ../../drivers ../../library/guest
LIB_FILES=guest uc drivers

LDSCRIPT = ../../library/guest/default.ld

# get platform dependent flags
include ../../target
include ../../templates/platform/$(PLATFORM).cfg
include ../../templates/cpu/$(PLATFORM_CPU).cfg

# include templates
include ../../templates/make/toolchain.inc
include ../../templates/make/generic.inc
include ../../templates/make/application.inc




# force update when something changes
$(OBJ_FILES): Makefile ../../target
