#ifndef _SOC_UART_H_
#define _SOC_UART_H_

//Again, what kind of address is this?
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

extern void soc_uart_init();

#endif /* _SOC_UART_H_ */
