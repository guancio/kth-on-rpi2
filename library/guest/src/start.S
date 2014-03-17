
    .global _start
    .extern _main
    .extern lib_init
    .extern lib_atexit    
    .extern handler_rpc
    .extern guest_data
    
    .code 32
    .section .startup, "ax"
    .balign 4
    
    
_start:
start:
    /* jump table */
    b handler_reset
    b handler_rpc
    nop /* reserved */
    nop /* reserved */    
    nop /* reserved */
    nop /* reserved */
    nop /* reserved */
    nop /* reserved */
    
handler_reset:
    /* save guest data */
    ldr r8, =guest_data
    str r3, [r8, #0]
    str r4, [r8, #4]

    /* clean BSS */
    mov r0, #0
    ldr	r1,=__bss_start__
    ldr	r2,=__bss_end__
bss_loop:
    str r0, [r1], #4
    cmp r1, r2
    blo bss_loop
    

    /* set stack pointer at the end of the MB */

// 	ldr r1, = 0x80200000
//    ldr r1, =0x00200000
//	and r13, r15, #0xF0000000 @ Set SP relative to PC
	ldr sp, =__stack_top__;
	//add r13, r13, r1

    bl _main

    /* all done */
    b . /* TODO: a real exit */        
