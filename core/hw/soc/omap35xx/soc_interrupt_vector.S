#include <family_context_top.inc>
#include "soc_defs.h"

//http://www.ti.com/lit/ug/spruf98y/spruf98y.pdf Chapter 10 contains
//information about the interrupt controller of OMAP 3550.

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

//System exception handlers:
  
//Implementation of normal interrupt handling.
//Occurs when IRQ input asserted.
impl_irq:
    //1. Save context.
	//////////////////
	//Link register (function return point) is decremented by 4.
    sub lr, lr, #4
	//Call store_context.
    store_context

    //2. Get current interrupt and call it.
	///////////////////////////////////////
	//Load the virtual base address of the interrupt controller into
	//the register r0.
    ldr r0, =INTC_VIRT_BASE
	//At the virtual base address of the IC, go forward in memory
	//4*INTC_SIR_IRQ_ADR bytes, where INTC_SIR_IRQ_ADR is the
	//offset to the INTCPS_SIR_IRQ register, and load 32 bits at
	//that location into r1.
    ldr r0, [r0, #4 * INTC_SIR_IRQ_ADR]
	//Binary-AND r0 with 0111 1111: grab the first seven bits of r0.
	//The first seven bits of r0 is the number of the currently
	//active IRQ.
    and r0, r0, #0x7f

    //3. Jump to handler.
	//Load the address of the IRQ function table (array with all
	//IRQ handlers) into r3.
    ldr r3, =irq_function_table
	//We use a scaled register offset to load the 32 bits at address
	//r3 + (r0 left shifted two positions (*2^2), zeros in new
	//positions) into r12.
	//Since r3 is the IRQ function table and r0 the active IRQ
	//number, we get the corresponding entry in the table.
	//We left shift because the table has 32-bit entries, but we
	//count in bytes.
    ldr r12, [r3, r0, lsl #2]
	//Branch to r12 with link, and exchange instruction set.
    blx r12

    //4. Reset interrupt.
	//Load the virtual base address of the interrupt controller into
	//the register r0.
    ldr r0, =INTC_VIRT_BASE
	//Store an "1" at bit 0 in r1 (rest zeroes).
    ldr r1, =INTC_CONTROL_NEWIRQAGR
	//Store r1 at the register with offset 4*INTC_CONTROL from the
	//virtual base address of the interrupt controller. This is the
	//INTCPS_CONTROL register and writing an "1" into bit 0 of this
	//register resets IRQ output and enables new IRQ generation.
    str r1, [r0, #4 * INTC_CONTROL]
        
    //5. Restore context.
	//Note: This code is shared with some other exception
	//handlers.     
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
