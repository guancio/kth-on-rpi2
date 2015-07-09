#ifndef _SOC_DEFS_H_
#define _SOC_DEFS_H_

//Virtual base address
#define IO_BASE 				0xF0000000
//Currently, the offset is calculated so that the virtual address space overlaps
//with that of the OMAP35xx.
//48 + B2 = FA -> Virtual peripherals base address is 0xFA000000
//             -> Offset is 0xBB000000
#define IO_OFFSET				0xBB000000
#define IO_VA_ADDRESS(x)		((x) + IO_OFFSET)

/* INTERRUPT CONTROLLER */
#define INTC_BASE 0x3F00B200
#define INTC_VIRT_BASE (IO_VA_ADDRESS(INTC_BASE))

//Number of interrupt sources: 64 plus 8
#define INTC_SOURCE_COUNT 64

/* UART DEFINITIONS */
//The base address of the UART.
#define UART_BASE 0x3F201000

#define UART_VA_BASE (IO_VA_ADDRESS(UART_BASE))

/* CLOCKS */
#define CLOCK_PER 0x3F101070

//TODO: Powersaving?
#define POWERSAVING_BASE 0xffff4000

/*
#define CLOCK_PER_FCLKEN (0 / 4)
#define CLOCK_PER_ICLKEN (0x10 / 4)
#define CLOCK_PER_CLKSTST (0x4C / 4)
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

//TODO: Added 64 to all "basic" IRQs to distinguish them from others.
#define AIC_IRQ_NUM_TIMER 0+64
#define AIC_IRQ_NUM_MAILBOX 1+64
#define AIC_IRQ_NUM_DOORBELL0 2+64
#define AIC_IRQ_NUM_DOORBELL1 3+64

#define AIC_IRQ_NUM_GPU0HALT 4+64
#define AIC_IRQ_NUM_GPU1HALT 5+64
#define AIC_IRQ_NUM_ILLEGALACCESS1 6+64
#define AIC_IRQ_NUM_ILLEGALACCESS0 7+64

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
