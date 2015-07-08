#include <family_context_top.inc>
#include "soc_defs.h"

	.global impl_irq
	.global impl_fiq
	.global impl_swi
	.global impl_undef
	.global impl_dabort
	.global impl_pabort
	.extern hypercall_table
    .code 32
    .align 4
    .section .vector, "ax"

 /*
  * system exception handlers
  */
   
impl_irq:
    /* 1. save context */
    sub lr, lr, #4
    store_context

    /* 2. get current interrupt and call it */
    ldr r0, =INTC_VIRT_BASE
    ldr r0, [r0, #4 * INTC_SIR_IRQ_ADR]
    and r0, r0, #0x7f

    /* 3. jump to handler */
    ldr r3, =irq_function_table
    ldr r12, [r3, r0, lsl #2]
    blx r12

    /* 4. reset interrupt */
    ldr r0, =INTC_VIRT_BASE
    ldr r1, =INTC_CONTROL_NEWIRQAGR
    str r1, [r0, #4 * INTC_CONTROL]
        
    /* 5. restore context 
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
    
    #mrc p15, 0, r0, c6, c0, 0
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
//  ldr r4, =hypercall_table
//	ldr pc, [r4, r3, lsl #2] @Call hypercall routine
    bx r12
    /* returns by exception bottom */

/*
 * Exception handler literals
 *
 */
    .ltorg

#include "family_context_bottom.inc"
