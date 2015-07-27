#include "hw.h"
#include "soc_defs.h"
extern void delay();
extern void debug_breakpoint(); //TODO: Remove this line after debugging.

//Since the Raspberry Pi 2 Model B does not have any JTAG pins enabled by
//default, we should/must enable them at this stage, or maybe even earlier if
//that is possible.

//The other parts of this file concern enabling writing on the GPIO pins we want
//to use.

//A struct with all the registers would be 

typedef struct {
    uint32_t gppud; //GPIO pull-up/pull-down register
	uint32_t gppudclk0; //GPIO pull-up/pull-down Clock register 0
	uint32_t gppudclk1; //GPIO pull-up/pull-down Clock register 1
} volatile gppud_registers;

typedef struct {
    uint32_t gpfsel0;
    uint32_t gpfsel1;
    uint32_t gpfsel2;
    uint32_t gpfsel3;
    uint32_t gpfsel4;
    uint32_t gpfsel5;
} volatile function_select_registers;

static gppud_registers *gppud = 0;
static function_select_registers *fsel = 0;

//Enables writing to the GPIO pins we want to use.
void soc_gpio_init(){
	unsigned int register_a;
	//gppud_registers *gppud;
	gppud = (gppud_registers *)IO_VA_ADDRESS(GPPUD_BASE);

	//Control signal: Disable pull-up/pull-down on GPIO pin 
	//determined by GPPUDCLK.
	gppud->gppud = 0;

	//Delay for at least 150 CPU cycles (set-up time of control 
	//signal).
	for(register_a = 0; register_a < 150; register_a++){
		delay();
	}

	//Writing a "1" to a bit in GPPUDCLK0 or GPPUDCLK1 allows us to 
	//select that bit (bits in GPPUDCLK1 are transposed by 32) for 
	//control.
	//JTAG:		4 (Alt5: ARM_TDI), 22 (Alt4: ARM_TRST), 24 (Alt4: ARM_TDO),
	//			25 (Alt4: ARM_TCK), 27 (Alt4: ARM_TMS)
	//UART:		14 (Alt1: TXD0), 15 (Alt1: RXD0)
	////TODO: (currently removed) LED:		47 (Alt1: OK LED)	
	gppud->gppudclk0 = (1<<4)|(1<<14)|(1<<15)|(1<<22)|(1<<24)|(1<<25)|(1<<27);

	////gppud->gppudclk1 = (1<<15);
	//Delay for at least 150 CPU cycles (holding time of control signal).
	for(register_a = 0; register_a < 150; register_a++){
		delay();
	}

	//Remove the clock and control signal
	gppud->gppud = 0;
	gppud->gppudclk0 = 0;
	////gppud->gppudclk1 = 0;

	//////////////////////////////////////////////////////////////////////
	//Now, initialize certain pins on the RPi2 to function as JTAG pins.
	//This is only needed for SoCs which do not have JTAG pins by default.
	//////////////////////////////////////////////////////////////////////
	//function_select_registers *fsel;
	fsel = (function_select_registers *)IO_VA_ADDRESS(GPFSEL_BASE);

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

	//Now, physical pins number 7 (GPIO 4), 13 (GPIO 27), 15 (GPIO 22),
	//18 (GPIO 24) and 22 (GPIO 25) should work as JTAG pins with their
	//respective signals.
}

