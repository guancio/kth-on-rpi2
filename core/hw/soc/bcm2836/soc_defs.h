#ifndef _SOC_DEFS_H_
#define _SOC_DEFS_H_

//Virtual base address
#define IO_BASE 				0xF0000000
//Currently, the offset is calculated so that the virtual address space overlaps with that of the OMAP35xx
//48 + B2 = FA -> Virtual peripherals base address is 0xFA000000
//             -> Offset is 0xBB000000
#define IO_OFFSET				0xBB000000
#define IO_VA_ADDRESS(x)		((x) + IO_OFFSET)

/* INTERRUPT CONTROLLER */
#define INTC_BASE 0x3F00B200
#define INTC_VIRT_BASE (IO_VA_ADDRESS(INTC_BASE))

//Number of interrupt sources
#define INTC_SOURCE_COUNT 64

//Below might not be needed...
/*
#define INTC_SIR_IRQ_ADR 0x10
#define INTC_SIR_FIQ_ADR 0x11
#define INTC_CONTROL     0x12

#define INTC_CONTROL_NEWIRQAGR (1 << 0)
#define INTC_CONTROL_NEWFIQAGR (1UL << 1)
#define INTC_SYSCONFIG_RESET (1UL << 1)
#define INTC_SYSCONFIG_AUTOIDLE (1UL << 0)
#define INTC_SYSSTATUS_RESET_DONE (1UL << 0)
*/

/* UART DEFINITIONS */
//The base address of the UART.
#define UART_BASE 0x3F201000

#define UART_VA_BASE (IO_VA_ADDRESS(UART_BASE))

/*
#define UART_RHR 0
#define UART_THR 0
#define UART_LSR 5

#define UART_LSR_TX_FIFO_E (1UL << 5)
#define UART_LSR_RX_FIFO_E (1UL << 0)

#define UART_SYSC_SOFTRESET (1UL << 1)

#define UART_SYSS_RESETDONE (1UL << 0)

#define UART_LCR_MODE_OP 0
#define UART_LCR_MODE_A 0x80
#define UART_LCR_MODE_B 0xBF

#define UART_LCR_LENGTH_5 ( 0UL << 0)
#define UART_LCR_LENGTH_6 ( 1UL << 0)
#define UART_LCR_LENGTH_7 ( 2UL << 0)
#define UART_LCR_LENGTH_8 ( 3UL << 0)

#define UART_LCR_STOP_1  (0UL << 2)
#define UART_LCR_STOP_15 (1UL << 2)

#define UART_LCR_PARITY_EN (1UL << 3)
#define UART_LCR_PARITY_ODD  (0UL << 4)
#define UART_LCR_PARITY_EVEN (1UL << 4)
#define UART_LCR_BREAK_EN (1UL << 6)
#define UART_LCR_DIV_EN (1UL << 7)

#define UART_MCR_DTR ( 1UL << 0)
#define UART_MCR_RTS ( 1UL << 1)

#define UART_FCR_FIFO_EN  (1UL << 0)
#define UART_FCR_RX_CLEAR (1UL << 1)
#define UART_FCR_TX_CLEAR (1UL << 2)

#define UART_MDR1_MODE_DISABLE 7
#define UART_MDR1_MODE_UART16 0

#define UART_IER_SLEEP (1UL << 4)

#define UART_EFR_ENHANCED_EN (1U << 4)
*/

/* CLOCKS */
#define CLOCK_PER 0x3F101070

//TODO: Powersaving?
#define POWERSAVING_BASE 0xffff4000

/*
#define CLOCK_PER_FCLKEN (0 / 4)
#define CLOCK_PER_ICLKEN (0x10 / 4)
#define CLOCK_PER_CLKSTST (0x4C / 4)
*/

/* GP IO */
/*
#define SMC_CONTROL 0x48002000
#define SMC_CONTROL_PAD_CONFIG_BASE 0x48002030

#define SMC_CONTROL_PAD_CONFIG_PULLENABLE (1UL << 3)
#define SMC_CONTROL_PAD_CONFIG_PULLUP (1UL << 4)
#define SMC_CONTROL_PAD_CONFIG_INPUTENABLE (1UL << 8)

#define SMC_PAD_NUMBER_UART1_TX  166
#define SMC_PAD_NUMBER_UART1_RTS 167
#define SMC_PAD_NUMBER_UART1_CTS 168
#define SMC_PAD_NUMBER_UART1_RX  169

#define SMC_PAD_NUMBER_UART3_CTS 181
#define SMC_PAD_NUMBER_UART3_RTS 182
#define SMC_PAD_NUMBER_UART3_RX  183
#define SMC_PAD_NUMBER_UART3_TX  184
*/

/* DMA */

#define DMA4_BASE 0x3F007200
//TODO: Unsure of below row
//#define DMA4_DMAC_SIZE 0x1000 		/*4K bytes*/
#define DMA4_NUM_OF_CHANNELS 16
#define DMA4_CHANNEL_STRIDE 0x100

//#define DMA4_SYSCONFIG_SOFTRESET (1UL << 1)
//#define DMA4_SYSSTATUS_RESETDONE (1UL << 0)
//TODO: DMA channel 15 is separate from the first 15

/* CCR: Channel Control Register */
/*
#define DMA4_CHANNEL_CCR_ENABLE    (1 << 7)
#define DMA4_CHANNEL_CCR_RD_ACTIVE (1 << 9)
#define DMA4_CHANNEL_CCR_WR_ACTIVE (1 << 10)

#define DMA4_CHANNEL_CCR_CONST_FILL (1 << 16)

#define DMA4_CHANNEL_CCR_SRC_AMODE_CONST   (0 << 12)
#define DMA4_CHANNEL_CCR_SRC_AMODE_POSTINC (1 << 12)
#define DMA4_CHANNEL_CCR_SRC_AMODE_SINGLE  (2 << 12)
#define DMA4_CHANNEL_CCR_SRC_AMODE_DOUBLE  (3 << 12)

#define DMA4_CHANNEL_CCR_DST_AMODE_CONST   (0 << 14)
#define DMA4_CHANNEL_CCR_DST_AMODE_POSTINC (1 << 14)
#define DMA4_CHANNEL_CCR_DST_AMODE_SINGLE  (2 << 14)
#define DMA4_CHANNEL_CCR_DST_AMODE_DOUBLE  (3 << 14)
*/
/* CSDP: Channel Source Destination Parameters */
/*
#define DMA4_CHANNEL_CSDP_DST_BURSN_EN_1   (0 << 14)
#define DMA4_CHANNEL_CSDP_DST_BURSN_EN_16  (1 << 14)
#define DMA4_CHANNEL_CSDP_DST_BURSN_EN_32  (2 << 14)
#define DMA4_CHANNEL_CSDP_DST_BURSN_EN_64  (3 << 14)

#define DMA4_CHANNEL_CSDP_SRC_BURSN_EN_1   (0 << 7)
#define DMA4_CHANNEL_CSDP_SRC_BURSN_EN_16  (1 << 7)
#define DMA4_CHANNEL_CSDP_SRC_BURSN_EN_32  (2 << 7)
#define DMA4_CHANNEL_CSDP_SRC_BURSN_EN_64  (3 << 7)

#define DMA4_CHANNEL_CSDP_DST_PACKED  (1 << 13)

#define DMA4_CHANNEL_CSDP_DATA_TYPE_8  (0 << 1)
#define DMA4_CHANNEL_CSDP_DATA_TYPE_16 (1 << 1)
#define DMA4_CHANNEL_CSDP_DATA_TYPE_32 (2 << 1)

#define DMA4_CHANNEL_CICR_BLOCK_IE (1UL << 5)
#define DMA4_CHANNEL_CICR_TRANS_ERR (1UL << 8)
#define DMA4_CHANNEL_CICR_MISALIGNED_ERR (1UL << 11)
#define DMA4_CHANNEL_CICR_SUPERVISOR_ERR (1UL << 10)
*/

/* SOC interrupt sources */
/*
#define INTC_IRQ_SDMA_0 12
#define INTC_IRQ_SDMA_1 13
#define INTC_IRQ_SDMA_2 14
#define INTC_IRQ_SDMA_3 15
#define INTC_IRQ_GPTIMERn(n)  (36 + (n))

#define INTC_IRQ_TIMER0 66
#define INTC_IRQ_TIMER1_MS 67
#define INTC_IRQ_TIMER2 68
#define INTC_IRQ_TIMER3 69
#define INTC_IRQ_TIMER4 92
#define INTC_IRQ_TIMER5 93
#define INTC_IRQ_TIMER6 94
#define INTC_IRQ_TIMER7 95

#define INTC_IRQ_EDMACOMP 12
#define INTC_IRQ_EDMAMPERR 13
#define INTC_IRQ_EDMAERRINT 14
*/


//TODO: Added 32 to all "basic" IRQs to distinguish them from others.
#define AIC_IRQ_NUM_TIMER 0+32
#define AIC_IRQ_NUM_MAILBOX 1+32
#define AIC_IRQ_NUM_DOORBELL 2+32
#define AIC_IRQ_NUM_DOORBELL 3+32

#define AIC_IRQ_NUM_GPU0HALT 4+32
#define AIC_IRQ_NUM_GPU1HALT 5+32
#define AIC_IRQ_NUM_ILLEGALACCESS1 6+32
#define AIC_IRQ_NUM_ILLEGALACCESS0 7+32

#define INTC_IRQ_AUX_INT 29
#define INTC_IRQ_I2C_SPI_SLV_INT 43
#define INTC_IRQ_PWA0 45
#define INTC_IRQ_PWA1 46

#define INTC_IRQ_SMI 48

#define INTC_IRQ_GPIO_INT0 49
#define INTC_IRQ_GPIO_INT1 50
#define INTC_IRQ_GPIO_INT2 51
#define INTC_IRQ_GPIO_INT3 52

#define INTC_IRQ_I2C_INT 53
#define INTC_IRQ_SPI_INT 54
#define INTC_IRQ_PCM_INT 55

#define INTC_IRQ_UART_INT 57

/* ENTIRELY NEW STUFF */

//The base address of the GPPUD registers.
#define GPPUD_BASE 0x3F200094

//Base address of timer
#define TIMER_BASE 0x3F00B400

//The base address of the function select registers
#define GPFSEL_BASE 0x3F200000

#endif /* _SOC_DEFS_H_ */
