/*
 * ARM context top file
 * This file should be included on top of your exception handler.
 * It contains the low level context handling definitions and macros.
 */

#define INDEX_R0 0    
#define INDEX_SP 13
#define INDEX_LR 14
#define INDEX_PC 15
#define INDEX_PSR 16

.macro GET_PID
    mrc p15, 0, r0, c0, c0, 5
    and r0, r0, #0x3   
.endm
#define CONTEXT_STACK_LEN 128

    /*
     * Saves context, modifies R12 and LR.
     */
.macro store_context
     
     push { r0-r3 }     
     GET_PID
    /* move to r1 */
    mov r1, r0 
    /* 92: The size of one stack is 23 words, one word is 4 bytes. */
    mov r2, #CONTEXT_STACK_LEN
    mul r1, r1, r2
    /* Add resulting offset to context_stack_curr (first stack) */
    ldr r0, =context_stack_curr
    add r0, r1, r0
    /* Store in r0 the value at r1 */
    ldr r2, [r0]
    add r2, r2, #1

    /* r0 will now hold the address of the current stack */
    add r1, r0, #4
    ldr r1, [r1, r2, lsl #2] 
    str r2, [r0]
 
    //  ldr r0, =context_stack_curr
    //   ldr r1, =context_stack
    //  ldr r2, [r0]
    // add r2, r2, #1     
    // ldr r1, [r1, r2, lsl #2]
    // str r2, [r0]

     str lr, [r1, #4 * INDEX_PC]
     mov lr, r1
     pop { r0-r3 }
     
     stmia lr, {r0-lr}^
     mrs r12, spsr
     str r12, [lr, #4 * INDEX_PSR]    
.endm
     
     


    /*
     * Saves context, modifies R12 but retains LR.
     */
.macro store_context_keep_lr
     
     push { r0-r3 }    
    GET_PID
    /* move to r1 */
    mov r1, r0 
    /* 92: The size of one stack is 23 words, one word is 4 bytes. */
    mov r2, #CONTEXT_STACK_LEN
    mul r1, r1, r2
    /* Add resulting offset to context_stack_curr (first stack) */
    ldr r0, =context_stack_curr
    add r0, r1, r0
    /* Store in r0 the value at r1 */
    ldr r2, [r0]
    add r2, r2, #1

    /* r0 will now hold the address of the current stack */
    add r1, r0, #4
    ldr r1, [r1, r2, lsl #2] 
    str r2, [r0]
   //  ldr r0, =context_stack_curr// get the pointer to context_stack_curr eq line 53 
   //  ldr r1, =context_stack// get the pointer to context_stack eq line 59
    // ldr r2, [r0] //=// loads// the value of context_stack_curr eq line 55
    // add r2, r2, #1//= increase the value of context_stack_curr eq line 56
    // ldr r1, [r1, r2, lsl #2]// load pointer to ctx eq to line 60 
     //str r2, [r0]
     
    
     str lr, [r1, #4 * INDEX_PC]
     mov lr, r1
     pop { r0-r3 }
    
     stmia lr, {r0-lr}^
     mrs r12, spsr
     str r12, [lr, #4 * INDEX_PSR]    
     ldr lr, [lr, #4 * INDEX_PC]
.endm
     
     
.macro restore_context

    GET_PID
    /* move to r1 */
    mov r1, r0 
    /* 92: The size of one stack is 23 words, one word is 4 bytes. */
    mov r2, #CONTEXT_STACK_LEN
    mul r1, r1, r2
    /* Add resulting offset to context_stack_curr (first stack) */
    ldr r0, =context_stack_curr
    add r0, r1, r0
    /* Store in r0 the value at r1 */
    ldr r2, [r0]

    /* r0 will now hold the address of the current stack */
    add r1, r0, #4
    ldr lr, [r1, r2, lsl #2] 
    sub r2, r2, #1
    str r2, [r0]

    // ldr r0, =context_stack_curr
    // ldr r1, =context_stack
    // ldr r2, [r0]
    // ldr lr, [r1, r2, lsl #2]
    // sub r2, r2, #1     
    // str r2, [r0]
     
     ldr r0, [lr, #4 * INDEX_PSR]
     msr spsr, r0
     ldmia lr, {r0-lr} ^
     ldr lr, [lr, #4 * INDEX_PC]    
.endm

