include ../target

ifeq ($(SIMULATION_OVP),1)
		SUBDIRS = ovp_integratorCP ovp_arm9 generic_a8
endif

all:
	 set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d ; done


clean:
	 for d in $(SUBDIRS); do $(MAKE) clean -C $$d ; done

test:
	echo no tests for simulation yet
