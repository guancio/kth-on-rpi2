/*
 * copy guest blob from start of bss to its intermmediate location
 *
 * NOTE: this code runs at the very start: no stack is available 
 *       and BSS has not been initialized yet
 */

    .global arm_move_guest_blob
    .extern guests_db
    .code 32
    .align  0

#include "guest_blob.h"
#include "arm_common.h"


arm_move_guest_blob:

    /* get guest header and see if it is valid */
    ldr r0, =__data_end__ + HAL_OFFSET   
    ldr r2, [r0], #4
    ldr r3, =GUESTS_MAGIC 
    cmp r3, r2
    bne fail
    ldr r2, [r0], #4
    cmp r2, #8      @@ minimum guest size (size + padr and no data)
    blt fail
       
    add r1, r0, r2  @@ end of all (inclusive)
    
    /*
     * r0 = pointer to guest data
     * r1 = end of all
     */
    ldr r2, =guests_db + HAL_OFFSET
    mov r6, #0
    str r6, [r2]    @@ guest_db.count = 0
     
foreach_guest:
    /* get size & padr. make sure size is 4 alligned */
    ldr r3, [r0], #4    @@ SIZE
    ldr r4, [r0], #4    @@ PADR
    ldr r5, =HAL_PHYS_START
    add r4, r4, r5
    add r3, r3, #3
    bic r3, r3, #3
                
    /* update guest_db */
    ldr r6, [r2]
    
    mov r7, #12
    mul r7, r6, r7
    add r7, r7, r2
    str r4, [r7, #4]!        @@ ptr_phy
    str r0, [r7, #4]!        @@ ptr_phy_tmp
    str r3, [r7, #4]         @@ size
    
    add r6, #1
    cmp r6, #MAX_GUESTS    
    bge fail  @ no more room :(    
    str r6, [r2]            @@ cnt++
        
    mov r5, r0 @@ start of this guest
    add r0, r3  
    add r4, r3 @@ PADR_end
    mov r6, r0 @@ end of this guest
copy_one:
    ldr r7, [r6, #-4]!
    str r7, [r4, #-4]!
    cmp r6, r5
    bgt copy_one
    
    cmp r0, r1
    blt foreach_guest

done:    
    mov pc, lr
fail:
    b .    
