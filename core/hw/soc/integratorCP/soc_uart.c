#include "hw.h"
#include "soc_defs.h"

static BASE_REG uart = 0;


void stdio_write_char(int c)
{

	if(uart) {
		while(uart[UART_FR] & (FR_TXFF));
		uart[UART_DR] = c;
		if(c == '\n') {
			while(uart[UART_FR] & (FR_TXFF));
			uart[UART_DR] = '\r';
		}
	}
}


extern int stdio_read_char()
{
    return -1; /* TODO */    
}

extern int stdio_can_write()
{
	if(uart) {
		return (uart[UART_FR] & FR_TXFF) == 0;
	}
	return 0;

}
extern int stdio_can_read()
{
    return 0; /* TODO */
}

/**********************************************************************/
void soc_uart_init()
{
	uint32_t divider, remainder, fraction;
	uart = (BASE_REG)(UART0_VA_BASE);
    //usart_registers *usart0 = (usart_registers *) USART0_BASE;

	uart[UART_CR] = 0; //disable everything
	uart[UART_ICR] = 0x7ff; //Clear all interrupt status

	//IBRD = UART_CLK / (16 * BAUD_RATE)
	//FBRD = ROUND ((64 * MOD(UART_CLK,(16 *BAUD_RATE))) / (16 * BAUD_RATE)
	divider = UART_CLK / (16 * BAUD_RATE);
	remainder = UART_CLK % (16 * BAUD_RATE);
	fraction = (8 * remainder / BAUD_RATE) >> 1;
	fraction += (8 * remainder / BAUD_RATE) & 1;

	uart[UART_IBRD] = divider;
	uart[UART_FBRD] = fraction;

	// Set N, 8, 1, FIFO Enable
	uart[UART_LCR_H] = (LCRH_WLEN8 | LCRH_FEN);

	//enable
	uart[UART_CR] = (CR_RXE | CR_TXE | CR_UARTEN);
	printf("Uart controller setup\n");

}
    
