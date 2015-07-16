.section ".text.startup"

//This sets the program entry point.
.global _start
_start:
    mov sp,#0x1000000
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
