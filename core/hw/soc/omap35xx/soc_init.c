#include "hw.h"

void irq_handler();

/* context */
void soc_init()
{
//  soc_ctrl_init();
//  soc_gpio_init();
    soc_uart_init();
    
    soc_interrupt_init();
    //soc_timer_init();
	/*IRQ 0x4A for UART
	 *For Linux, add support to let guest register its own interrupts */
	cpu_irq_set_handler(0x4a, (cpu_callback)irq_handler);
}
