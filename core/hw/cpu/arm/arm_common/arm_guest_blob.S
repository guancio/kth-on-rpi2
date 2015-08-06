/*
 * Copies guest blob from start of BSS to its intermediate location.
 *
 * NOTE: This code runs at the very start: no stack is available 
 *       and BSS has not been initialized yet.
 */

    .global arm_move_guest_blob
    .extern guests_db
    .code 32
    .align  0

#include "guest_blob.h"
#include "arm_common.h"

arm_move_guest_blob:

    /* Get guest header and see if it is valid. */
    ldr r0, =__data_end__ + HAL_OFFSET
    ldr r2, [r0], #4
    ldr r3, =GUESTS_MAGIC
    cmp r3, r2
    bne fail

    /* Get guest count and verify. */
    ldr r2, [r0], #4
    cmp r2, #0
    ble fail

    /* r0=binary end, r1=binary start, r2=binary count, r3=pmem end */        
    add r1, r0, r2, LSL # 4
    ldr r3, [r0]
    add r0, r3
    ldr r3, =(HAL_PHYS_START+HAL_PHYS_SIZE)  

    /* Save guest count and pstart. */     
    ldr r10, =guests_db + HAL_OFFSET
    str r2, [r10], #8
    str r3, [r10], #4       @@ pstart
     
foreach_guest:
    /* Get size & padr. make sure size is 4-aligned. */
    ldr r4, [r1], #-4      @@ VADR
    ldr r5, [r1], #-4      @@ PSIZE
    ldr r6, [r1], #-4      @@ FWSIZE
    ldr r7, [r1], #-4	   @@ OFFSET

    /* Calc pstart, align to MB. */
    sub r3, r3, r5    
    mov r3, r3, LSR #20
    mov r3, r3, LSL #20
    
    /* Update guest db. */
    stmia r10!, {r3-r7}

    /* Copy guest fw to start of the physical memory of this guest. */
    sub r0, r6  @@ start of this fw
    mov r4, r0  @@ copy it!
    add r5, r3, r7  @@ pmem start + offset
    add r6, r6, r5  @@ pmem end
    add r6, r7		@@ pmem + offset
        
copy_one:
    ldmia r4!, {r8, r9}
    stmia r5!, {r8, r9}    
    cmp r5, r6
    ble copy_one
    
    /* More guests? */
    subs r2, r2, #1
    bne foreach_guest

    /* Save pstart. */
    ldr r7, =guests_db + HAL_OFFSET        
    str r3, [r7, #4]       @@ pstart

done:    
    mov pc, lr
    
fail:
    b .    
