TARGET=uc
SRC_DIRS = src include

# get platform dependent flags
include ../../target
include ../../templates/platform/$(PLATFORM).cfg
include ../../templates/cpu/$(PLATFORM_CPU).cfg


# include templates
include ../../templates/make/toolchain.inc
include ../../templates/make/generic.inc
include ../../templates/make/library.inc


# for host build
include ../../templates/make/library_host.inc
