#include "hw.h"

/* context */
void soc_init()
{
    soc_ctrl_init();
    soc_uart_init();
    
    soc_interrupt_init();
    soc_timer_init();
}
