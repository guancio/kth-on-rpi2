
#include <family_context_top.inc>
#include "soc_defs.h"

#    .extern interrupt_handlers
#    .extern impl_reset

	.global impl_irq
	.global impl_fiq
	.global impl_swi
	.global impl_undef
	.global impl_dabort
	.global impl_pabort

    .code 32
    .align 4
    .section .startup,"ax"


 /*
  * system exception handlers
  */
   
impl_irq:
    /* 1. save context */
    sub lr, lr, #4
    store_context

    /* 2. get current interrupt and call it */
    ldr r0, =INTC_BASE    
    ldr r0, [r0, #4 * INTC_SIR_IRQ_ADR]
    and r0, r0, #0x7f
    
   
    /* 3. jump to handler */
    ldr r3, =irq_function_table
    ldr r12, [r3, r0, lsl #2]
    blx r12
    
    /* 4. reset interrupt */
    ldr r0, =INTC_BASE
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
    
    mrc p15, 0, r0, c6, c0, 0
    mrc p15, 0, r1, c5, c0, 1    
    mov r2, lr
    
    ldr lr, =exception_bottom
    bx r3
    /* returns by exception bottom */

impl_dabort:
    sub lr, lr, #8
    store_context_keep_lr

    ldr r3, =family_callback_data_abort
    ldr r3, [r3]
    
    mrc p15, 0, r0, c6, c0, 0
    mrc p15, 0, r1, c5, c0, 0
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
    ldr r2, [r11]
    bic r2, r2, #0xFF000000

    ldr lr, =exception_bottom
    bx r12
    /* returns by exception bottom */

/*
 * Exception handler literals
 *
 */
    .ltorg
    
#include "family_context_bottom.inc"
