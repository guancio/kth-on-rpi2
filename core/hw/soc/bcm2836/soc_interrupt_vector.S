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

//Implementation of normal interrupt handling.
//Occurs when IRQ input asserted.
impl_irq:
    //1. Save context.
	//////////////////
	//Link register (function return point) is decremented by 4.
    sub lr, lr, #4
	//Call store_context. //TODO: Why not store_context_keep_lr?
    store_context
    
	///////////
	//OLD CODE:
	///////////

    //TODO: 2. Get current interrupt.
	///////////////////////////////////////
	//Load the virtual base address of the interrupt controller into
	//the register r0.
    ldr r0, =INTC_VIRT_BASE
	//At the virtual base address of the IC, go forward in memory
	//4*INTC_SIR_IRQ_ADR bytes, where INTC_SIR_IRQ_ADR is the
	//offset to the INTCPS_SIR_IRQ register, and load 32 bits at
	//that location into r1.
	//TODO: There is no analogue in BCM2836.
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

    //TODO: 4. Reset interrupt.
	//Load the virtual base address of the interrupt controller into
	//the register r0.
    ldr r0, =INTC_VIRT_BASE
	//TODO: There is no analogue of the below for BCM2836...
	//Store a "1" at bit 0 in r1 (rest zeroes).
    ldr r1, =INTC_CONTROL_NEWIRQAGR
	//Store r1 at the register with offset 4*INTC_CONTROL from the
	//virtual base address of the interrupt controller. This is the
	//INTCPS_CONTROL register and writing an "1" into bit 0 of this
	//register resets IRQ output and enables new IRQ generation.
    str r1, [r0, #4 * INTC_CONTROL]

	//////////////////
	//NEW CODE (TODO):
	//////////////////

	//2. Decide if the interrupt is timer-related, UART-related, or neither.
	//IRQ Basic pending address: 0xFA00B200
	//IRQ pending address 0: 0xFA00B204
	//IRQ pending address 1: 0xFA00B208
	//In order to do this, we use a "switch" statement.
	//This switch statement can be easily expanded upon with more interrupts.
	//TODO: Replace hard-coded addresses with pre-DEFINEd ones.

		//Get the contents of the IRQ Basic pending register:
		ldr r0, #0xFA00B200
		ldr r0, [r0]

		//Get the contents of the IRQ pending register 1:
		ldr r1, #0xFA00B204
		ldr r1, [r1]

		//Get the contents of the IRQ pending register 2:
		ldr r2, #0xFA00B208
		ldr r2, [r2]

		case_1:
			//Check if the Basic pending register has a pending timer interrupt:
			//Binary AND with 1 in position 0, rest zeroes
			and r3, r0, #0x1
			cmp r3, #0x1 //If this is true then we have a timer interrupt
			//If true, Set r0 to IRQ number 64 (0 among basic registers)
			bne case_2 //Branches to next if cmp is false
			//Code for case 1 here
			ldr r0, #64
			//3. Jump to the correct handler.
			//Somewhat similar to the previous code
			//r0 has to be set to the IRQ number before this...
			ldr r3, =irq_function_table
			ldr r12, [r3, r0, lsl #2]
			blx r12

			//Clear pending timer interrupts.
			//Write 0x7FF to timer->tic
			ldr r0, #0xFA00B40C
			ldr r1, #0x7FF
			str r1, [r0]

			//branch to after_switch
			b after_switch
		case_2:
			//Check if the pending register 2 has a pending UART interrupt:
			//Binary AND with 1 in position 5, rest zeroess
			and r3, r2, #0x100000
			cmp r3, #0x100000 //If this is true then we have a UART interrupt
			//If true, Set r0 to IRQ number 57 (57 among "normal" registers)
			bne case_default //Branches to next if cmp is false
			//Code for case 2 here
			ldr r0, #57
			//3. Jump to the correct handler.
			//Somewhat similar to the previous code
			//r0 has to be set to the IRQ number before this...
			ldr r3, =irq_function_table
			ldr r12, [r3, r0, lsl #2]
			blx r12

			//Clear pending UART interrupts.
			//Write anything at all to uart->icr
			ldr r0, #0xFA201044
			ldr r1, #0x1
			str r1, [r0]

			//branch to after_switch
			b after_switch
		case_default:
			//TODO: What the heck do we do here???
			b after_switch
		after_switch:
			//TODO: What the heck do we do here???

		//5. Restore context.
		//Note: This code is shared with some other exception
		//handlers.
exception_bottom:
    restore_context
    movs pc, lr

//Fast IRQ. We do not use this yet, so this is not needed yet.
//Occurs when FIQ input asserted.
impl_fiq:
	/* TODO: Write this when we want to use FIQ. */
    b .

//Implementation of pre-fetch abort interrupt handling.
//Occurs when executing BKPT or instruction at invalid address.
impl_pabort:
	//1. Save context.
	//////////////////
	//Link register (function return point) is decremented by 4.
    sub lr, lr, #4
	//Call store_context_keep_lr.
    store_context_keep_lr
    
	//2. Get current interrupt.
	///////////////////////////////////////
	//Store address to interrupt data in r3.
    ldr r3, =family_callback_inst_abort
	//Resolve the value at the adress stored on the line above.     
    ldr r3, [r3]
	//This stores the instruction fault address register (IFAR) in 
	//r0.
	//(Co-processor 15 is the system control co-processor)
    mrc p15, 0, r0, c6, c0, 2
	//This stores the instruction fault status register (IFSR) in 
	//r1.
	mrc p15, 0, r1, c5, c0, 1
	//Copy the value of lr into r2.   
    mov r2, lr
    
	//3. Restore context.
	//Set link register to the address of exception_bottom.
    ldr lr, =exception_bottom
	//Branch to r3.
    bx r3
    /* returns by exception bottom */

//Implementation of data-abort interrupt handling.
//Occurs when reading from or writing to an invalid address.
impl_dabort:
	//1. Save context.
	//////////////////
	//Link register (function return point) is decremented by 4.
    sub lr, lr, #8
	//Call store_context_keep_lr.
    store_context_keep_lr

	//2. Get current interrupt.
	///////////////////////////////////////
	//Store address to interrupt data in r3.
    ldr r3, =family_callback_data_abort
	//Resolve the value at the adress stored on the line above.s
    ldr r3, [r3]
    
    //Clear state of exclusive access monitors, used by kuser helpers
    clrex
	//This stores the data fault address register (DFAR) in r0.
    mrc p15, 0, r0, c6, c0, 0
	//This stores the data fault status register (DFSR) in r1.
    mrc p15, 0, r1, c5, c0, 0
	//Copy the value of lr into r2. 
    mov r2, lr
    
	//3. Restore context.
	//Set link register to the address of exception_bottom.
    ldr lr, =exception_bottom
	//Branch to r3.
    bx r3
    /* returns by exception bottom */

//Implementation of undefined instruction interrupt handling.
//Occurs when executing undefined instruction code.
impl_undef:
	//1. Save context.
	//////////////////
	//Call store_context_keep_lr.
    store_context_keep_lr

    //2. Get current interrupt.
	///////////////////////////////////////
	//Store address to interrupt data in r3.
    ldr r3, =family_callback_undef
	//Resolve the value at the adress stored on the line above.
    ldr r3, [r3]
    
	//Link register (function return point) is decremented by 4 and
	//stored in r2.
    sub r2, lr, #4
	//Copy the value of r2 into r0. 
    ldr r0, [r2]

    //3. Restore context.
	//Set link register to the address of exception_bottom.
    ldr lr, =exception_bottom
	//Branch to r3.
    bx r3
    /* returns by exception bottom */

//Implementation of software interrupt handling.
//Occurs when executing SWI instruction.
impl_swi:    
    //1. Save context.
	//////////////////
	//Call store_context_keep_lr.
    store_context_keep_lr

    //2. Get current interrupt.
	///////////////////////////////////////
	//Store address to interrupt data in r3.
    ldr r12, =family_callback_swi
	//Resolve the value at the adress stored on the line above.
    ldr r12, [r12]    


    //TODO: The following only works in 32-bit mode.
	//But it should not be too hard to determine what mode you are 
	//operating in...
    sub r11, lr, #4    
	//Note: r11 is ARM frame pointer.
    ldr r2, [r11]
	//bic is the "bit clear" operation, i.e. op1 "AND NOT" op2.
	//This would copy the first byte octet in r2 back to r2 and
	//replace the rest of r2 with zeroes. 
    bic r2, r2, #0xFF000000


	//3. Restore context.
	//Set link register to the address of exception_bottom.
    ldr lr, =exception_bottom
	//Branch to r12.
    bx r12
    /* returns by exception bottom */

/*
 * Exception handler literals
 *
 */
    .ltorg
    
#include "family_context_bottom.inc"
