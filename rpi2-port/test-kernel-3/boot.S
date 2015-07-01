.section ".text.startup"

.global _start
_start:
    mov sp,#0x8000
    bl kernel_main
hang: b hang

.global write_to_address
write_to_address:
    str r1,[r0]
    bx lr

.global read_from_address
read_from_address:
    ldr r0,[r0]
    bx lr

.global delay
delay:
    bx lr

.word 0x01020101 //Dummy word
.word 0x016f2818

