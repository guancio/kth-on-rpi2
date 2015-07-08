#include "hw.h"

typedef struct {
    uint32_t gppud; //GPIO pull-up/pull-down register
	uint32_t gppudclk0; //GPIO pull-up/pull-down Clock register 0
	uint32_t gppudclk1; //GPIO pull-up/pull-down Clock register 1
} volatile gppud_registers;

static gppud_registers *gppud = 0;

//Should delay the CPU with [count] cycles.
static inline void delay(int32_t count){
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : : [count]"r"(count) : "cc");
}

//Enables writing to the GPIO pins we want to use.
void soc_gppud_init(){
	gppud = (gppud_registers *)IO_VA_ADDRESS(GPPUD_BASE);

	//Control signal: Disable pull-up/pull-down on GPIO pin determined by
	//GPPUDCLK.
	gppud->gppud = 0;

	//Delay for at least 150 CPU cycles (set-up time of control signal).
	delay(150);

	//Writing a "1" to a bit in GPPUDCLK0 or GPPUDCLK1 allows us to select
	//that bit (bits in GPPUDCLK1 are transposed by 32) for control.
	//JTAG:		4 (Alt5: ARM_TDI), 22 (Alt4: ARM_TRST), 24 (Alt4: ARM_TDO),
	//			25 (Alt4: ARM_TCK), 27 (Alt4: ARM_TMS)
	//UART:		14 (Alt1: TXD0), 15 (Alt1: RXD0)
	//TODO: (currently removed) LED:		47 (Alt1: OK LED)	
	gppud->gppudclk0 = (1<<4)|(1<<14)|(1<<15)|(1<<22)|(1<<24)|(1<<25)|(1<<27);

	////gppud->gppudclk1 = (1<<15);
	//Delay for at least 150 CPU cycles (holding time of control signal).
	delay(150);

	//Remove the clock and control signal
	gppud->gppud = 0;
	gppud->gppudclk0 = 0;
	////gppud->gppudclk1 = 0;
}
