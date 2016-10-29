#include "arm_common.h"
  .code 32
  .align  0

.global boot_slave1
boot_slave1:
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK
    ldr sp, =(__hyper_stack_top_core_1__ + HAL_OFFSET)
    bl arm_setup_initial_pt_slave
    
    ldr r0, = __hyper_stack_top_core_1__
    bl arm_setup_initial_stack
    
    bl start_slave

.global boot_slave2
boot_slave2:
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK
    ldr sp, =(__hyper_stack_top_core_2__ + HAL_OFFSET)
    bl arm_setup_initial_pt_slave
    
    ldr r0, = __hyper_stack_top_core_2__
    bl arm_setup_initial_stack
    
    bl start_slave

.global boot_slave3
boot_slave3:
    msr CPSR_c, #ARM_MODE_SUPERVISOR | ARM_INTERRUPT_MASK
    ldr sp, =(__hyper_stack_top_core_3__ + HAL_OFFSET)
    bl arm_setup_initial_pt_slave
    
    ldr r0, = __hyper_stack_top_core_3__
    bl arm_setup_initial_stack
    
    bl start_slave

