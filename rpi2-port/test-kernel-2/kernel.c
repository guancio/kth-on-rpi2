#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Simple loop variable */
volatile unsigned int tim;

///////////////////////////////////////////////////
 
static inline void mmio_write(uint32_t reg, uint32_t data){
	*(volatile uint32_t *)reg = data;
}
 
static inline uint32_t mmio_read(uint32_t reg){
	return *(volatile uint32_t *)reg;
}
 
/* Loop <delay> times in a way that the compiler won't optimize away. */
static inline void delay(int32_t count){
	__asm__ volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : : [count]"r"(count) : "cc");
}
 
size_t strlen(const char* str){
	size_t ret = 0;
	while ( str[ret] != 0 )
		ret++;
	return ret;
}
 
enum
{
	// The GPIO registers' base address. This should work on RPi2.
	GPIO0_BASE = 0x3F200000UL,

	// Controls actuation of PUD (pull up/down) to ALL GPIO pins.
	GPPUD = (GPIO0_BASE + 0x94),
	// Controls actuation of pull up/down for specific GPIO pin.
	GPPUDCLK0 = (GPIO0_BASE + 0x98),
	////////////////////////////////////////////////////////////////////////
	//// LED STUFF
	LED_GPFSEL = (GPIO0_BASE + 0x10),
	LED_GPCLR = (GPIO0_BASE + 0x2C),
	LED_GPSET = (GPIO0_BASE + 0x20),

	////////////////////////////////////////////////////////////////////////
	// The base address for UART. TODO: This is changed for RPi2.
	//From what we know about the difference in physical addresses between
	//	the RPi versions, and under the assumption that all other things
	//	are unchanged between the BCM 2835 and the BCM 2836, this is
	//	the base address of the ARM PL011 UART-variant of the board.
	UART0_BASE = 0x3F201000UL,

	//More information about this UART can be found in the documentation of
	//	the BCM2835 at chapter 13, page 175. TODO: "UART" has been
	//	replaced by "UART0" in these register names. Why?
	///////////////////////////////////////////
	//Address map of the ARM PL011 UART-variant

	//Data register
	UART0_DR     = (UART0_BASE + 0x00),

	//???
	UART0_RSRECR = (UART0_BASE + 0x04),

	//Flag register
	UART0_FR     = (UART0_BASE + 0x18),

	//Apparently unused according to the documentation
	UART0_ILPR   = (UART0_BASE + 0x20),

	//Integer baud rate divisor
	UART0_IBRD   = (UART0_BASE + 0x24),

	//Fractional baud rate divisor
	UART0_FBRD   = (UART0_BASE + 0x28),

	//Line control register
	UART0_LCRH   = (UART0_BASE + 0x2C),

	//Control register
	UART0_CR     = (UART0_BASE + 0x30),

	//Interrupt FIFO level select register
	UART0_IFLS   = (UART0_BASE + 0x34),

	//Interrupt mask set clear register
	UART0_IMSC   = (UART0_BASE + 0x38),

	//Raw interrupt status register
	UART0_RIS    = (UART0_BASE + 0x3C),

	//Masked interrupt status register
	UART0_MIS    = (UART0_BASE + 0x40),

	//Interrupt clear register
	UART0_ICR    = (UART0_BASE + 0x44),

	//DMA control register
	UART0_DMACR  = (UART0_BASE + 0x48),

	//Test control register
	UART0_ITCR   = (UART0_BASE + 0x80),

	//Integration test input register
	UART0_ITIP   = (UART0_BASE + 0x84),

	//Integration test output register
	UART0_ITOP   = (UART0_BASE + 0x88),

	//Test data register
	UART0_TDR    = (UART0_BASE + 0x8C),
	////////////////////////////////////////////
};

void uart_init(){
	/*
	1. zero the UART_CR register. In particular, zeroing the first
	bit disables the UART.
	*/
	mmio_write(UART0_CR, 0x00000000);
 
	/*2. zero the GPPUD register (which is not specific to the UART).
	Zeroing the two first two bits prepares to disable the built-in
	pull-up/pull-down resistors for ALL GPIO pins. These normally
	ensure that the pins are not susceptible to random
	electromagnetic fluctuations. This must be used in conjunction
	with the GPPUDCLK registers, which tell which pins to apply the
	operations set in GPPUD on.

	Then wait for 150 CPU cycles, which the documentation specifies.
	This will provide set-up time for the control signal.
	*/
	mmio_write(GPPUD, 0x00000000);
	delay(150);

	//FIXME: At this point, the program crashed before.
	//This might have to do with that the following lines removing the GPPUD
	//were separated from this line by an infinite loop.
	//3. Write to the 15th and 16th bits on the GPPUDCLK0, which gives
	//the instruction assigned to GPUPUD above to the GPIO pins with
	//BCM number 14 and 15, since the BCM pins are zero-indexed.

	//Then wait for 150 CPU cycles, which the documentation specifies.
	//This will provide halt time for the control signal.
	mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);
	
	//(4. At this point, the documentation specifies that you should
	//write to GPPUD to remove the control signal (?). However, since
	//our signal was just zeroes, we do not do this.) TODO: Test 
	//to add zeroing of GPPUD in any case...
	mmio_write(GPPUD, 0x00000000);
 
	//5. However, we must still write to GPPUDCLK0 to remove the clock
	//(terminology?) assigning the control signal to a particular pin.
	mmio_write(GPPUDCLK0, 0x00000000);
 
	//6. Now, we clear pending UART interrupts by writing 1s to the 11
	//first bits of UART_ICR. Some of the 11 first bits are
	//"don't care" bits, but most of them correspond to various UART
	//interrupts, which are then cleared.
	mmio_write(UART0_ICR, 0x7FF);

	/*7. Now, we want to set the UART_IBRD register, which holds the
	integer part of the baud rate divisor value. This is calculated
	as {baud rate divisor}=({uart reference clock}/(16*{Baud rate}))
	In our case, we want baud rate 115200 (pretty much standard) and
	UART reference clock is defined as 3 MHz. Apparently standard
	UART reference clocks are all multiples of 1.8432 MHz, and so it
	is quite possible, if not likely, that the value of 3 MHz makes
	a lot of kittens die somewhere.

	The baud rate divisor evaluates to 1,627604167 with the values
	above and so the integer part is 1, with the fractional part
	being 0,627604167. Since the UART0_FBRD holds a 6-bit number,
	we want to approximate 0,627604167 as a fraction of 64
	(this is a convention). This can be done by the formula
	(Fractional part * 64) + 0.5, which is about 40,6. TODO: Set to 41?
	*/
	mmio_write(UART0_IBRD, 1); //NOTE: Number not in hexadecimal
	mmio_write(UART0_FBRD, 40); //NOTE: Number not in hexadecimal
 
	//8. We write 1s to bit 4 in UART_LCRH, which enables FIFOs.
	//Writing 1s to bits 5 and 6 will set word length to 8.
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
 
	//9. We write 1s to all bits in UART0_IMSC (the interrupt mask
	//set/clear register), which sets the masks.
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
 
	//10. Writing 1 to bit 0 in UART0_CR (the control register) will
	//enable the UART, and bits 8 and 9 enable transmit/receive.
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}
 
void uart_putc(unsigned char byte){
	//Wait for UART to become ready to transmit - wait while bit 5 in
	//UART_FR is set, meaning that the transmit FIFO is full.
	while ( mmio_read(UART0_FR) & (1 << 5) ) { }
	
	//When FIFO is enabled, data written to UART_DR will be put in the FIFO.
	mmio_write(UART0_DR, byte);
}
 
unsigned char uart_getc(){
	//Wait for UART to become ready to receive - wait while bit 4 in
	//UART_FR is set, meaning that the receive FIFO is empty.
	while ( mmio_read(UART0_FR) & (1 << 4) ) { }

	//TODO: if the FIFOs are enabled, the data byte and the 4-bit status
	//(break, frame, parity, and overrun) is pushed onto the 12-bit wide re
	//ceive FIFO 
	return mmio_read(UART0_DR);
}
 
void uart_write(const unsigned char* buffer, size_t size){
	//Writes all the chars in buffer to the UART.
	for ( size_t i = 0; i < size; i++ )
		uart_putc(buffer[i]);
}

//Simply a wrapper for uart_write
void uart_puts(const char* str){
	uart_write((const unsigned char*) str, strlen(str));
}

void blink (){
	while(1){
		//Wait...
		for(tim = 0; tim < 500000; tim++){
		    ;//Do nothing...
		}
		//Turn OK LED on
		mmio_write(LED_GPSET,(1 << 15));

		//Wait...
		for(tim = 0; tim < 500000; tim++){
		    ;//Do nothing...
		}
		//Turn OK LED off
		mmio_write(LED_GPCLR, (1 << 15));
	}
}


#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags){
	(void) r0;
	(void) r1;
	(void) atags;
	//Initialize LED
	//mmio_write(LED_GPFSEL, (1 << 21));

 	//Initialize UART
	//uart_init();
	//Initialize LED
	//mmio_write(LED_GPFSEL, (1 << 21));
	//blink();
	while(1){
		for(tim = 0; tim < 900000; tim++){
		    ;//Do nothing...
		}
		uart_puts("Hello, kernel World!\r\n");

		//Wait...
		for(tim = 0; tim < 900000; tim++){
		    ;//Do nothing...
		}
		uart_puts("Goodbye, kernel World!\r\n");
	}
}
