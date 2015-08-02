# Board-related files

This folder contains sub-directories for every board the STH supports. Within
each sub-directory you can find board_mem.c which describes the memory addresses
from the board you want the hypervisor to be able to access, and board.c which
details ATAG initialization, if this is to be done manually. The code for the
Raspberry Pi 2 and for the Beagleboard is the most recent, so please look at in
those folders for examples.
