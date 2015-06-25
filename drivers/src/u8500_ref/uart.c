
#include <uclib.h>


#define BASE_UART2          0x80007000
#define UART_DR                0
#define UART_FR                6

static BASE_REG uart = (BASE_REG) BASE_UART2;

int stdio_can_write()
{
    if(uart) {
        return (uart[UART_FR] & (1UL << 5)) == 0;
    }    
    return 0;
}

void stdio_write_char(int c)
{
    if(uart) {
        while(uart[UART_FR] & (1UL << 5)) ;
        uart[UART_DR] = c;
        
        if(c == '\n') {
            while(uart[UART_FR] & (1UL << 5)) ;
            uart[UART_DR] = '\r';
        }        
    }
}

int stdio_read_char()
{
    return -1; /* TODO */    
}


int stdio_can_read()
{
    return 0; /* TODO */
}


void printf_putchar(char c)
{
    stdio_write_char(c);
}
