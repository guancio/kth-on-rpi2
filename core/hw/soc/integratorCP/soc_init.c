#include "hw.h"

extern __hyper_pt_start__;

void irq_handler();

void soc_init()
{

	soc_uart_init();
	soc_interrupt_init();
	soc_timer_init();
	printf("IntegratorCP SOC initialized\n");

	/*IRQ 1 for UART*/
	cpu_irq_set_handler(1, (cpu_callback)irq_handler);
}
