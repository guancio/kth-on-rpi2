#ifndef _SOC_GPPUD_H_
#define _SOC_GPPUD_H_

//The base address of the GPPUD registers.
#define GPPUD_BASE 0x3F200094

typedef struct {
    uint32_t gppud; //GPIO pull-up/pull-down register
	uint32_t gppudclk0; //GPIO pull-up/pull-down Clock register
	uint32_t gppudclk1; //GPIO pull-up/pull-down Clock register
} volatile gppud_registers;

extern void soc_gppud_init();

#endif /* _SOC_GPPUD_H_ */
