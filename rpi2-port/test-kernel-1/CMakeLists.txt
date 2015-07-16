cmake_minimum_required(VERSION 2.8)
project(test-kernel-1 C ASM)

#These are the compilation flags we want.
unset(CMAKE_C_FLAGS)
set(CMAKE_C_FLAGS "-O2 -mfpu=vfp -mfloat-abi=hard -march=armv7-a -std=c99 -mtune=cortex-a7 -nostartfiles -nostdlib -ffreestanding")

#We use the linker script linker.ld.
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-T,${PROJECT_SOURCE_DIR}/linker.ld")

add_executable(test-kernel-1
	kernel.c
	boot.S)

add_custom_command(
	TARGET test-kernel-1 POST_BUILD
	COMMENT "Converting the ELF output file to a binary image..." 
	COMMAND ${CMAKE_OBJCOPY} ./test-kernel-1${CMAKE_EXECUTABLE_SUFFIX} -O binary ./test-kernel-1.bin
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)

add_custom_command(
	TARGET test-kernel-1 POST_BUILD
	COMMENT "Converting the binary image into a uImage..." 
	COMMAND mkimage -A arm -O linux -T kernel -C none -a 0x1000000 -e 0x1000000 -n test-kernel-1 -d ./test-kernel-1.bin /tftpboot/uImage
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
