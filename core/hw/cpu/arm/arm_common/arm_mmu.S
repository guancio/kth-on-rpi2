
/*
 * common ARM MMU functions
 */

    .global mem_mmu_set_domain
    .global mem_mmu_set_translation_table
    .global mem_mmu_set_enable

    .global mem_mmu_pid_set
    .global mem_mmu_pid_get

    .code 32
    .align  0

#include "arm_common.h"

/*
 * MMU control
 */


mem_mmu_set_domain:
    mcr p15, 0, r0, c3, c0, 0
    bx lr

mem_mmu_set_translation_table:
    mcr p15, 0, r0, c2, c0, 0
    bx lr


mem_mmu_set_enable:
    mrc p15, 0, r1, c1, c0, 0
    cmp r0, #0
    orrne r1, r1, #1
    biceq r1, r1, #1
    mcr p15, 0, r1, c1, c0, 0
    bx lr


/*
 * FCSE
 */
mem_mmu_pid_set:
    lsl r0, r0, #25
    mcr p15, 0, r0, c13, c0, 0
    bx lr
    
mem_mmu_pid_get:
    mrc p15, 0, r0, c13, c0, 0
    lsr r0, r0, #25
    bx lr
    
