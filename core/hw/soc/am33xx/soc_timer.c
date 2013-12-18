#include <hw.h>

#include "soc_defs.h"


#define GTIMER_COUNT 7

#define GTIMER_1MS_FIRST 0
#define GTIMER_1MS_LAST 0

#define GTIMER_TCLR_START_STOP_CTRL (1 << 0)
#define GTIMER_TCLR_AUTO_RELOAD (1 << 1)


#define GTIMER_TISR_MAT_IT_FLAG_CLEAR (1 << 0)
#define GTIMER_TISR_OVT_IT_FLAG_CLEAR (1 << 1)
#define GTIMER_TISR_TCAR_IT_FLAG_CLEAR (1 << 2)

#define GTIMER_TIER_MAT_IT_EN  (1 << 0)
#define GTIMER_TIER_OVF_IT_EN  (1 << 1)
#define GTIMER_TIER_TCAR_IT_EN (1 << 2)

#define TICK_COUNTER 0xFF

typedef struct {
    uint32_t tidr;
    uint32_t unused1[3];
    uint32_t tiocp_cfg;
    uint32_t unused2[3];
    
    uint32_t irq_eoi;
    uint32_t irqstatus_raw;
    uint32_t irqstatus;
    uint32_t irqenable_set;
    uint32_t irqenable_clr;
    uint32_t irqwakeen;
    
    uint32_t tclr;
    uint32_t tcrr;
    uint32_t tldr;
    uint32_t ttgr;
    uint32_t twps;
    uint32_t tmar;
    uint32_t tcar1;
    uint32_t tsicr;
    uint32_t tcar2;
    
} volatile gtimer;


typedef struct {
    uint32_t tidr;
    uint32_t unused1[3];
    uint32_t tiocp_cfg;
    uint32_t tistat;
    uint32_t tisr;
    uint32_t tier;
    uint32_t twer;
    uint32_t tclr;
    uint32_t tcrr;
    uint32_t tldr;
    uint32_t ttgr;
    uint32_t twps;
    uint32_t tmar;
    uint32_t tcar1;
    uint32_t tsicr;
    uint32_t tcar2;
    
    uint32_t tpir;     
    uint32_t tnir;
    uint32_t tcvr;
    uint32_t tocr;
    uint32_t towr;   
} volatile gtimer_1ms;


static uint32_t TIMER_BASES_1MS [] = {
    0x44E31000,    
};

static uint32_t TIMER_BASES [] = {
    0x44E05000,
//    0x44E31000,    // 1ms timer
    0x48040000,
    0x48042000, /* TODO: WHY IS TIMER3 TURNED OFF? */
    0x48044000,    
    0x48046000,
    0x48048000, /* TODO: WHY IS TIMER6 TURNED OFF? */
    0x4804A000    
};


/* --------------------------------------------------------- */
cpu_callback tick_handler = 0;

return_value timer_tick_handler_stub(uint32_t r0, uint32_t r1, uint32_t r2);

// ------------------------------------------------------
// timer support functions
// ------------------------------------------------------

gtimer *timer_get(int n)
{
    if(n < 0  || n >= GTIMER_COUNT) return 0;   
    return (gtimer *) TIMER_BASES[n];
}

static gtimer_1ms *timer_tick_get()
{
    return (gtimer_1ms *) TIMER_BASES_1MS[0];
}


void timer_tick_clear_interrupt()
{
    gtimer_1ms *timer = timer_tick_get();
    volatile uint32_t what;
        
    what = timer->tisr;
    timer->tisr = what;    
}


void timer_tick_start(cpu_callback handler)
{    
    gtimer_1ms *timer = timer_tick_get();
    
    cpu_irq_set_enable(INTC_IRQ_TIMER1_MS, FALSE);
    
    timer->tpir = 232000;
    timer->tnir = -768000;
    timer->tldr = timer->tcrr = 0xFFFFFFE0;
    
    // XXX: for some reason this gives us 1ms instead of the value above 
    // timer->tldr = timer->tcrr = -(32 * 256 * 5  / 17);    
    timer->tldr = timer->tcrr = -1024 * 64;
    // timer->tldr = timer->tcrr = 0xFFFFE6FF;    
    // timer->tldr = timer->tcrr = -1024 * 4;

    // 
    tick_handler = handler;
    cpu_irq_set_handler(INTC_IRQ_TIMER1_MS, timer_tick_handler_stub);
    cpu_irq_set_enable(INTC_IRQ_TIMER1_MS, TRUE);
    
    /* clear old status bits, set interrupt type and start it */
    timer->tisr |= GTIMER_TISR_MAT_IT_FLAG_CLEAR | GTIMER_TISR_OVT_IT_FLAG_CLEAR | GTIMER_TISR_TCAR_IT_FLAG_CLEAR;
    timer->tier = GTIMER_TIER_OVF_IT_EN;
    timer->tclr |= GTIMER_TCLR_START_STOP_CTRL | GTIMER_TCLR_AUTO_RELOAD;
}

void timer_tick_stop()
{
    gtimer_1ms *timer = timer_tick_get();
    timer->tclr &= ~GTIMER_TCLR_START_STOP_CTRL;    
}


/* ----------------------------------------------------- */

/* XXX: this handler is shared between MTU0:0 to MTU0:3,
 *      we should check for MTU0:0 before dispatching the timer callback
 */
return_value timer_tick_handler_stub(uint32_t r0, uint32_t r1, uint32_t r2)
{
    if(tick_handler)
        tick_handler(r0, r1, r2);
    
    /* clear interrupt: */
    timer_tick_clear_interrupt();
    
    return RV_OK;    
}    



void soc_timer_init()
{
    int i;
    gtimer *timer;
    
    
    // NORMAL TIMERS
    for(i = 0; i < GTIMER_COUNT; i++) {
        timer = timer_get(i);
        
        // XXX: stupid bug (?) work around, for now
        if((uint32_t) timer == 0x48042000 || (uint32_t) timer == 0x48048000 ) {
            printf("[timer_init] FPTIMER%d at 0x08lx was BYPASSED due to bug\n", i, timer);
            continue;
        }
        
        printf("[timer_init] GPTIMER%d at 0x%x is version %d.%d\n", i, timer,
               (timer->tidr >> 8) & 0x7, timer->tidr & 0x3F
               );
        
        // turn it off and clear interrupt
        timer->tclr &= ~GTIMER_TCLR_START_STOP_CTRL;
        timer->irqenable_set = 0;
    }
    
    
    // 1ms TIMER    
    // turn it off and clear interrupt
    timer_tick_stop();
    timer_tick_clear_interrupt();        
}


