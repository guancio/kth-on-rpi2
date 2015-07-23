#ifndef _SOC_DEFS_H_
#define _SOC_DEFS_H_

//Virtual base address
#define IO_BASE 				0xF0000000
//The offset was calculated so that the virtual address space overlaps
//with that of the OMAP35xx. Note: This has been proven wrong, and gives exceptions in
//soc_gpio_init.
//48 + B2 = FA -> Virtual peripherals base address is 0xFA000000
//             -> Offset is 0xBB000000 (BB + 3F = FA)
//TODO: Try 0xB2000000 instead... 
#define IO_OFFSET				0xBB000000
#define IO_VA_ADDRESS(x)		((x) + IO_OFFSET)

/* GPIO */

//The base address of the function select registers
#define GPFSEL_BASE 0x3F200000

//The base address of the GPPUD registers.
#define GPPUD_BASE 0x3F200094

/* Interrupt Controller */
#define INTC_BASE 0x3F00B200
#define INTC_VIRT_BASE (IO_VA_ADDRESS(INTC_BASE))
#define INTC_VIRT_IRQ_BASIC_PENDING INTC_VIRT_BASE
#define INTC_VIRT_IRQ_PENDING_1 (IO_VA_ADDRESS(0x3F00B204))
#define INTC_VIRT_IRQ_PENDING_2 (IO_VA_ADDRESS(0x3F00B208))

//Number of interrupt sources: 64 plus 8
#define INTC_SOURCE_COUNT 64+8

/* SOC interrupt sources */

//TODO: Added 64 to all "basic" IRQs to distinguish them from others.
#define INTC_IRQ_NUM_TIMER 0+64
#define INTC_IRQ_NUM_MAILBOX 1+64
#define INTC_IRQ_NUM_DOORBELL0 2+64
#define INTC_IRQ_NUM_DOORBELL1 3+64
#define INTC_IRQ_NUM_GPU0HALT 4+64
#define INTC_IRQ_NUM_GPU1HALT 5+64
#define INTC_IRQ_NUM_ILLEGALACCESS1 6+64
#define INTC_IRQ_NUM_ILLEGALACCESS0 7+64

#define INTC_IRQ_NUM_AUX 29
#define INTC_IRQ_NUM_I2C_SPI_SLV 43
#define INTC_IRQ_NUM_PWA0 45
#define INTC_IRQ_NUM_PWA1 46
#define INTC_IRQ_NUM_SMI 48
#define INTC_IRQ_NUM_GPIO0 49
#define INTC_IRQ_NUM_GPIO1 50
#define INTC_IRQ_NUM_GPIO2 51
#define INTC_IRQ_NUM_GPIO3 52
#define INTC_IRQ_NUM_I2C 53
#define INTC_IRQ_NUM_SPI 54
#define INTC_IRQ_NUM_PCM 55
#define INTC_IRQ_NUM_UART 57

/* Timer */

//Base address of timer
#define TIMER_BASE 0x3F00B400
#define TIMER_TIC 0x3F00B40C

#define TIMER_VIRT_BASE (IO_VA_ADDRESS(TIMER_BASE))
#define TIMER_VIRT_TIC (IO_VA_ADDRESS(TIMER_TIC))

/* UART */
//The base address of the UART.
#define UART_BASE 0x3F201000
#define UART_ICR 0x3F201044

#define UART_VIRT_BASE (IO_VA_ADDRESS(UART_BASE))
#define UART_VIRT_ICR (IO_VA_ADDRESS(UART_ICR))

/* Clocks */ //TODO
//#define CLOCK_PER 0x3F101070

/* DMA */ //TODO

//#define DMA4_BASE 0x3F007200
//TODO: Unsure of below row
//#define DMA4_DMAC_SIZE 0x1000 		/*4K bytes*/
//#define DMA4_NUM_OF_CHANNELS 16
//#define DMA4_CHANNEL_STRIDE 0x100

//TODO: DMA channel 15 is separate from the first 15

#endif /* _SOC_DEFS_H_ */
