#ifndef _SOC_H_
#define _SOC_H_

#include "soc_clocks.h"
#include "soc_timer.h"
#include "soc_interrupt.h"
#include "soc_uart.h"

//TODO: GPIO base?
#define IO_BASE 				0x3F200000
//TODO: The below would make this the offset from physical to virtual addresses.
#define IO_OFFSET				0
//TODO: Get virtual address? From what?
#define IO_VA_ADDRESS(x)		((x) + IO_OFFSET)

void soc_init();

#endif /* _SOC_H_ */
