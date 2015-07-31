/* This is the entry point of the SICS Thin Hypervisor */
/*
 * Boot code for ARMv7a
 *
 * NOTE: this file only contains the reset code, the
 * exception table and other exception handlers are under SoC
 */

    .global impl_reset
    .global _interrupt_vector_table

	.extern arm_move_guest_blob

    .extern start_
    .extern impl_undef
    .extern impl_swi
    .extern impl_pabort
    .extern impl_dabort
    .extern impl_irq
    .extern impl_fiq
    
    .code 32
    .align 0
    .section .vector,"ax"

#include "arm_common.h"

 /*
  * System exception handlers
  *
  * We use the slow absolute jump since this code will be relocated to 0xFFFF0000
  */

_interrupt_vector_table:
    b impl_reset        @ relative jump, this will be called from padr
    ldr pc, adr_undef
    ldr pc, adr_swi
    ldr pc, adr_pabort
    ldr pc, adr_dabort
    b .
    ldr pc, adr_irq
    ldr pc, adr_fiq

adr_fiq: 	.word impl_fiq
adr_irq: 	.word impl_irq
adr_swi: 	.word impl_swi
adr_pabort: .word impl_pabort
adr_dabort: .word impl_dabort
adr_undef: 	.word impl_undef

	.align 4
	.section .startup, "ax"

impl_reset:
    /* Start in supervisor mode, disable interrupts. */
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK
	/* Stack pointer starts at the physical address of the hyper stack top. */
    ldr sp, =(__hyper_stack_top__ + HAL_OFFSET)

	//TODO: TEST:
	/* Enables pins for writing and configures them for JTAG functionality.
	 * This is best put at the start so that you can debug the entire program. */
	bl debug_gpio_init

	//TODO: Currently we have no smartLoader or OVP, so what does the below line mean?
    /* Guest is now loaded by smartLoader peripheral (OVP bootloader). */
    bl  arm_move_guest_blob

    /* Clean BSS. */
	bl arm_clear_bss

	/* Setup pages and switch to virtual memory. */
	bl arm_setup_initial_pt
	/* From here on (more precisely, from a point at the end of above function), 
	 * you are in virtual memory! */

    /* Setup real stacks now, run core init and reclaim the initial stacks. */
	bl arm_setup_initial_stack

	//TEST
	/*
	ldr r0, =0x47 //Set r0 to 0x47.
debug_loop:
	cmp r0, #0x11 //Compare r0 to 0x11.
	blne debug_loop //Loop if r0 is not equal to 0x11.
	*/

    /* Init rest of hypervisor in C. */
	/* start_ can be found in core/hypervisor/init.c */
    bl start_
    
    /* Should not be reached! */
_hang:
	bl .
