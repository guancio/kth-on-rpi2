//An implementation of an interrupt vector table for BCM2836.

//The ARM Cortex-A7 processor(s) on the BCM2836 starts executing
//code that can be found in the entries of the IVT, instead of
//looking for addresses to exception handlers. 

//A (virtual) base address (defined as INC_VIRT_BASE in soc_defs.h),
//so we do not need this here.
//#define AIC_BASE 0xFFFFF000

//Offsets for registers. Divided by 4 since that gives the entry number.
/*
#define AIC_IVR (0x100 / 4)
#define AIC_IPR (0x10c / 4)

#define AIC_FVR (0x104 / 4)

#define AIC_IECR (0x120 / 4)
#define AIC_IDCR (0x124 / 4)
#define AIC_ICCR (0x128 / 4)
#define AIC_ISCR (0x12c / 4)

#define AIC_EOICR (0x130 / 4)
#define AIC_ISR (0x108 / 4)

#define AIC_ISR_IRQID_MASK 31
*/

#include "family_context_top.inc"
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

impl_irq:
    //1. Save context.
	//////////////////
    sub lr, lr, #4
    store_context
    
    //2. Get current interrupt and call it.
	///////////////////////////////////////
	//Load the virtual base address of the interrupt controller into
	//the register r0.
    ldr r0, =INTC_VIRT_BASE
	//At the virtual base address, go forward in memory
	//4*INTC_SIR_IRQ_ADR bytes, where INTC_SIR_IRQ_ADR is the
	//offset to the INTCPS_SIR_IRQ register, and load 32 bits at
	//that location into r1.
    ldr r0, [r0, #4 * INTC_SIR_IRQ_ADR]
	//Binary-AND r0 with 0111 1111: grab the first seven bits of r0.
	//The first seven bits of r0 is the numer of the currently
	//active IRQ.
    and r0, r0, #0x7f
    
    //3. Clear/ack interrupt in some versions, or
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

//Fast IRQ. We do not use this yet, so this is not needed yet.
impl_fiq:
/* TODO */
    b .


impl_pabort:
    sub lr, lr, #4
    store_context_keep_lr
    
    ldr r3, =family_callback_inst_abort        
    ldr r3, [r3]
    
    mrc p15, 0, r0, c6, c0, 0 //TODO: This last zero could be 0 or 2
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
    
    //Clear state of exclusive access monitors, used by kuser helpers
    clrex

    mrc p15, 0, r0, c6, c0, 0
    mrc p15, 0, r1, c5, c0, 0
    mov r2, lr
    
    ldr lr, =exception_bottom
    bx r3
    /* returns by exception bottom */

//Implementation of software interrupt.
impl_undef:
    store_context_keep_lr

    ldr r3, =family_callback_undef
    ldr r3, [r3]
    
    sub r2, lr, #4
    ldr r0, [r2]

    ldr lr, =exception_bottom
    bx r3
    /* returns by exception bottom */

//Implementation of software interrupt.
impl_swi:    
    store_context_keep_lr

    //Get SWI data
    ldr r12, =family_callback_swi
    ldr r12, [r12]    

    /* XXX: this only works from 32-bit mode */
	//Note: r11 is ARM frame pointer.
    sub r11, lr, #4    
    ldr r2, [r11]
	//bic is the "bit clear" operation, i.e. op1 "AND NOT" op2.
    bic r2, r2, #0xFA00B200

    ldr lr, =exception_bottom
    bx r12
    /* returns by exception bottom */

/*
 * Exception handler literals
 *
 */
    .ltorg
    
#include "family_context_bottom.inc"
