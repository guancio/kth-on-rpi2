.section ".text.startup"

.global _start

.global _get_stack_pointer

_start:
    // Set the stack pointer, which progresses downwards through memory
    ldr     sp, =0x1000000
    sub     sp, sp, #0x4

    // Run the c startup function - should not return and will call kernel_main
    b       _cstartup

_inf_loop:
    b       _inf_loop


_get_stack_pointer:
    // Return the stack pointer value
    str     sp, [sp]
    ldr     r0, [sp]

    // Return from the function
    mov     pc, lr

.word 0x01020101 //Dummy word
.word 0x11111111 //Dummy word
.word 0x016f2818


