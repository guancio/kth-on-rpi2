#ifndef _SOC_UART_H_
#define _SOC_UART_H_

//The base address of the UART.
#define UART0_BASE 0x3F201000

typedef struct {
    uint32_t dr;
    uint32_t rsrecr;
	uint32_t unused0;
	uint32_t unused1;
	uint16_t unused2;
	uint16_t fr; 

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
} volatile uart_registers;

extern void soc_uart_init();

#endif /* _SOC_UART_H_ */
