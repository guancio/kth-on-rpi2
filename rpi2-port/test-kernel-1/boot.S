.section ".text.startup"

//This sets the program entry point.
.global _start
_start:
    mov sp,#0x8000
    bl kernel_main
hang: b hang

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

.section ".text.magic"

.word 0x01020101 //Dummy word
//The magic word below needs to be placed at offset 0x24 from the start of the .img. Please use an hex editor and open the image file and verify that this is the case after compilation if you modify either boot.S or linker.ld.
.word 0x016f2818
