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

	/* TODO: The index of the core we are trying to start with the ASM under
	 * this label should be loaded into r0 at call. Here, it is hard-coded to
	 * be r0. */
	mov r0, #0x1

	/* Stack pointer starts at the physical address of the hyper stack top. */
	cmp r0, #0x1
	bne case_core_two
	ldr sp, =(__hyper_stack_top_core_1__ + HAL_OFFSET)
	b end_of_switch
case_core_two:
	cmp r0, #0x2
	bne case_core_three
	ldr sp, =(__hyper_stack_top_core_2__ + HAL_OFFSET)
	b end_of_switch
case_core_three:
	cmp r0, #0x3
	bne invalid_core
	ldr sp, =(__hyper_stack_top_core_3__ + HAL_OFFSET)
	b end_of_switch
invalid_core:
	/* TODO: Here, we can put a call to a general error handler to display a
	 * message about initialization from an invalid core number. */
end_of_switch:
    /* Clean BSS. 
	bl arm_clear_bss */

	/* Setup pages and switch to virtual memory. */
	bl arm_setup_initial_slave_pt
	/* From here on (more precisely, from a point at the end of above function), 
	 * you are in virtual memory! */

	/* Switch to master page table */
	bl arm_reset_initial_pt

    /* Setup real stacks now, run core init and reclaim the initial stacks. */
	mov r0, #0x1
	bl arm_setup_initial_stack

    /* Init rest of hypervisor in C. start_ can be found in
	 * core/hypervisor/init.c */
    bl slave_start_
    
    /* Should not be reached! */
	bl _hang
