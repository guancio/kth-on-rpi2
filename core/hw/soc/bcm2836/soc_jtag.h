//Since the Raspberry Pi 2 Model B does not have any JTAG pins enabled by default, we should/must enable them at this stage, or maybe even earlier if that is possible.

#ifndef _SOC_JTAG_H_
#define _SOC_JTAG_H_

//The base address of the function select registers
#define GPFSEL_BASE 0x3F200000

typedef struct {
    uint32_t gpfsel0;
    uint32_t gpfsel1;
    uint32_t gpfsel2;
    uint32_t gpfsel3;
    uint32_t gpfsel4;
    uint32_t gpfsel5;
} volatile function_select_registers;

extern void soc_jtag_init();

#endif /* _SOC_JTAG_H_ */
