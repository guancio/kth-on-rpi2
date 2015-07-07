//The timer on the BCM2836 chip is based on, but not identical to, the ARM SP804 timer. It is described on page 196 onwards of the BCM2835 documentation. 

#ifndef _SOC_TIMER_H_
#define _SOC_TIMER_H_

//Base address of timer
#define TIMER_BASE 0x3F00B400

typedef struct {
    uint32_t tl; //Timer load register
	tv; //Timer Value register
	tc; //Timer control register
	tic; //Timter IRQ clear register
	tri; //Timer Raw IRQ register
	tmi; //Timer Masked IRQ register
	tr; //Timer Reload register
	tpd; //The timer pre-divider register (not in ARM SP804)
	fr; //Free running counter (not in ARM SP804)
} volatile timer_registers;

extern void soc_timer_init();

#endif /* _SOC_TIMER_H_ */
