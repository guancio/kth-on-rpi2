/*
 * ARM context bottom file
 * This file should be included at bottom of your exception handler.
 * It contains the context structures and support functions.
 */


/*
 * CPU context functions 
 */
     .code 32
     .global cpu_context_current_get
     .global cpu_context_current_set
     .global cpu_context_initial_set
     .global cpu_context_depth_get
     
cpu_context_current_get:
    ldr r0, =context_stack_curr
    add r1, r0, #4 @  ldr r1, =context_stack
    ldr r0, [r0]
    ldr r0, [r1, r0, lsl #2]
    bx lr

cpu_context_current_set:
    ldr r1, =context_stack_curr
    add r2, r1, #4 @ ldr r2, =context_stack
    ldr r1, [r1]
    str r0, [r2, r1, lsl #2]
    bx lr    
    
cpu_context_initial_set:
    ldr r2, =context_stack
    str r0, [r2, #4]
    bx lr

cpu_context_depth_get:
    ldr r0, =context_stack_curr
    ldr r0, [r0]
    bx lr
    
    
    

/*
 * exception handler data
 *
 * NOTE: it is important that context_stack == context_stack_curr + 4
 */
 
    .data
context_stack_curr:
    .word 0
context_stack:
    .word 0
    .word def_context1
    .word def_context2
    .word def_context3
    .word def_context4
    .word def_context5
    .word def_context6
    .word def_context7
    .word 0
    
    .data
def_context1: .space 4 * 20, 0
def_context2: .space 4 * 20, 0
def_context3: .space 4 * 20, 0
def_context4: .space 4 * 20, 0
def_context5: .space 4 * 20, 0
def_context6: .space 4 * 20, 0
def_context7: .space 4 * 20, 0


