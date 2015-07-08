
#include <family_context_top.inc>
#include <cpu_gic.h>
#include "soc_defs.h"

    .global _interrupt_vector_table
    .extern interrupt_handlers
    .extern impl_reset

    .code 32
    .align 4

    .section .startup,"ax"


_interrupt_vector_table:
    b impl_reset
    b impl_undef
    b impl_swi
    b impl_pabort
    b impl_dabort
    nop
    b impl_irq
    b impl_fiq
 /*
  * system exception handlers
  */
   
impl_irq:
    /* 1. save context */
    sub lr, lr, #4
    store_context

    /* 2. get current interrupt and call it*/
    ldr r0, =GICC_BASE
    ldr r1, [r0, #GICC_IAR*4 ]
    and r4, r1, #0xFF

    ldr r0, =interrupt_handlers          
    ldr r5, [r0, r4, lsl #2]     
    mov lr, pc
    bx r5

       
    /* 3. acknowledge the interrupt now */
    ldr r0, =GICC_BASE
    str r4, [r0, #GICC_EOIR*4 ]

        
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
