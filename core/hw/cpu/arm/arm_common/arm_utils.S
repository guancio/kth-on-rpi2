
    .global cpu_interrupt_set
    .global cpu_interrupt_user_set
    .global cpu_read_cpsr
    .global hwreg_read
    .global hwreg_write
    .global hwreg_update
    .global hwreg_wait

    .code 32
    .align  0

#include "arm_common.h"

cpu_interrupt_set:
    and r2, r0, #ARM_IRQ_MASK | ARM_FIQ_MASK /* keep only the interrupt bits! */
    mrs r0, CPSR    
    bic r1, r0, #ARM_IRQ_MASK | ARM_FIQ_MASK
    orr r1, r1, r2
    msr CPSR_c, r1
    bx lr
    

/* XXX: re-write this to be more like the one above */
/* XXX: why do we have this function anyway? why not use the context ?? */
/* XXX: prepare to remove if not used in future */
cpu_interrupt_user_set:
    mrs r1, SPSR
    and r2, r1, #ARM_IRQ_MASK | ARM_FIQ_MASK
    bic r1, #ARM_IRQ_MASK | ARM_FIQ_MASK
    orr r1, r0
#    mov r0, r2
	mov r0, r1
    msr SPSR_c, r1
    bx lr

cpu_read_cpsr:
	mrs r0, CPSR
	bx lr

    
hwreg_read:
    ldr r0, [r0]
    bx lr
    
hwreg_write:
    str r1, [r0]
    bx lr
    
// uint32_t hwreg_update(adr, mask, value)
hwreg_update:
    ldr r3, [r0]
    bic r3, r3, r1
    and r2, r2, r1 /* anti stupidity operation */
    orr r3, r3, r2
    str r3, [r0]
    mov r0, r3
    bx lr    
    
// void hwreg_wait(adr, mask, value)    
hwreg_wait:
    and r2, r2, r1 /* anti stupidity operation */
hwreg_wait_2:
    ldr r3, [r0]
    and r3, r3, r1    
    cmp r3, r2
    bxeq lr
    b hwreg_wait_2

