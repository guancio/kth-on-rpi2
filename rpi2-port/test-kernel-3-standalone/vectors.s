.globl _start
_start:
    mov sp,#0x8000
    bl kernel_main
hang: b hang

.globl write_to_address
write_to_address:
    str r1,[r0]
    bx lr

.globl read_from_address
read_from_address:
    ldr r0,[r0]
    bx lr

.globl delay
delay:
    bx lr
