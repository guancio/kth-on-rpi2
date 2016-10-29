/*
 * ARM context bottom file
 * This file should be included at bottom of your exception handler.
 * It contains the context structures and support functions.
 */


/*
 * CPU context functions
 */
     .code 32
     .global get_current_vm
     .global cpu_context_current_get
     .global cpu_context_current_set
     .global cpu_context_initial_set
     .global cpu_context_depth_get  
     
     .global context_stack_1
     .global context_stack_curr_1
    .global context_stack
     .global context_stack_curr

#define CONTEXT_STACK_LEN 128


get_current_vm:
    /* Uses r0 and r1 */
    bl get_pid

    /* Load the address of the virtual machines in r1. */
    ldr r1, =curr_vms

    /* Load into r0 the value at address r1+r0*4 - should be a pointer to the
	current virtual machine. */
    ldr r0, [r1, r0, lsl #2]

    /* Return - at this point a pointer to the current virtual machine will be
	stored in r0. */
    bx lr


cpu_context_current_get:

    /* Fetches CPU core index into register 0. */
    
    GET_PID
    /* move to r1 */
    mov r1, r0 
    /* 92: The size of one stack is 23 words, one word is 4 bytes. */
    mov r2, #CONTEXT_STACK_LEN
    mul r1, r1, r2
    /* Add resulting offset to context_stack_curr (first stack) */
    ldr r2, =context_stack_curr
    add r1, r1, r2
    /* Store in r0 the value at r1 */
     ldr r0, [r1]
    /* r0 will now hold the address of the current stack */
    add r1, r1, #4
    ldr r0, [r1, r0, lsl #2] 
    bx lr

	/* Old version:
    ldr r0, =context_stack_curr
    add r1, r0, #4 @  ldr r1, =context_stack
    ldr r0, [r0]
    ldr r0, [r1, r0, lsl #2]
    bx lr
	*/

cpu_context_current_set:
    /* r1, r2 clear to use - r0 used */
    mov r3, r0
    GET_PID
    /* move to r1 */
    mov r1, r0 
    /* 92: The size of one stack is 23 words, one word is 4 bytes. */
    mov r2, #CONTEXT_STACK_LEN
    mul r1, r1, r2
    /* Add resulting offset to context_stack_curr (first stack) */
    ldr r2, =context_stack_curr
    add r1, r1, r2
    /* Store in r0 the value at r1 */
     ldr r0, [r1]
    /* r0 will now hold the address of the current stack */
    add r1, r1, #4
    
    str r3, [r1, r0, lsl #2]
    bx lr

	/* OLD:
    ldr r1, =context_stack_curr
    add r2, r1, #4 @ ldr r2, =context_stack
    ldr r1, [r1]
    str r0, [r2, r1, lsl #2]
    bx lr  
	*/

cpu_context_initial_set:
    /* r2 clear to use - r0, r1 used */
    /* 92: The size of one stack is 23 words, one word is 4 bytes. */
    mov r2, #CONTEXT_STACK_LEN
    mul r1, r1, r2
    /* Add resulting offset to context_stack_curr (first stack) */ 
    ldr r2, =context_stack_curr
    add r1, r1, r2
    add r1, r1, #4
    /* Store in r2 the value at r1 */

    str r0, [r1, #4]
    bx lr

    	/* OLD:
    ldr r2, =context_stack
    str r0, [r2, #4]
    bx lr
	*/

cpu_context_depth_get:
	/* r0 clear to use */
	bl get_pid
        mov r1, r0
	/* 92: The size of one stack is 23 words, one word is 4 bytes. */
        mov r2, #CONTEXT_STACK_LEN
	mul r1, r1, r2
	/* Add resulting offset to context_stack_curr (first stack) */
        ldr r2, =context_stack_curr
	add r1, r1, r2

	add r1, r1, #4
	/* Store in r1 the value at r1 */
	ldr r0, [r1]

	ldr r0, [r0]
    bx lr
	
	/* OLD:
    ldr r0, =context_stack_curr
    ldr r0, [r0]
    bx lr
	*/
    
    

/*
 * Exception handler data
 *
 * NOTE: it is important that context_stack == context_stack_curr + 4
 */

/* Core 0 */
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
    .space CONTEXT_STACK_LEN-40, 0
/* Core 1 */
    .data
context_stack_curr_1:
    .word 0
context_stack_1:
    .word 0
    .word def_context_1_1
    .word def_context_1_2
    .word def_context_1_3
    .word def_context_1_4
    .word def_context_1_5
    .word def_context_1_6
    .word def_context_1_7
    .word 0

    .space CONTEXT_STACK_LEN-40, 0
/* Core 2 */
    .data
context_stack_curr_2:
    .word 0
context_stack_2:
    .word 0
    .word def_context_2_1
    .word def_context_2_2
    .word def_context_2_3
    .word def_context_2_4
    .word def_context_2_5
    .word def_context_2_6
    .word def_context_2_7
    .word 0

    .space CONTEXT_STACK_LEN-40, 0
/* Core 3 */
    .data
context_stack_curr_3:
    .word 0
context_stack_3:
    .word 0
    .word def_context_3_1
    .word def_context_3_2
    .word def_context_3_3
    .word def_context_3_4
    .word def_context_3_5
    .word def_context_3_6
    .word def_context_3_7
    .word 0

    .space CONTEXT_STACK_LEN-40, 0
/* The context space are defined here. TODO: Is it really necessary to replicate
 * everything? */

    .data
def_context1: .space 4 * 20, 0
def_context2: .space 4 * 20, 0
def_context3: .space 4 * 20, 0
def_context4: .space 4 * 20, 0
def_context5: .space 4 * 20, 0
def_context6: .space 4 * 20, 0
def_context7: .space 4 * 20, 0

    .data
def_context_1_1: .space 4 * 20, 0
def_context_1_2: .space 4 * 20, 0
def_context_1_3: .space 4 * 20, 0
def_context_1_4: .space 4 * 20, 0
def_context_1_5: .space 4 * 20, 0
def_context_1_6: .space 4 * 20, 0
def_context_1_7: .space 4 * 20, 0

    .data
def_context_2_1: .space 4 * 20, 0
def_context_2_2: .space 4 * 20, 0
def_context_2_3: .space 4 * 20, 0
def_context_2_4: .space 4 * 20, 0
def_context_2_5: .space 4 * 20, 0
def_context_2_6: .space 4 * 20, 0
def_context_2_7: .space 4 * 20, 0

    .data
def_context_3_1: .space 4 * 20, 0
def_context_3_2: .space 4 * 20, 0
def_context_3_3: .space 4 * 20, 0
def_context_3_4: .space 4 * 20, 0
def_context_3_5: .space 4 * 20, 0
def_context_3_6: .space 4 * 20, 0
def_context_3_7: .space 4 * 20, 0
