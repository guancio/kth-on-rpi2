
#include <hw.h>

#include "soc_defs.h"

static BASE_REG uart = 0;

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

/**********************************************************************/
void soc_uart_init()
{
    uart = (BASE_REG) BASE_UART2;    
    
    uart[UART_CR] = 0;
    uart[UART_RSR] = 0;
    uart[UART_CR] = 1;
    
    /* 0x350014 => UARTCLK=38.4MHz, BRDI=20, DIVFRAC=53(0x35) (BRDF=0.8)*/    
    uart[UART_IBRD] = 0x0014;
    uart[UART_FBRD] = 0x0035;
    uart[UART_LCRH_TX] = 0x0070; /* 8N1 */        

    uart[UART_IFLS] = 0x1B; /* 8 chars FIFO interrupt */
    uart[UART_ICR] = 0x1FFF; /* clean interrupts */    
    uart[UART_IMSC] = 0x380; /* interrupt on overrun, break, parity and framing errors */    
    uart[UART_CR] |= 0x300; /* enable TX, RX */        
        
    printf("DEBUG: UART configured\n");        
}
    
