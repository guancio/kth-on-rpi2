

#include <hw.h>

#include "soc_defs.h"
#include "soc_ctrl.h"

clock_per * clock_reg = (clock_per *) CLOCK_PER;
 

// set clock and make sure it really is up
static void soc_clocks_safe_enable(volatile uint32_t *p)
{
    hwreg_write(p, CLOCK_PER_MODE_ENABLED);
    hwreg_wait(p, CLOCK_PER_MODE_MASK, CLOCK_PER_MODE_ENABLED);
}


static void soc_clocks_control_safe_enable(volatile uint32_t *p, int clocks)
{
    hwreg_update(p, 3, clocks); /* NO SLEEP */
    hwreg_wait(p, 3, 0);
}


/* ----------------------------------------------------------------------- */

void soc_ctrl_init()
{
    clock_per    * per  = (clock_per    *) CLOCK_PER;
    clock_wakeup * wkup = (clock_wakeup *) CLOCK_WAKEUP;
    
    
    /* TODO: we should first setup PLLs etc. */
    
    /* turn all on, CLKCTRL = NO SLEEP */    
    soc_clocks_control_safe_enable( &per->l4ls_clkstctrl, 0x03004500);
    soc_clocks_control_safe_enable( &per->l3s_clkstctrl, 0x00000008);
    soc_clocks_control_safe_enable( &per->l3_clkstctrl, 0x000000DC);  
    soc_clocks_control_safe_enable( &wkup->clkstctrl, 0x00001E10);
    
    
    /* peripherals clocks: enable everything we can use! */    
    soc_clocks_safe_enable(& per->uart1);    
    soc_clocks_safe_enable(& per->gpio1);
    soc_clocks_safe_enable(& per->gpio2);
    soc_clocks_safe_enable(& per->gpio3);
    soc_clocks_safe_enable(& per->timer2);
    soc_clocks_safe_enable(& per->timer3);
    soc_clocks_safe_enable(& per->timer4);
    soc_clocks_safe_enable(& per->timer5);
    soc_clocks_safe_enable(& per->timer6);
    soc_clocks_safe_enable(& per->timer7);
    soc_clocks_safe_enable(& per->tpcc); /* DMA channel controller */
    soc_clocks_safe_enable(& per->tptc0); /* DMA transfer controller 0 */
    soc_clocks_safe_enable(& per->tptc1); /* DMA transfer controller 1 */
    soc_clocks_safe_enable(& per->tptc2); /* DMA transfer controller 2 */
    
    soc_clocks_safe_enable(& wkup->gpio0);
    soc_clocks_safe_enable(& wkup->timer0);
    soc_clocks_safe_enable(& wkup->timer1);
    soc_clocks_safe_enable(& wkup->uart0);            
}

