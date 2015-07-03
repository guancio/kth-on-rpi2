#include <stddef.h>

//Function prototypes and imports
extern void write_to_address(unsigned int, unsigned int);
extern unsigned int read_from_address(unsigned int);
extern void delay(unsigned int);

////////////////////////////////////////////////////////////////////////////////
////                     DEFINES
////////////////////////////////////////////////////////////////////////////////
// The GPIO registers' base address. This should work on RPi2.
#define GPIO0_BASE 0x3F200000
// Controls actuation of PUD (pull up/down) to ALL GPIO pins.
#define GPPUD 0x3F200094
// Controls actuation of pull up/down for specific GPIO pin.
#define GPPUDCLK0 0x3F200098

////////////////////////////////////////////////////////////////////////////////
//// 			LED STUFF
#define LED_GPFSEL 0x3F200010
#define LED_GPCLR 0x3F20002C
#define LED_GPSET 0x3F200020

void kernel_main(void){
	unsigned int register_a;

	//Initialize LED
	register_a = read_from_address(LED_GPFSEL);
	register_a |= (1 << 21);
	write_to_address(LED_GPFSEL, register_a);

	while(1){
		//Wait...
		for(register_a = 0; register_a < 500000; register_a++){
		    ;//Do nothing...
		}
		//Turn LED on
		write_to_address(LED_GPSET, 1<<15);

		//Wait...
		for(register_a = 0; register_a < 500000; register_a++){
		    ;//Do nothing...
		}
		//Turn LED off
		write_to_address(LED_GPCLR, 1<<15);
	}
}
