
#include <uclib.h>

#define UART3_VA_BASE (IO_VA_ADDRESS(UART3_BASE))

#define UART1_BASE 0x4806A000
#define UART2_BASE 0x4806C000
#define UART3_BASE 0x49020000

#define IO_BASE 				0xF0000000
#define IO_OFFSET				0xB2000000
#define IO_VA_ADDRESS(x)		((x) + IO_OFFSET)

#define UART_LCR_MODE_OP 0
#define UART_LCR_MODE_A 0x80
#define UART_LCR_MODE_B 0xBF

#define UART_LCR_DIV_EN (1UL << 7)

#define UART_LCR_LENGTH_8 ( 3UL << 0)
#define UART_LCR_STOP_1  (0UL << 2)

#define UART_LSR_TX_FIFO_E (1UL << 5)
#define UART_LSR_RX_FIFO_E (1UL << 0)

#define UART_OP_FLAGS  (UART_LCR_MODE_OP | UART_LCR_STOP_1 | UART_LCR_LENGTH_8)


typedef union {
    struct {
        uint32_t dll;
        uint32_t dlh;
        uint32_t iir;
        uint32_t lcr;
        uint32_t mcr;
        uint32_t lsr;
        uint32_t msr;
        uint32_t tlr; /* and SPR */
        uint32_t mdr1;
        uint32_t mdr2;
        uint32_t unuse1[4];
        uint32_t usar;
        uint32_t unused2;
        uint32_t scr;
        uint32_t ssr;
        uint32_t unused3[2];    
        uint32_t mvr;    
        uint32_t sysc;
        uint32_t syss;
        uint32_t wer;
    } a;
    
    struct {
        uint32_t dll;
        uint32_t dlh;
        uint32_t efr;
        uint32_t lcr;
        uint32_t xon1_addr1;
        uint32_t xon2_addr2;
        uint32_t xoff1;
        uint32_t tlr; /* and xoff2 */
        uint32_t mdr1;
        uint32_t mdr2;
        uint32_t unuse1[4];
        uint32_t usar;
        uint32_t unused2;
        uint32_t scr;
        uint32_t ssr;
        uint32_t unused3[2];    
        uint32_t mvr;    
        uint32_t sysc;
        uint32_t syss;
        uint32_t wer;
    } b;
    
    struct {
        uint32_t rhr;
        uint32_t ier;
        uint32_t iir;
        uint32_t lcr;
        uint32_t mcr;
        uint32_t lsr;
        uint32_t msr;
        uint32_t tlr; /* and TCR */
        uint32_t mdr1;
        uint32_t mdr2;
        uint32_t unuse1[6];
        uint32_t scr;
        uint32_t ssr;
        uint32_t unused3[2];
        uint32_t mvr;    
        uint32_t sysc;
        uint32_t syss;
        uint32_t wer;
    } op;
} volatile uart_registers;

/* ------------------------------------------------ */

static uart_registers *uart = (uart_registers *) UART3_VA_BASE;

/* ------------------------------------------------ */
/* stdio */

int stdio_can_write()
{
    /* switch to OP mode if needed */
    if( (uart->op.lcr & UART_LCR_DIV_EN))
        uart->op.lcr = UART_OP_FLAGS;

    return (uart->op.lsr & UART_LSR_TX_FIFO_E) ? TRUE : FALSE;
}

int stdio_can_read()
{
    /* switch to OP mode if needed */
    if( (uart->op.lcr & UART_LCR_DIV_EN))
        uart->op.lcr = UART_OP_FLAGS;

    return  (uart->op.lsr & UART_LSR_RX_FIFO_E) ? FALSE : TRUE;
}

void stdio_write_char(int c)
{
    while(!stdio_can_write()) /* wait */ ;
        
    uart->op.rhr = c; /* = THR */
    
    if(c == '\n') {
        while(!stdio_can_write()) /* wait */ ;
        uart->op.rhr = '\r'; /* = THR */
    }    
}
int stdio_read_char()
{
    if(stdio_can_read()) {
        return uart->op.rhr;    
    }
    return -1;
}


/* ----------------------------------------------------------------------- */

void printf_putchar(char c)
{
    stdio_write_char(c);
}
