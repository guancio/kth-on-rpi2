#include "hw.h"

//Since the Raspberry Pi 2 Model B does not have any JTAG pins enabled by default, we should/must enable them at this stage, or maybe even earlier if that is possible.

static function_select_registers *fsel = 0;

//Initializes certain pins on the RPi2 to function as JTAG pins.
soc_jtag_init(){
	fsel = (function_select_registers *) GPFSEL_BASE;
	unsigned int register_a;

	//Set GPIO4 to alternative function 5 by writing to GPFSEL0.
	register_a = fsel->gpfsel0; //Now holds the 32 bits of register GPFSEL0.
	register_a &= ~(7<<12); //Clear settings for GPIO4.
	register_a |= 2<<12; //Set GPIO4 to alternative function 5 (ARM_TDI).
	fsel->gpfsel0 = register_a;

	//Set other GPIOs to alternative functions by writing at GPFSEL2.
	register_a = fsel->gpfsel2; //Now holds the 32 bits of register GPFSEL2.
	register_a &= ~(7<<6); //Clear settings for GPIO22.
	register_a |= 3<<6; //Set GPIO22 to alternative function 4 (ARM_TRST).
	register_a &= ~(7<<12); //Clear settings for GPIO24.
	register_a |= 3<<12; //Set GPIO24 to alternative function 4 (ARM_TDO).
	register_a &= ~(7<<15); //Clear settings for GPIO25.
	register_a |= 3<<15; //Set GPIO25 to alternative function 4 (ARM_TCK).
	register_a &= ~(7<<21); //Clear settings for GPIO27.
	register_a |= 3<<21; //Set GPIO27 to alternative function 4 (ARM_TMS).
	fsel->gpfsel2 = register_a;
}
