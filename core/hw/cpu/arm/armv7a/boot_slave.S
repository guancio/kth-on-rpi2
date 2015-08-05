/*
 * Boot code for slave guests starting on different ARMv7a cores
 */

    .global impl_core_1_reset
	.global impl_core_2_reset
	.global impl_core_3_reset
    
    .code 32
    .align 0
    .section .vector,"ax"

#include "arm_common.h"

	.align 4
	.section .startup, "ax"

impl_core_1_reset:
    /* Start in supervisor mode, disable interrupts. */
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK

	/* Stack pointer starts at the physical address of the hyper stack top. */
	ldr sp, =(__hyper_stack_top_core_1__ + HAL_OFFSET)

	/* Use secondary page table to switch to virtual memory. */
	//TODO: Remove this for something which starts from an already generated
	//temporary page table. (arm_setup_initial_pt_slave and arm_reset_initial_pt)
	bl arm_setup_initial_pt_slave
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

impl_core_2_reset:
    /* Start in supervisor mode, disable interrupts. */
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK

	/* Stack pointer starts at the physical address of the hyper stack top. */
	ldr sp, =(__hyper_stack_top_core_2__ + HAL_OFFSET)

	/* Use secondary page table to switch to virtual memory. */
	bl arm_setup_initial_pt_slave
	/* From here on (more precisely, from a point at the end of above function), 
	 * you are in virtual memory! */

	/* Switch to master page table */
	bl arm_reset_initial_pt

    /* Setup real stacks now, run core init and reclaim the initial stacks. */
	mov r0, #0x2
	bl arm_setup_initial_stack

    /* Init rest of hypervisor in C. start_ can be found in
	 * core/hypervisor/init.c */
    bl slave_start_
    
    /* Should not be reached! */
	bl _hang

impl_core_3_reset:
    /* Start in supervisor mode, disable interrupts. */
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK

	/* Stack pointer starts at the physical address of the hyper stack top. */
	ldr sp, =(__hyper_stack_top_core_3__ + HAL_OFFSET)

	/* Use secondary page table to switch to virtual memory. */
	bl arm_setup_initial_pt_slave
	/* From here on (more precisely, from a point at the end of above function), 
	 * you are in virtual memory! */

	/* Switch to master page table */
	bl arm_reset_initial_pt

    /* Setup real stacks now, run core init and reclaim the initial stacks. */
	mov r0, #0x3
	bl arm_setup_initial_stack

    /* Init rest of hypervisor in C. start_ can be found in
	 * core/hypervisor/init.c */
    bl slave_start_
    
    /* Should not be reached! */
	bl _hang
