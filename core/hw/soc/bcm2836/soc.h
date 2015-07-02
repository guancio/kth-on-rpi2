#ifndef _SOC_H_
#define _SOC_H_

#include "soc_clocks.h"
#include "soc_timer.h"
#include "soc_interrupt.h"
#include "soc_uart.h"

//To be replaced...
#define IO_BASE 				0xF0000000
#define IO_OFFSET				0
#define IO_VA_ADDRESS(x)		((x) + IO_OFFSET)

void soc_init();

#endif /* _SOC_H_ */
