
SUBDIRS = minimal trusted dtest dtest1 dtest2 dtest3

all:
	 set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d ; done


clean:
	 for d in $(SUBDIRS); do $(MAKE) clean -C $$d ; done
