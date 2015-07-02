//More of the same interrupt stuff...

#define AIC_BASE 0xFFFFF000
#define AIC_IVR (0x100 / 4)
#define AIC_IPR (0x10c / 4)

#define AIC_FVR (0x104 / 4)

#define AIC_IECR (0x120 / 4)
#define AIC_IDCR (0x124 / 4)
#define AIC_ICCR (0x128 / 4)
#define AIC_ISCR (0x12c / 4)

#define AIC_EOICR (0x130 / 4)
#define AIC_ISR (0x108 / 4)

#define AIC_ISR_IRQID_MASK 31


#include "family_context_top.inc"

    .global impl_irq
    .global impl_fiq
    .global impl_swi
    .global impl_undef
    .global impl_dabort
    .global impl_pabort
    
    .code 32
    .align 4

impl_irq:
    /* 1. save context */
    sub lr, lr, #4
    store_context
    
    /* 2. call handler */
    ldr r0, =AIC_BASE            
    ldr r1, [r0, #4 * AIC_IVR]    
    mov lr, pc
    bx r1
    
    /* 3. clear interrupt */
    ldr r0, =AIC_BASE        
    ldr r1, [r0, #4 * AIC_ISR]   
    str r1, [r0, #4 * AIC_EOICR]

    @ start of OVP hack to clear AIC
    mov r2, #1
    lsl r1, r2, r1
    str r1, [r0, #4 * AIC_IDCR]
    str r1, [r0, #4 * AIC_IECR]        
    @ end of OVP hack
    
    /* 4. restore context 
     * note: this code is shared with some other
     * exception handlers */    
exception_bottom:
    restore_context    
    movs pc, lr
    
impl_fiq:
/* TODO */
    b .

impl_pabort:
    sub lr, lr, #4
    store_context_keep_lr
    
    ldr r3, =family_callback_inst_abort        
    ldr r3, [r3]
    
    mrc p15, 0, r0, c6, c0, 0
    mrc p15, 0, r1, c5, c0, 1    
    mov r2, lr
    
    ldr lr, =exception_bottom
    bx r3
    /* returns by exception bottom */

impl_dabort:
    sub lr, lr, #8
    store_context_keep_lr

    ldr r3, =family_callback_data_abort
    ldr r3, [r3]
    
    mrc p15, 0, r0, c6, c0, 0
    mrc p15, 0, r1, c5, c0, 0
    mov r2, lr
    
    ldr lr, =exception_bottom
    bx r3
    /* returns by exception bottom */

    
impl_undef:
    store_context_keep_lr

    ldr r3, =family_callback_undef
    ldr r3, [r3]
    
    sub r2, lr, #4
    ldr r0, [r2]

    ldr lr, =exception_bottom
    bx r3
    /* returns by exception bottom */
    
impl_swi:    
    store_context_keep_lr

    /* get SWI data */
    ldr r12, =family_callback_swi
    ldr r12, [r12]    

    /* XXX: this only works from 32-bit mode */
    sub r11, lr, #4    
    ldr r2, [r11]
    bic r2, r2, #0xFF000000

    ldr lr, =exception_bottom
    bx r12
    /* returns by exception bottom */

/*
 * Exception handler literals
 *
 */
    .ltorg
    
#include "family_context_bottom.inc"
