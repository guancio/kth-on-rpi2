#ifndef _SOC_CLOCKS_H_
#define  _SOC_CLOCKS_H_

//What type of address is this?
#define POWERSAVING_BASE 0xffff4000

typedef struct {
	uint32_t cr;
	uint32_t pcer;
	uint32_t pcdr;
	uint32_t pcsr;
} volatile powersaving_registers;

extern void soc_clocks_init();

#endif /* _SOC_CLOCKS_H_ */
