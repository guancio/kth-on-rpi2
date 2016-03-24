TARGET=testguest
SRC_DIRS += src 

DEPENDS = ../../library/uc ../../drivers ../../library/guest
LIB_FILES=guest uc drivers

LDSCRIPT = ../../library/guest/default.ld

# get platform dependent flags
include ../../target
include ../../templates/platform/$(PLATFORM).cfg
include ../../templates/cpu/$(PLATFORM_CPU).cfg

ifndef TEST_NAME
  TEST_NAME = TEST_UNDEFINED
endif
SRC_CFLAGS += -D$(TEST_NAME)=1

# include templates
include ../../templates/make/toolchain.inc
include ../../templates/make/generic.inc
include ../../templates/make/application.inc


# force update when something changes
$(OBJ_FILES): Makefile ../../target
