#include "family_context_top.inc"
#include "soc_defs.h"


    .global impl_irq
    .global impl_fiq
    .global impl_swi
    .global impl_undef
    .global impl_dabort
    .global impl_pabort

    .code 32
    .align 4
    .section .vector,  "ax"

 /*
  * system exception handlers
  */
   
impl_irq:
    /* 1. save context */
    sub lr, lr, #4
    store_context
    
    /* 2. get current interrupt and call it */
    ldr r0, =PIC_VA_BASE
    ldr r0, [r0, #4 * IRQ_STATUS ] // nth bit is the interrupt source
	clz r4, r0
	mov r3, #31
	sub r0, r3, r4		//r0 contains interrupt number
	ldr r3, =irq_function_table
	ldr r12, [r3, r0, lsl #2]
	blx r12

    /* 3. clear interrupt */
	ldr r0, =PIC_VA_BASE
	ldr r1, =INTSRC_IRQ_TIMERINT1
	mov r2, #1
	lsl r1, r2, r1
	str r1, [r0, #4 * IRQ_ENABLESET] //
    /* 4. restore context 
     * note: this code is shared with some other
     * exception handlers */    
exception_bottom:
    restore_context    
    movs pc, lr
    
impl_fiq:
/* TODO */
    b .

impl_pabort:
    sub lr, lr, #4
    store_context_keep_lr
    
    ldr r3, =family_callback_inst_abort        
    ldr r3, [r3]
    
    mrc p15, 0, r0, c6, c0, 2	@get IFAR
    mrc p15, 0, r1, c5, c0, 1	@get IFSR
    mov r2, lr
    
    ldr lr, =exception_bottom
    bx r3
    /* returns by exception bottom */

impl_dabort:
    sub lr, lr, #8
    store_context_keep_lr

    ldr r3, =family_callback_data_abort
    ldr r3, [r3]

	/*Clear state of exclusive access monitors, used by kuser helpers*/
	clrex
    
    mrc p15, 0, r0, c6, c0, 0	@get FAR
    mrc p15, 0, r1, c5, c0, 0	@get FSR
    mov r2, lr
    
    ldr lr, =exception_bottom
    bx r3
    /* returns by exception bottom */

    
impl_undef:
    store_context_keep_lr

    ldr r3, =family_callback_undef
    ldr r3, [r3]
    
    sub r2, lr, #4
    ldr r0, [r2]

    ldr lr, =exception_bottom
    bx r3
    /* returns by exception bottom */
    
impl_swi:    
    store_context_keep_lr

    /* get SWI data */
    ldr r12, =family_callback_swi
    ldr r12, [r12]    

    /* XXX: this only works from 32-bit mode */
    sub r11, lr, #4    
    ldr r3, [r11]
    bic r3, r3, #0xFF000000

    ldr lr, =exception_bottom
    bx r12
    /* returns by exception bottom */

/*
 * Exception handler literals
 *
 */
    .ltorg
    
/*Put context elsewhere */
    .align 4
    .section .text

#include "family_context_bottom.inc"
