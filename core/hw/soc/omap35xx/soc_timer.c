#include <hw.h>

#include "soc_defs.h"


#define GTIMER_FIRST 1
//#define GTIMER_LAST 10
#define GTIMER_LAST 12


#define GTIMER_TCLR_START_STOP_CTRL (1 << 0)
#define GTIMER_TCLR_AUTO_RELOAD (1 << 1)


#define GTIMER_TISR_MAT_IT_FLAG_CLEAR (1 << 0)
#define GTIMER_TISR_OVT_IT_FLAG_CLEAR (1 << 1)
#define GTIMER_TISR_TCAR_IT_FLAG_CLEAR (1 << 2)

#define GTIMER_TIER_MAT_IT_EN  (1 << 0)
#define GTIMER_TIER_OVF_IT_EN  (1 << 1)
#define GTIMER_TIER_TCAR_IT_EN (1 << 2)

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
    
    /* these are not available in timer 3-9 an 11??
    uint32_t tpir;     
    uint32_t tnir;
    uint32_t tcvr;
    uint32_t tocr;
    uint32_t towr;    */
} volatile gtimer;

/* --------------------------------------------------------- */

static uint32_t TIMER_BASES [] = {
    0x48318000,
    0x49032000,
    0x49034000,
    0x49036000,
    0x49038000,
    0x4903A000,
    0x4903C000,
    0x4903E000,
    0x49040000,
    0x48086000,
    0x48088000,
    0x48304000
};

static gtimer *timer;

// ------------------------------------------------------
// timer support functions
// ------------------------------------------------------

gtimer *timer_get(int n)
{
    /* reserved for tick timer */
    if(n == 0) return 0;
    
    /* valid timer ? */
    if(n < GTIMER_FIRST || n > GTIMER_LAST) return 0;
            
    return (gtimer *) (IO_VA_ADDRESS(TIMER_BASES[n - GTIMER_FIRST]));
}

static gtimer *timer_tick_get()
{
    return (gtimer *) IO_VA_ADDRESS(TIMER_BASES[11]);
}

// --------------------------------------------------
// Tick timer
// --------------------------------------------------

static cpu_callback tick_handler = 0;

void timer_tick_clear_interrupt()
{
	timer->tisr = GTIMER_TISR_OVT_IT_FLAG_CLEAR;// | GTIMER_TISR_MAT_IT_FLAG_CLEAR | GTIMER_TISR_TCAR_IT_FLAG_CLEAR;
}

static return_value timer_tick_handler_stub(uint32_t irq, uint32_t r1, uint32_t r2 )
{
    if(tick_handler)
        tick_handler(irq, r1, r2);
    /* clear interrupt: */
    timer_tick_clear_interrupt();
    
    return RV_OK;
}







// --------------------------------------------------
// start a timer
// --------------------------------------------------

void timer_tick_start(cpu_callback handler)
{
/*DMTIMER as timer tick*/

	cpu_irq_set_enable(INTC_IRQ_TIMER7, FALSE);
	//timer = timer_tick_get();
	timer = timer_get(12);

/*	volatile uint32_t *gpt12_ick = 0xfa004c10;
	uint32_t v;
	v = *gpt12_ick;
	v |= 2;
	*gpt12_ick = v;*/

    timer->tsicr = 6; /*reset*/
    uint32_t i =0, l =0;
    /*wait for reset*/

	while (!(timer->tistat & 1)) {
		i++;
		if (i > 100000) {
			printf("Timer failed to reset\n");
		}
	}
	l = timer->tiocp_cfg;
	l |= (0x2 << 3);  /* Smart idle mode */
	l |= (0x2 << 8);   /* Perserve f-clock on idle */
	timer->tiocp_cfg = l;
	timer->tsicr = (1 << 2); /*Timer control, posted*/


	/*Period = 0x100 -1 = 0xff, load = 0xFFFFFFFF - period */

	timer->tldr = 0xffffff00;
	timer->tcrr = 0xffffff00; /*counter reg*/

    tick_handler = handler;

    cpu_irq_set_handler(INTC_IRQ_TIMER7, timer_tick_handler_stub);
    cpu_irq_set_enable(INTC_IRQ_TIMER7, TRUE);

    /* clear old status bits, set interrupt type and start it */
	timer->tclr |= GTIMER_TCLR_START_STOP_CTRL | GTIMER_TCLR_AUTO_RELOAD; /*Auto reload enable*/
    timer->tisr |= GTIMER_TISR_MAT_IT_FLAG_CLEAR | GTIMER_TISR_OVT_IT_FLAG_CLEAR | GTIMER_TISR_TCAR_IT_FLAG_CLEAR;
    timer->tier = GTIMER_TIER_OVF_IT_EN; /*TIMER INTERRUPT ENABLE*/
    timer->twer = GTIMER_TIER_OVF_IT_EN; /*TIMER WAKEUP ENABLE*/
}


// --------------------------------------------------
// initialize timer component
// --------------------------------------------------
void soc_timer_init()
{
    int i;
    gtimer *timer;
    
    for(i = GTIMER_FIRST; i <= GTIMER_LAST; i++) {
        timer = timer_get(i);        
        printf("[timer_init] GPTIMER%d at 0x%x is version %d.%d\n", i, timer,
               (timer->tidr >> 4) & 0xF, timer->tidr & 0xF
               );        
        // turn it off and clear interrupt
        timer->tclr |= ~GTIMER_TCLR_START_STOP_CTRL;
        timer->tisr |= GTIMER_TISR_MAT_IT_FLAG_CLEAR | GTIMER_TISR_OVT_IT_FLAG_CLEAR | GTIMER_TISR_TCAR_IT_FLAG_CLEAR;
    }    
}
