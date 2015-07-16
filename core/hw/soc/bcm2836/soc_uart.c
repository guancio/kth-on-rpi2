#include "hw.h"
#include "soc_defs.h"

//The UART on the BCM2836 chip is based on, but not identical to, the PL011
//UART. It is described on page 175 onwards of the BCM2835 documentation.

typedef struct {
    uint32_t dr; //Data Register
    uint32_t rsrecr; //???
	uint32_t unused0[4];
	uint32_t fr; //Flag register
    uint32_t ilpr; //Not in use
    uint32_t ibrd; //Integer Baud rate divisor
    uint32_t fbrd; //Fractional Baud rate divisor
    uint32_t lcrh; //Line Control register
    uint32_t cr; //Control register
    uint32_t ifls; //Interrupt FIFO Level Select Register
    uint32_t imsc; //Interrupt Mask Set Clear Register
    uint32_t ris; //Raw Interrupt Status Register
    uint32_t mis; //Masked Interrupt Status Register
    uint32_t icr; //Interrupt Clear Register
    uint32_t dmacr; //DMA Control Register
    uint32_t unused1[13];
    uint32_t itcr; //Test Control Register
    uint32_t itip; //Integration test input register
    uint32_t itop; //Integration test output register
	uint32_t tdr; //Test data register
} volatile uart_registers;

static uart_registers *uart = 0; //TODO: Why assign zero?

//This function print a char over the UART.
//We pass an int even though the char is in the first 8 bits.
//Bits 8-11 (zero-indexed) contain various error messages (the rest is Reserved)
//- see page 179 of BCM2835 documentation. 
void stdio_write_char(int c){

	//unsigned char* byte = (unsigned char*)&c;
	while (!stdio_can_write()){
		//While we can't write, loop and do nothing...
	}
	
	//When FIFO is enabled, data written to UART_DR will be put in the FIFO.
	//It is imperative that bits (zero-indexed) 8-11 are not set, because then
	//They will be read as errors. Be careful when passing ints here!
	uart->dr = c;
}

//This function is called when waiting to be able to write.
//Returns TRUE if you can, FALSE otherwise.
extern int stdio_can_write(){
	//Binary AND of the Flag Register and a "1" in bit 5 (counting from right to
	//left, starting at zero). If that bit isn't set, you can write to the UART.
    return (uart->fr & (1 << 5)) == 0;            
}

//This function is called when waiting to be able to write.
//Returns TRUE if you can, FALSE otherwise.
extern int stdio_can_read(){
	//Binary AND of the Flag Register and a "1" in bit 4 (counting from right to
	//left, starting at zero). If that bit isn't set, you can read from the UART.
	return (uart->fr & (1 << 4)) == 0;
}

//This function reads a char from the UART.
//We return an int even though the char is in the first 8 bits.
//Bits 8-11 (zero-indexed) contain various error messages (the rest is Reserved)
//- see page 179 of BCM2835 documentation. 
extern int stdio_read_char(){

	//Wait for us to be able to read...
	while (!stdio_can_read()){
		//While not able to read, do nothing...
	}
	
	//Return the 32 bits in the DR register.
	return uart->dr;   
}

/**********************************************************************/

//Disables, then initializes the UART.
void soc_uart_init(){
	//"uart" is now a struct located at UART_BASE.
	//The struct entries are located starting at UART_BASE and upwards every
	//four bytes.
	uart = (uart_registers *)IO_VA_ADDRESS(UART_BASE);

	#if 0
	//*ms is probably not hard-coded...
	//What the heck is this and why would we need it???
    memspace_t *ms_uart;
    
    
    ms_uart = env_map_from(PROC_TYPE_HYPERVISOR, PROC_TYPE_HYPERVISOR,
                           "__soc_usart", USART0_BASE, sizeof(usart_registers) , TRUE);
        
    usart0 = (usart_registers *) ms_uart->vadr;
	#endif

	//TODO: Since we are not 100% the UART has not been initialized at this
	//point (U-Boot, et.c.) we might want to disable it first, and then do setup

	//TODO: Wait for end of transmission/reception - should be here??? Verify this works correctly.
	/*
	while (uart->fr & (1 << 7) == 0 || uart->fr & (1 << 4) == 0){
		//Do nothing...
	}
	*/

	//Disable the UART
	uart->cr = 0;
	//Flush the transmit FIFO
	uart->lcrh = (0 << 4);
	
	//Clear interrupts...
	uart->icr = 0x7FF;
	//Set integer part of Baud rate divisor to 1.
	uart->ibrd = 1;
	//Set fractional part of Baud rate divisor to 40.
	uart->fbrd = 40;
	//Enable FIFO and set word length to 8.
	uart->lcrh = (1 << 4) | (1 << 5) | (1 << 6);
	//Set all supported UART-related interrupt masks.
	uart->imsc = (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                        (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10);
	//Enable both transmission and reception over the UART (and nothing else).
	uart->cr = (1 << 0) | (1 << 8) | (1 << 9);
}
    
