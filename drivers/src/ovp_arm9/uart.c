
#include <uclib.h>


#define USART0_BASE 0xFFFD0000

typedef struct {
    uint32_t cr;
    uint32_t mr;
    uint32_t ier;
    uint32_t idr;
    uint32_t imr;
    uint32_t csr;
    uint32_t rhr;
    uint32_t thr;
    uint32_t brgr;
    uint32_t rtor;
    uint32_t ttgr;
    uint32_t unused0;
    uint32_t rpr;
    uint32_t rcr;
    uint32_t tpr;
    uint32_t tcr;
} volatile usart_registers;


#define USART_BUFFER_SIZE 16
static char buffer_out[USART_BUFFER_SIZE];

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
    usart_registers *usart0 = (usart_registers *) USART0_BASE;
    while(usart0->tcr != 0)
        ;

    buffer_out[0] = c;
    usart0->tpr = (uint32_t)buffer_out;
    usart0->tcr = 1;
}
