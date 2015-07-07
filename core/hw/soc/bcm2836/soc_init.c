#include "hw.h"

//Initializes everything you need on the SoC.
//TODO: Disable non-essential function calls here - or disable the content of the functions.
void soc_init(){
	soc_gppud_init(); //This must be first.
	soc_jtag_init();
	//soc_clocks_init(); //TODO: What do we need this for?
	soc_interrupt_init();
	soc_timer_init();
	soc_uart_init();
}
