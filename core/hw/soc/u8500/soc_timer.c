#include <hw.h>

#include "soc_defs.h"

#define TICK_COUNTER 0xFF

cpu_callback tick_handler = 0;


/* XXX: this handler is shared between MTU0:0 to MTU0:3,
 *      we should check for MTU0:0 before dispatching the timer callback
 */
return_value tick_handler_stub(uint32_t r0, uint32_t r1, uint32_t r2)
{
    BASE_REG timer = (BASE_REG) MTU0_BASE;    
 BASE_REG pt0   = timer + MTU_PT_BASE + MTU_PT_SIZE * 0;
    
    return_value ret = RV_OK;
    
    /* call tick handler */
    if(tick_handler)
        ret = tick_handler(r0, r1, r2);
    
    /* remove timer interrupt */
    timer[MTU_ICR] = (1UL << 0); /* clear interrupt 0 */
    
    /* reload the counter */
    pt0[MTU_PT_LR] = TICK_COUNTER; /* timer load value */          
        
    return ret;
}


void timer_tick_start(cpu_callback handler)
{    
    /* use timer MTU0_PT0 */
    BASE_REG timer = (BASE_REG) MTU0_BASE;
    BASE_REG pt0   = timer + MTU_PT_BASE + MTU_PT_SIZE * 0;
    
    tick_handler = handler;
    
    pt0[MTU_PT_LR] = TICK_COUNTER; /* timer load value */
    pt0[MTU_PT_CR] = 0x86; /* enable, 1/16, 32-bit */
    timer[MTU_IMSC] |= (1UL << 0); /* enable interrupt for 0 */    
}

void timer_tick_stop()
{
    BASE_REG timer = (BASE_REG) MTU0_BASE;
    BASE_REG pt0   = timer + MTU_PT_BASE + MTU_PT_SIZE * 0;    
    pt0[MTU_PT_CR] = 0; /* bit 7 is enable/disable */
}




void soc_timer_init()
{
    BASE_REG timer = (BASE_REG) MTU0_BASE;
    
    /* mask and clear all interrupts */
    timer[MTU_IMSC] = 0;
    timer[MTU_ICR] = -1;
    
    cpu_irq_set_enable(IRQ_MTU0, TRUE);
    cpu_irq_set_handler(IRQ_MTU0, tick_handler_stub);        
}

#ifdef DEBUG
void soc_timer_dump() /* DEBUG */
{
    int b;
    BASE_REG timer = (BASE_REG) MTU0_BASE;    
    printf("- TIMER  DEBUG -\n");
    
    printf("%x: ", timer);
    for(b = 0; b < 8; b++) printf("%x ", timer[b]);
    printf("\n");    
}
#endif
