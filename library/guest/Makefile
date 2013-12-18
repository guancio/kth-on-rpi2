TARGET= guest
SRC_DIRS += src 
INC_DIRS += include

#DEPENDS = ../../library/uc ../../library/mem

# get platform dependent flags
include ../../target
include ../../templates/platform/$(PLATFORM).cfg
include ../../templates/cpu/$(PLATFORM_CPU).cfg

# include templates
include ../../templates/make/toolchain.inc
include ../../templates/make/generic.inc
include ../../templates/make/library.inc
