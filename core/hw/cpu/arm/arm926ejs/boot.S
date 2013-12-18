
/*
 * boot code from ARMv5, including exception vector table
 *
 */

    .global impl_reset
    .global _interrupt_vector_table    
    .extern cpu_init
    .extern soc_init
    .extern board_init
    .extern core_init
    .extern start_
    .extern arm_move_guest_blob
    .extern arm_setup_initial_pt

    .extern impl_undef
    .extern impl_swi
    .extern impl_pabort
    .extern impl_dabort
    .extern impl_irq
    .extern impl_fiq

    .code 32
    .align 0
    .section .startup,"ax"
    

#include "arm_common.h"


 /*
  * system exception handlers.
  *
  * we use the slow absolute jump since this code will be relocated to 0x00000000
  */

_interrupt_vector_table:
    b impl_reset        @ relative jump, this will be called from padr
    ldr pc, adr_undef
    ldr pc, adr_swi
    ldr pc, adr_pabort
    ldr pc, adr_dabort
    b .
    ldr pc, adr_irq
    ldr pc, adr_fiq

adr_fiq: .word impl_fiq
adr_irq: .word impl_irq
adr_swi: .word impl_swi
adr_pabort: .word impl_pabort
adr_dabort: .word impl_dabort
adr_undef: .word impl_undef



/*
 * reset handler
 * This is the entry point of the SICS Thin Hypervisor  
 */   

impl_reset:
    /* Start in supervisor mode, disable interrupts and setup a temporary stack */
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK
    ldr sp, =(__hyper_stack_top__ + HAL_OFFSET)
    
        
    /* move the guest blob out of the way so we can get a clean BSS */
    bl arm_move_guest_blob
    bl arm_clear_bss
    
    /* switch to virtual memory */
    bl arm_setup_initial_pt
    
    /* setup real stacks now, run core init and reclaim the initial stacks */
    bl arm_setup_initial_stack
    bl core_init
    bl arm_reclaim_initial_pt 

    bl cpu_init
    bl soc_init
    bl board_init
    
    /* run system code */
    bl start_

    /* Should not be reached */
_hang:
	bl .
