#!/bin/bash

cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=./toolchain-arm-none-eabi.cmake -DCMAKE_C_FLAGS="-nostartfiles" ./
