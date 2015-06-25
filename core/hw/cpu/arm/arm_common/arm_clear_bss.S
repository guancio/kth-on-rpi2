/*
 * clean the BSS
 * 
 * NOTE: no BSS (duh!) or virtual memory is available when this code runs
 */

    .global arm_clear_bss
    .code 32
    .align  0

#include "arm_common.h"


arm_clear_bss:
    mov r0, #0
    ldr	r1,=(__hyper_bss_start__ + HAL_OFFSET)
    ldr	r2,=(__hyper_bss_end__ + HAL_OFFSET)
    
bss_loop:
    str r0, [r1], #4
    cmp r1, r2
    blo bss_loop
    
    bx lr
