/* This is the entry point of the SICS Thin Hypervisor */
/*
 * boot code from ARMv7a
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
  * system exception handlers.
  *
  * we use the slow absolute jump since this code will be relocated to 0xFFFF0000
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
    /* Start in supervisor mode, disable interrupts */
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK
    ldr sp, =(__hyper_stack_top__ + HAL_OFFSET)

    /* Guest is now loaded by smartLoader peripheral (OVP bootloader) */
    bl  arm_move_guest_blob

    /* clean BSS */
	bl arm_clear_bss

	/* Setup pages and switch to virtual memory*/
	bl arm_setup_initial_pt

    /* setup real stacks now, run core init and reclaim the initial stacks */
	bl arm_setup_initial_stack

    /* Init rest of hypervisor in C */
    bl start_
    
    /* Should not be reached */
_hang:
	bl .
