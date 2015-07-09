.section ".text.dummy"

//We need at least one dummy word, because the compiler will remove zeroes at 
//the start of the program.
.word 0x11201045 //Dummy word

.section ".text.magic"

//The magic word below needs to be placed at offset 0x24 from the
//start of the .img. Please use an hex editor and open the image
//file and verify that this is the case after compilation if you
//modify either boot.S or linker.ld.

.word 0x11201045 //Dummy word
.word 0x016f2818

.section ".text.startup"

//This sets the program entry point.
//WARNING: Do NOT change this code, or the magic number might jump out of place.
.global _start
_start:
    mov sp,#0x8000
    bl kernel_main
hang: b hang

.section ".text.functions"

//Do a nonsense operation one cycle.
//Needed for when we want to do nothing for x number of cycles.
.global delay
delay:
    bx lr

//Writes to an address
.global write_to_address
write_to_address:
    str r1,[r0]
    bx lr

//Reads from an address
.global read_from_address
read_from_address:
    ldr r0,[r0]
    bx lr
