/*
 * setup the system stack
 */

    .global arm_setup_initial_stack
    .code 32
    .align  0

#include "arm_common.h"


arm_setup_initial_stack:

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

