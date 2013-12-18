
#include <uclib.h>
#include "soc_defs.h"

static BASE_REG uart = (BASE_REG)(UART0_VA_BASE);


void stdio_write_char(int);

void printf_putchar(int c)
{
	stdio_write_char(c);
}

/*
 * Uart already setup at hypervisor startup to enable debug info
 * User only needs to provide this function to print to uart
 */

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
