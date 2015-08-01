/*
 * Boot code for slave guests starting on different ARMv7a cores
 *
 */

    .global impl_slave_reset

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

	.align 4
	.section .startup, "ax"

impl_slave_reset:
    /* Start in supervisor mode, disable interrupts. */
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK
	/* Stack pointer starts at the physical address of the hyper stack top. */
    /* ldr sp, =(__hyper_stack_top__ + HAL_OFFSET)*/ /* Switch to another hyper stack, which you create in the linker */

    /* Clean BSS. 
	bl arm_clear_bss */

	/* Setup pages and switch to virtual memory. */
	bl arm_setup_initial_slave_pt

	/* Switch to master page table */
	bl arm_reset_initial_pt

	/* From here on (more precisely, from a point at the end of above function), 
	 * you are in virtual memory! */

    /* Setup real stacks now, run core init and reclaim the initial stacks. */
	bl arm_setup_initial_stack

    /* Init rest of hypervisor in C. */
	/* start_ can be found in core/hypervisor/init.c */
    bl start_
    
    /* Should not be reached! */
	bl _hang
