
#ifndef _SOC_DEFS_H_
#define _SOC_DEFS_H_

/* SOC or CPU generic stuff */
#define PERIPH_BASE 0xA0410000

#define GICD_BASE (PERIPH_BASE + 0x1000)
#define GICC_BASE (PERIPH_BASE + 0x0100)

/* PRCMU */
#define PRCMU_BASE 0x80157000
#define PRCMU_UARTCLK_MGT 6
#define PRCMU_PER1_MGT 11
#define PRCMU_PER2_MGT 12
#define PRCMU_PER3_MGT 13
#define PRCMU_PER5_MGT 14
#define PRCMU_PER6_MGT 15
#define PRCMU_PER7_MGT 16
#define PRCMU_TCR (460/4)

#define PRCM_PLLDIV 0
#define PRCM_CLKEN (1UL << 8)
#define PRCM_CLK38 (1UL << 9)
#define PRCM_CLK38SRC (1UL << 10)
#define PRCM_CLK38DIV (1UL << 11)


/* PRCC */
#define PRCC_PCKEN              0
#define PRCC_PCKDIS             1
#define PRCC_KCKEN              2
#define PRCC_KCKDIS             3
#define PRCC_PCKSR              4
#define PRCC_PKCKSR             5

/* GPIO */
#define GPIO0_BASE 0x8012E000
#define GPIO_AFSLA (0x20 / 4)
#define GPIO_AFSLB (0x24 / 4)

/* UART */
#define BASE_UART0          0x80120000
#define BASE_UART1          0x80121000
#define BASE_UART2          0x80007000

#define UART_DR                0
#define UART_RSR               1
#define UART_FR                6
#define UART_LCRH_RX           7
#define UART_ILPR              8
#define UART_IBRD              9
#define UART_FBRD              10
#define UART_LCRH_TX           11
#define UART_CR                12
#define UART_IFLS              13
#define UART_IMSC              14
#define UART_RIS               15
#define UART_MIS               16
#define UART_ICR               17

/* TIMER */

#define MTU0_BASE 0xA03C6000

#define MTU_IMSC 0
#define MTU_RIS  1
#define MTU_MIS  2
#define MTU_ICR  3

#define MTU_PT_BASE 4
#define MTU_PT_SIZE 4

#define MTU_PT_LR  0
#define MTU_PT_VAL 1
#define MTU_PT_CR  2
#define MTU_PT_BGLR 3

/* IRQ */
#define IRQ_SHPI_START 32
#define IRQ_MTU0 (IRQ_SHPI_START + 4)

#endif /* _SOC_DEFS_H_ */
