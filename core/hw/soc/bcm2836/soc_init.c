#include <hw.h>
#include <soc_defs.h>
extern void soc_gpio_init();
extern void soc_interrupt_init();
extern void soc_timer_init();
extern void soc_uart_init();

//Initializes everything you need on the SoC.
void soc_init(){
	soc_gpio_init(); //This must be first - enables GPIO pins for writing.
	//soc_clocks_init(); //TODO: What do we need this for?
	soc_interrupt_init();
	soc_timer_init();
	soc_uart_init();
}
