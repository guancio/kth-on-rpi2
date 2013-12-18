#include "hw.h"

void soc_init()
{
	soc_clocks_init();
	soc_interrupt_init();
	soc_timer_init();
	soc_uart_init();
}
