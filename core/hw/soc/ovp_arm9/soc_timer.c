#include "hw.h"
#include "soc_timer.h"
#include "soc_interrupt.h"

static timer_registers *timer = 0;
static cpu_callback tick_handler = 0;

void tick_handler_stub(uint32_t r0, uint32_t r1, uint32_t r2)
{
    
    /* ack the timer interrupt */    
//    uint32_t dummy = timer->channels[1].sr; /* this ack the irq */
    timer->channels[1].sr; /* this ack the irq */
    /* call tick handler */
    if(tick_handler)
        tick_handler(r0, r1, r2);
    
}

/*
 * use timer 1 for system tick.
 * the reason we use this one is because the other
 * aren't working in OVP...
 */
void timer_tick_start(cpu_callback handler)
{
    uint32_t tmp;
    
    /* disable timer clock and interrupts for now */
    timer->channels[1].ccr = 0x2;         
    timer->channels[1].idr = -1;
    
    /* remove pending interrupts */
    tmp = timer->channels[1].sr;
    
    /* setup timer */
    timer->channels[1].cmr = 0x4004;
    
    /* set handler */
    tick_handler = handler;
    cpu_irq_set_handler(AIC_IRQ_NUM_TIMER1, (cpu_callback)tick_handler_stub);
    soc_interrupt_set_configuration(AIC_IRQ_NUM_TIMER1,
                                    6, TRUE, TRUE);
    
    /* enable timer */    
    timer->channels[1].ier = (1UL << 4);
    cpu_irq_set_enable(AIC_IRQ_NUM_TIMER1, TRUE);        
    
    timer->channels[1].ccr = 0x1;    
    
    /* the simplified OVP implementation nneds this to start */
    timer->channels[1].rc = 312; /* ~5 ms */
}

void timer_tick_stop()
{
    timer->channels[1].ccr = 0x2;         
    timer->channels[1].idr = -1;    
}

void soc_timer_init()
{
    
    memspace_t *ms = env_map_from(PROC_TYPE_HYPERVISOR, PROC_TYPE_HYPERVISOR,
                           "__soc_timer", TIMER_BASE, sizeof(timer_registers) , TRUE);
    
    timer = ms->vadr;
    
    /* disable timer clock and interrupts */
    timer->channels[0].ccr = 0x3;         
    timer->channels[1].ccr = 0x3;         
    timer->channels[2].ccr = 0x3;         
    timer->channels[0].idr = -1;
    timer->channels[1].idr = -1;
    timer->channels[2].idr = -1;   
}
