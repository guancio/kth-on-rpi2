
SUBDIRS = minimal trusted dtest

all:
	 set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d ; done


clean:
	 for d in $(SUBDIRS); do $(MAKE) clean -C $$d ; done
