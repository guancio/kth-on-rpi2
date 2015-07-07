#include <uclib.h>

#define BASE_UART2          0x3F201000

//These are offsets (in 4*byte) from the base address.
//UART_DR is 0
#define UART_DR_OFFSET      0

//Last 16 of this.
#define UART_FR_OFFSET		6




//BASE_REG is simply a volatile unsigned 32-bit fixed-length integer
//This type of allocation stores the BASE_REG on memory location BASE_UART2.
static BASE_REG uart = (BASE_REG) BASE_UART2;

//This function returns 1 (true) if the UART is ready for writing, and 0 if the UART is not ready for writing.
//You can write as long as the transmit FIFO is not full, so we check the 6th bit of UART_FR.
int stdio_can_write(){
    if(uart) {
		//Originally left shift 5, we change this to left shift 20
		//5 + 15 = 20 (since 0-indexation)
        return (uart[UART_FR_OFFSET] & (1 << 6)) == 0;
    }    
    return 0;
}

void stdio_write_char(int c)
{
    if(uart) {
	//Originally left shift 5, we change this to left shift 20
        while(!stdio_can_write()){
			//Do nothing...
		}
	char* byte = (char*)&c;
        uart[UART_DR_OFFSET] = byte;      
    }
}

int stdio_read_char()
{
	if(uart) {
		while (!stdio_can_read()){
			//Do nothing...
		}
		return uart[UART_DR_OFFSET];
	}
	return 0;
}


int stdio_can_read()
{
	if(uart){
    	return (uart[UART_FR_OFFSET] & (1 << 5)) == 0;
	}
	return 0;
}


void printf_putchar(char c)
{
    stdio_write_char(c);
}
