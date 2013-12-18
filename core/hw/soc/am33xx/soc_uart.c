
#include <hw.h>

#include "soc_defs.h"


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

static uart_registers *uart_ = 0;

/* ------------------------------------------------ */


/* stdio */
void stdio_write_char(int c)
{
    uart_registers *uart = uart_;
    if(!uart) return;
    
    while(!stdio_can_write()) /* wait */ ;
        
    uart = uart_;    
    uart->op.rhr = c; /* = THR */
    
    if(c == '\n') {
        while(!stdio_can_write()) /* wait */ ;
        uart->op.rhr = '\r'; /* = THR */
    }    
}
int stdio_read_char()
{
    if(stdio_can_read()) {
        uart_registers *uart = uart_;
        return uart->op.rhr;    
    }
    return -1;
}

int stdio_can_write()
{
   uart_registers *uart = uart_;
    
    /* is uart configured?? */
    if(!uart) return FALSE;
    
    /* switch to OP mode if needed */
    if( (uart->op.lcr & UART_LCR_DIV_EN))
        uart->op.lcr = UART_OP_FLAGS;
        
    return (uart->op.lsr & UART_LSR_TX_FIFO_E) ? TRUE : FALSE;
}
int stdio_can_read()
{
    uart_registers *uart = uart_;
    /* is uart configured?? */
    if(!uart) return FALSE;
    
    /* switch to OP mode if needed */
    if( (uart->op.lcr & UART_LCR_DIV_EN))
        uart->op.lcr = UART_OP_FLAGS;
    
    return  (uart->op.lsr & UART_LSR_RX_FIFO_E) ? FALSE : TRUE;
}


void soc_uart_set(uint32_t *base, int pad_rx, int pad_tx, int pad_cts, int pad_rts)
{
    uart_registers *uart;        
    // 1. use uart3
    uart_ = (uart_registers *) base;    
    uart = uart_;    
    
    // 2. set pad mode    
    if(pad_rx > 0)  gpio_set_pad_configuration(pad_rx , 0, TRUE , FALSE, FALSE);
    if(pad_tx > 0)  gpio_set_pad_configuration(pad_tx , 0, FALSE, FALSE, FALSE);
    if(pad_cts > 0) gpio_set_pad_configuration(pad_cts, 0, TRUE , TRUE , FALSE); // pull down
    if(pad_rts > 0) gpio_set_pad_configuration(pad_rts, 0, FALSE, FALSE, FALSE);
    
       
    // 3. reset UART
    uart->op.sysc = UART_SYSC_SOFTRESET;
    while( !(uart->op.syss & UART_SYSS_RESETDONE))
        ;
    
    // 4. configure it
    uart->op.lcr = UART_OP_FLAGS; /* MODE OP */
    uart->op.ier = 0x00;
    uart->op.mdr1 = UART_MDR1_MODE_DISABLE;
    uart->op.lcr = UART_LCR_DIV_EN | UART_LCR_LENGTH_8; /* mode A */
    uart->a.dll = 0x1A; /* 115200 bps */
    uart->a.dlh = 0;
    uart->a.lcr = UART_OP_FLAGS; /* mode OP */
    uart->op.mcr = UART_MCR_RTS | UART_MCR_DTR;
    uart->op.iir /* FCR */ = UART_FCR_FIFO_EN | UART_FCR_RX_CLEAR | UART_FCR_TX_CLEAR;
    uart->op.mdr1 = UART_MDR1_MODE_UART16;       
}


void soc_uart_init()
{
    // now, configure UART0:
    soc_uart_set(UART0_BASE,
                 CONTROL_MODULE_PADCONFIG_UART0_RDX,
                 CONTROL_MODULE_PADCONFIG_UART0_TXD,
                 CONTROL_MODULE_PADCONFIG_UART0_CTSN,
                 CONTROL_MODULE_PADCONFIG_UART0_RTSN
                 );    
}
