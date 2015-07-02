#include "hw.h"

//Initializes everything you need on the SoC.
void soc_init()
{
	soc_clocks_init();
	soc_interrupt_init();
	soc_timer_init();
	soc_uart_init();
}
