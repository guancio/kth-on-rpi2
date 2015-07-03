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
//// 			TIMER STUFF
#define ARM_TIMER_LOD 0x3F00B400
#define ARM_TIMER_VAL 0x3F00B404
#define ARM_TIMER_CTL 0x3F00B408
#define ARM_TIMER_DIV 0x3F00B41C
#define ARM_TIMER_CNT 0x3F00B420

#define SYSTIMERCLO 0x3F003004

//NOTE: Not a GPIO address!
#define TIMEOUT 1000000

////////////////////////////////////////////////////////////////////////////////
//// 			LED STUFF
#define LED_GPFSEL 0x3F200010
#define LED_GPCLR 0x3F20002C
#define LED_GPSET 0x3F200020

////////////////////////////////////////////////////////////////////////////////
//// 			JTAG STUFF
#define GPFSEL0 0x3F200000
#define GPFSEL2 0x3F200008

////////////////////////////////////////////////////////////////////////////////
//			UART STUFF
//More information about this UART can be found in the documentation of
//	the BCM2835 at chapter 13, page 175.
////////////////////////////////////////////////////////////////////////////////
//Address map of the ARM PL011 UART-variant

// The base address for UART.
#define UART0_BASE 0x3F201000
//Data register
#define UART0_DR 0x3F201000
//???
#define UART0_RSRECR 0x3F201004
//Flag register
#define UART0_FR 0x3F201018
//Apparently unused according to the documentation
#define UART0_ILPR 0x3F201020
//Integer baud rate divisor
#define UART0_IBRD 0x3F201024
//Fractional baud rate divisor
#define UART0_FBRD 0x3F201028
//Line control register
#define UART0_LCRH 0x3F20102C
//Control register
#define UART0_CR 0x3F201030
//Interrupt FIFO level select register
#define UART0_IFLS 0x3F201034
//Interrupt mask set clear register
#define UART0_IMSC 0x3F201038
//Raw interrupt status register
#define UART0_RIS 0x3F20103C
//Masked interrupt status register
#define UART0_MIS 0x3F201040
//Interrupt clear register
#define UART0_ICR 0x3F201044
//DMA control register
#define UART0_DMACR 0x3F201048
//Test control register
#define UART0_ITCR 0x3F201080
//Integration test input register
#define UART0_ITIP 0x3F201084
//Integration test output register
#define UART0_ITOP 0x3F201088
//Test data register
#define UART0_TDR 0x3F20108C
////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//			UART FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

//Returns the length of a C-style string.
size_t strlen(const char* str){
	size_t ret = 0;
	while ( str[ret] != 0 )
		ret++;
	return ret;
}

void uart_write_char(unsigned char byte){
	//Wait for UART to become ready to transmit - wait while bit 5 in
	//UART_FR is set, meaning that the transmit FIFO is full.
	while (read_from_address(UART0_FR) & (1 << 5)){
		//Do nothing...
	}
	
	//When FIFO is enabled, data written to UART_DR will be put in the FIFO.
	write_to_address(UART0_DR, byte);
}
/*
unsigned char uart_getc(){
	while (read_from_address(UART0_FR) & (1 << 4)){
		//Do nothing...
	}
	return read_from_address(UART0_DR);
}
*/

//Writes all the chars in buffer to the UART.
void uart_write_chars(const unsigned char* buffer, size_t size){
	for (size_t i = 0; i < size; i++){
		uart_write_char(buffer[i]);
	}
}

//Simply a wrapper for uart_write_chars
void uart_write_string(const char* str){
	uart_write_chars((const unsigned char*) str, strlen(str));
}

////////////////////////////////////////////////////////////////////////////////
//			MAIN
////////////////////////////////////////////////////////////////////////////////

int kernel_main ( void ){
	unsigned int register_a;
	unsigned int register_b;

	//Pull-up/pull-down thingy procedure enabling us to write on GPIO addresses...
	write_to_address(GPPUD,0);
	for(register_a = 0; register_a < 150; register_a++){
		delay(register_a);
	}
	//... specifically, GPIO pins 4, 22, 24, 25 and 27 (JTAG) and others
	write_to_address(GPPUDCLK0,(1<<4)|(1<<14)|(1<<15)|(1<<21)|(1<<22)|(1<<24)|(1<<25)|(1<<27));
	for(register_a = 0; register_a < 150; register_a++){
		delay(register_a);
	}
	write_to_address(GPPUDCLK0,0);

	//Initialization of UART
	write_to_address(UART0_ICR, 0x7FF);
	write_to_address(UART0_IBRD, 1); //NOTE: Number not in hexadecimal
	write_to_address(UART0_FBRD, 40); //NOTE: Number not in hexadecimal
	write_to_address(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
	write_to_address(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
	write_to_address(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));

	//Initialization of JTAG
	//Set GPIO4 to alternative function 5 by writing to register at GPFSEL0s
	register_a = read_from_address(GPFSEL0);
	register_a &= ~(7<<12); //gpio4
	register_a |= 2<<12; //gpio4 alt5 ARM_TDI
	write_to_address(GPFSEL0, register_a);

	//Set other GPIOs to alternative functions by writing at GPFSEL2
	register_a = read_from_address(GPFSEL2);
	register_a &= ~(7<<6); //gpio22
	register_a |= 3<<6; //alt4 ARM_TRST
	register_a &= ~(7<<12); //gpio24
	register_a |= 3<<12; //alt4 ARM_TDO
	register_a &= ~(7<<15); //gpio25
	register_a |= 3<<15; //alt4 ARM_TCK
	register_a &= ~(7<<21); //gpio27
	register_a |= 3<<21; //alt4 ARM_TMS
	write_to_address(GPFSEL2, register_a);

	//Initialization of timer
	write_to_address(ARM_TIMER_CTL, 0x00F90000);
	write_to_address(ARM_TIMER_CTL, 0x00F90200);

	//Infinite loop with timed blinks
	register_b = read_from_address(ARM_TIMER_CNT);
	while(1){
		//LED on!
		write_to_address(LED_GPSET, 1<<15);
		uart_write_string("Hello, kernel world!\r\n");
		while(1){
			register_a = read_from_address(ARM_TIMER_CNT);
			if((register_a - register_b) >= TIMEOUT){
				break;
			}
		}
		register_b += TIMEOUT;
		//LED off!
		write_to_address(LED_GPCLR, 1<<15);
		uart_write_string("Goodbye, kernel world!\r\n");
		while(1){
			register_a = read_from_address(ARM_TIMER_CNT);
			if((register_a - register_b) >= TIMEOUT){
				break;
			}
		}
		register_b += TIMEOUT;
	}
	return(0);
}
