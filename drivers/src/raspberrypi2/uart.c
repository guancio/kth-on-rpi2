#include <uclib.h>

#define UART_BASE          0x3F201000
#define IO_OFFSET				0xBB000000
#define IO_VA_ADDRESS(x)		((x) + IO_OFFSET)
#define UART_VA_BASE (IO_VA_ADDRESS(UART_BASE))

//These are offsets (in 4*byte) from the base address.
//UART_DR (data register) is 0
#define UART_DR_OFFSET      0

//UART_FR (flag register) is 0
#define UART_FR_OFFSET		6

//BASE_REG is simply a volatile unsigned 32-bit fixed-length integer
//This type of allocation stores the BASE_REG on memory location BASE_UART2.
static BASE_REG uart = (BASE_REG) UART_VA_BASE;

//This function returns 1 (true) if the UART is ready for writing, and 0 if the 
//UART is not ready for writing.
//You can write as long as the transmit FIFO is not full, so we check if the 6th
//bit of UART_FR is set.
int stdio_can_write(){
    if(uart) {
		//If bit 5 is set, that means that the transmit FIFO is full. That
		//means that there is no room to transmit more characters.
        return (uart[UART_FR_OFFSET] & (1 << 5)) == 0;
    }
    return 0;
}

//This function writes one character on the UART. Parameter is an int, even
//though we only write to 8 bits.
void stdio_write_char(int c){
    if(uart) {
        while(!stdio_can_write()){
			//Do nothing...
		}

		//This ensures us that no funny stuff happens with the registers other
		//than the first 8, which are supposed to hold a character in byte
		//format. TODO
		//char* byte = (char*)&c;

        uart[UART_DR_OFFSET] = c;      
    }
}

//This function reads one character from input.
int stdio_read_char(){
	if(uart) {
		while (!stdio_can_read()){
			//Do nothing...
		}
		return uart[UART_DR_OFFSET];
	}
	return 0;
}

//This function returns 0 if we cannot read a character (if base register is
//undefined or if there are no characters in the receive FIFO), and 1 otherwise.
int stdio_can_read(){
	if(uart){
		//If bit 4 is set, that means that the receive FIFO is empty. That
		//means that there are no characters to read.
    	return (uart[UART_FR_OFFSET] & (1 << 4)) == 0;
	}
	return 0;
}

//This wrapper function writes one character on the UART via stdio_write_char.
void printf_putchar(char c){
    if(c=='\n')
        stdio_write_char('\r');
    stdio_write_char(c);
}
