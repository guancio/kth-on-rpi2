#ifndef _SOC_UART_H_
#define _SOC_UART_H_

//The base address of the UART.
#define UART0_BASE 0x3F201000

typedef struct {
    uint32_t dr; //Data Register
    uint32_t rsrecr; //???
	uint32_t unused0[4];
	uint32_t fr; //Flag register
    uint32_t ilpr; //Not in use
    uint32_t ibrd; //Integer Baud rate divisor
    uint32_t fbrd; //Fractional Baud rate divisor
    uint32_t lcrh; //Line Control register
    uint32_t cr; //Control register
    uint32_t ifls; //Interrupt FIFO Level Select Register
    uint32_t imsc; //Interrupt Mask Set Clear Register
    uint32_t ris; //Raw Interrupt Status Register
    uint32_t mis; //Masked Interrupt Status Register
    uint32_t icr; //Interrupt Clear Register
    uint32_t dmacr; //DMA Control Register
    uint32_t unused1[13];
    uint32_t itcr; //Test Control Register
    uint32_t itip; //Integration test input register
    uint32_t itop; //Integration test output register
	uint32_t tdr; //Test data register
} volatile uart_registers;

extern void soc_uart_init();

#endif /* _SOC_UART_H_ */
