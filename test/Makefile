
#SUBDIRS = simulation library core drivers
SUBDIRS = uclib utillib memlib 

all:
	 -set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d ; done


clean:
	 for d in $(SUBDIRS); do $(MAKE) clean -C $$d ; done


test:
	@make clean
	@make all
	@clear		
	for d in $(SUBDIRS); do $(MAKE) test -C $$d; done
	@echo ""
	@echo " *** WARNING: when runing on host, many standard library functions will not be ***"
	@echo " ***          tested correcty. Make sure you run these tests on target too     ***"
	@echo
