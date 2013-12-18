
#ifndef _SOC_DEFS_H_
#define _SOC_DEFS_H_

#define CT_BASE              0x13000000	 /*  Counter/Timers */
#define IC_BASE              0x14000000	 /*  Interrupt Controller */
#define RTC_BASE             0x15000000	 /*  Real Time Clock */
#define UART0_BASE           0x16000000	 /*  UART 0 */
#define UART1_BASE           0x17000000	 /*  UART 1 */
#define KBD_BASE             0x18000000	 /*  Keyboard */
#define MOUSE_BASE           0x19000000	 /*  Mouse */

#define IO_BASE 				0xF0000000
/*Do not give this macro any address that uses bit 0,1,2, it disappears*/
#define IO_VA_ADDRESS(x)		(((x) >> 4) + IO_BASE)

/* TIMER-------------------------------------------------------- */

#define TIMER0_VA_BASE			(IO_VA_ADDRESS(CT_BASE) + 0x000)
#define TIMER1_VA_BASE 			(IO_VA_ADDRESS(CT_BASE) + 0x100)
#define TIMER2_VA_BASE 			(IO_VA_ADDRESS(CT_BASE) + 0x200)

#define TIMER_CHANNEL_COUNT 	3

//Timer Control registers
#define TIMER_LOAD 				0
#define TIMER_VALUE				1
#define TIMER_CONTROL			2
#define TIMER_INTCLR			3
#define TIMER_RIS				4
#define TIMER_MIS				5
#define TIMER_BGLOAD			6

//Timer control values
#define TIMER_CTRL_ONESHOT      (1 << 0)
#define TIMER_CTRL_32BIT        (1 << 1)
#define TIMER_CTRL_DIV1         (0 << 2)
#define TIMER_CTRL_DIV16        (1 << 2)
#define TIMER_CTRL_DIV256       (2 << 2)
#define TIMER_CTRL_IE           (1 << 5)      /* Interrupt Enable*/
#define TIMER_CTRL_PERIODIC     (1 << 6)
#define TIMER_CTRL_ENABLE      	(1 << 7)

/* PIC-------------------------------------------------------- */

#define PIC_VA_BASE				(IO_VA_ADDRESS(IC_BASE))

#define SIC_BASE				0xCA000000

#define IRQ_PIC_START 			0
#define IRQ_PIC_END 			31
#define IRQ_SIC_START			35
#define IRQ_SIC_END				46

// PIC REGISTERS
#define IRQ_STATUS				0
#define IRQ_RAWSTAT				1
#define IRQ_ENABLESET			2
#define IRQ_ENABLECLR			3
#define IRQ_SOFTSET				4
#define IRQ_SOFTCLR				5
#define FIQ_STATUS				6
#define FIQ_RAWSTAT				7
#define FIQ_ENABLESET			8
#define FIQ_ENABLECLR			9

// SIC REGISTERS
#define SIC_INT_STATUS			0
#define SIC_INT_RAWSTAT			1
#define SIC_INT_ENABLESET		2
#define SIC_INT_ENABLECLR		3
#define SIC_INT_SOFTSET			4
#define SIC_INT_SOFTCLR			5

/* Integrator PIC interrupt sources*/

#define INTSRC_IRQ_SOFTINT 		0
#define INTSRC_IRQ_UARTINT0 	1
#define INTSRC_IRQ_UARTINT1		2
#define INTSRC_IRQ_KBDINT		3
#define INTSRC_IRQ_MOUSEINT		4
#define INTSRC_IRQ_TIMERINT0	5
#define INTSRC_IRQ_TIMERINT1	6
#define INTSRC_IRQ_TIMERINT2	7
#define INTSRC_IRQ_RTCINT		8
#define INTSRC_IRQ_CLCDCINT		22
#define INTSRC_IRQ_MMCIINT0		23
#define INTSRC_IRQ_MMCIINT1		24
#define INTSRC_IRQ_AACIINT		25
#define INTSRC_IRQ_CPPLDINT		26
#define INTSRC_IRQ_ETH_INT		27
#define INTSRC_IRQ_TS_PENINT	28

/* UART-------------------------------------------------------- */

#define UART0_VA_BASE			(IO_VA_ADDRESS(UART0_BASE))
#define UART1_VA_BASE			(IO_VA_ADDRESS(UART1_BASE))


#define UART_CLK 14745600
#define BAUD_RATE 115200

/* Line control register (High) */
#define LCRH_WLEN8	0x60	/* 8 bits */
#define LCRH_FEN	0x10	/* Enable FIFO */

/* Control register */
#define CR_UARTEN	0x0001	/* UART enable */
#define CR_TXE		0x0100	/* Transmit enable */
#define CR_RXE		0x0200	/* Receive enable */

/* Flag register */
#define FR_RXFE		0x10	/* Receive FIFO empty */
#define FR_TXFF		0x20	/* Transmit FIFO full */

#define UART_DR                0
#define UART_RSR               1
#define UART_FR				   7
#define UART_ILPR			   9
#define UART_IBRD			   10
#define UART_FBRD			   11
#define UART_LCR_H			   12
#define UART_CR				   13
#define UART_IFLS			   14
#define UART_IMSC			   15
#define UART_RIS			   16
#define UART_MIS			   17
#define UART_ICR			   18
#define UART_DMACR			   19


#endif /* _SOC_DEFS_H_ */
