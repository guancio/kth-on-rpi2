#include "hw.h"

static uart_registers *uart = 0; //TODO: Why assign zero?

//This function prints one character over the UART.
void stdio_write_char(int c){

	unsigned char* byte = (unsigned char*)&c;
	while (!stdio_can_write()){
		//Do nothing...
	}
	
	//When FIFO is enabled, data written to UART_DR will be put in the FIFO.
	uart->dr = byte;
}

//This function is called when waiting to be able to write.
//Returns TRUE if you can, FALSE otherwise.
extern int stdio_can_write(){
	//Binary AND of the Flag Register and a "1" in bit 6 (counting from right to left, starting at zero). If that bit is not set, you can write to the UART.
    return (uart->fr & (1 << 6)) == 0;            
}

//This function is called when waiting to be able to write.
//Returns TRUE if you can, FALSE otherwise.
extern int stdio_can_read(){
	//Binary AND of the Flag Register and a "1" in bit 5 (counting from right to left, starting at zero). If that bit is not set, you can read from the UART.
	return (uart->fr & (1 << 5)) == 0;
}

extern int stdio_read_char(){
	while (!stdio_can_read()){
		//Do nothing...
	}
	return uart->dr;   
}

/**********************************************************************/

//This might be needed later, but not when we have U-Boot.
void soc_uart_init(){
	//"uart" is now a struct located at UART_BASE.
	//The struct entries are located starting at UART_BASE and upwards every
	//four bytes.
	uart = (uart_registers *) UART_BASE;
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
	while (uart->fr & (1 << 7) == 0 || uart->fr & (1 << 4) == 0){
		//Do nothing...
	}
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
    
