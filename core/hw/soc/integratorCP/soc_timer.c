#include "hw.h"
#include "soc_defs.h"

cpu_callback tick_handler = 0;
void tick_handler_stub(uint32_t r0, uint32_t r1, uint32_t r2)
{
	BASE_REG timer = (BASE_REG) TIMER1_VA_BASE;

    /* call tick handler */
    if(tick_handler)
        tick_handler(r0, r1, r2);
    
    /* ack the timer interrupt */
    timer[TIMER_INTCLR] = 1;

}

/*
 * use timer 1 for system tick.
 * the reason we use this one is because the other
 * aren't working in OVP...
 */
void timer_tick_start(cpu_callback handler)
{

	BASE_REG timer = (BASE_REG) TIMER1_VA_BASE;

    /* disable timer clock and interrupts for now */
    cpu_irq_set_enable(INTSRC_IRQ_TIMERINT1, FALSE);
    
    /* setup timer */
    timer[TIMER_LOAD] = 10000;
    timer[TIMER_VALUE] =10000;

    
    /* set handler */
    tick_handler = handler;
    cpu_irq_set_handler(INTSRC_IRQ_TIMERINT1, (cpu_callback)tick_handler_stub);
    cpu_irq_set_enable(INTSRC_IRQ_TIMERINT1, TRUE);

    
    /* enable timer */    
    timer[TIMER_CONTROL] = (TIMER_CTRL_ENABLE | TIMER_CTRL_PERIODIC | TIMER_CTRL_IE);

}

void timer_tick_stop()
{
	BASE_REG timer = (BASE_REG) TIMER1_VA_BASE;
	timer[TIMER_CONTROL] &= ~TIMER_CTRL_ENABLE;
}

void soc_timer_init()
{

	//Disable all timers
	BASE_REG timer = (BASE_REG) TIMER0_VA_BASE;
	timer[TIMER_CONTROL] = 0;

    timer = (BASE_REG) TIMER1_VA_BASE;
    timer[TIMER_CONTROL] = 0;
    
    timer = (BASE_REG) TIMER2_VA_BASE;
    timer[TIMER_CONTROL] = 0;
    printf("SOC Timer initialized\n");


}
