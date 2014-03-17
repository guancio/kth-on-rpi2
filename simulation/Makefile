include ../target

ifeq ($(SIMULATION_OVP),1)
		SUBDIRS = ovp_arm_any
endif

all:
	 set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d ; done


clean:
	 for d in $(SUBDIRS); do $(MAKE) clean -C $$d ; done

test:
	echo no tests for simulation yet
