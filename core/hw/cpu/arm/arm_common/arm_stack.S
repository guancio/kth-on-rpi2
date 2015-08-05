/*
 * Setup the stack for the core in question.
 * This is called during initialization, from boot.S (and boot_slave.S).
 */

    .global arm_setup_initial_stack
    .code 32
    .align  0

#include "arm_common.h"


arm_setup_initial_stack:
	/* TODO: Before jumping to the above label, r0 has been set to the index of
	 * the processor core we are initializing a stack for (typically 0, 1, 2 or
	 * 3). */
	cmp r0, #0x0
	//bne case_core_one //TODO: Commented out before building final multicore version
	ldr r0, = __hyper_stack_top__
	b setup_stack
/* //TODO: Commented out before building final multicore version...
case_core_one:
	cmp r0, #0x1
	bne case_core_two
	ldr r0, = __hyper_stack_top_core_1__
	b setup_stack
case_core_two:
	cmp r0, #0x2
	bne case_core_three
	ldr r0, = __hyper_stack_top_core_2__
	b setup_stack
case_core_three:
	cmp r0, #0x3
	bne invalid_core
	ldr r0, = __hyper_stack_top_core_3__
	b setup_stack
invalid_core:
	*/
	/* TODO: Here, we can put a call to a general error handler to display a
	 * message about initialization from an invalid core number. */

setup_stack:
    msr CPSR_c, #ARM_MODE_IRQ | ARM_INTERRUPT_MASK
    mov sp, r0
    sub r0, r0, #128

    msr CPSR_c, #ARM_MODE_FIQ | ARM_INTERRUPT_MASK
    mov sp, r0
    sub r0, r0, #128

    msr CPSR_c, #ARM_MODE_ABORT | ARM_INTERRUPT_MASK
    mov sp, r0
    sub r0, r0, #128

    msr CPSR_c, #ARM_MODE_UNDEFINED | ARM_INTERRUPT_MASK
    mov sp, r0
    sub r0, r0, #128

    msr CPSR_c, #ARM_MODE_SYSTEM | ARM_INTERRUPT_MASK
    mov sp, r0
    sub r0, r0, #128

    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK
    mov sp, r0
    sub r0, r0, #0x1000
    
    bx lr

