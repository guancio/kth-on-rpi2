
SUBDIRS = guesthelper

all:
	 set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d ; done


clean:
	 for d in $(SUBDIRS); do $(MAKE) clean -C $$d ; done


test:
	@make clean
	@make all
	@clear		
	for d in $(SUBDIRS); do $(MAKE) test -C $$d; done
