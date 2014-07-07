all: init clean test

init:
	cd ../../; sed -i.bak "s/^SOFTWARE/#SOFTWARE/g" target
	cd ../../; sed -i.bak "s/^#SOFTWARE = linux/SOFTWARE = linux/g" target

clean:
	cd ../..; make clean

test:
	cd ../..; make
	./deploy.sh;
	./run.sh

