#include "hw.h"
#include "soc_timer.h"
#include "soc_interrupt.h"

//The timer on the BCM2836 chip is based on, but not identical to, the ARM SP804 timer. It is described on page 196 onwards of the BCM2835 documentation. 

static timer_registers *timer = 0;
static cpu_callback tick_handler = 0;

//Acks the IRQ and calls Tick handler
void tick_handler_stub(uint32_t r0, uint32_t r1, uint32_t r2){
    //Writing in the Timer IRQ Clear register acks the IRQ.
    timer->tic = 0;
    //Call the Tick handler
    if(tick_handler){
        tick_handler(r0, r1, r2);
    }
}

//Start the ARM timer.
void timer_tick_start(cpu_callback handler){
    uint32_t register_a;
    
	//Disable timer and timer interrupt.
	register_a = timer->tcr;
	register_a &= ~((1<<5)|(1<<7)); //Could write decimal 5 shifted 5 instead
    timer->tcr = register_a;
    
	//To remove pending interrupts, just write anything to Timer IRQ clear register
	timer->tic = 0;  

	//Probably, the RPi2 code does not need this because of our careful handling when disabling.
    ////setup timer
    ////timer->channels[1].cmr = 0x4004;
    
    //TODO: Set handler
    tick_handler = handler;
    cpu_irq_set_handler(AIC_IRQ_NUM_TIMER, (cpu_callback)tick_handler_stub);
    soc_interrupt_set_configuration(AIC_IRQ_NUM_TIMER,
                                    6, TRUE, TRUE);
    
    ////enable timer
    ////timer->channels[1].ier = (1UL << 4);
	//Enable timer.
	timer->tcr = (1 << 5)|(1 << 7);
	//TODO: Check line below
    cpu_irq_set_enable(AIC_IRQ_NUM_TIMER, TRUE);        
    
	//TODO: What do the below lines do and are they essential?
    ////timer->channels[1].ccr = 0x1;
    ////the simplified OVP implementation nneds this to start
    ////timer->channels[1].rc = 312; /* ~5 ms */
}

void timer_tick_stop(){
    //Disable timer clock and interrupts.
	register_a = timer->tcr;
	register_a &= ~((1<<5)|(1<<7)); //Could write decimal 5 shifted 5 instead
    timer->tcr = register_a; 
}

void soc_timer_init(){
	/*Needs to be rewritten*/
#if 0
	//*ms is probably not hard-coded...
    memspace_t *ms = env_map_from(PROC_TYPE_HYPERVISOR, PROC_TYPE_HYPERVISOR,
                           "__soc_timer", TIMER_BASE, sizeof(timer_registers) , TRUE);
    
    timer = ms->vadr;
#endif

    //Disable timer clock and interrupts.
    register_a = timer->tcr;
	register_a &= ~((1<<5)|(1<<7)); //Could write decimal 5 shifted 5 instead
    timer->tcr = register_a;

}
