#!/bin/bash
qemu-system-arm -M beaglexm -m 512 -sd ../../../sth_deps/beagle_sd_working.img -nographic
