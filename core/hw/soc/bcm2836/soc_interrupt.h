#ifndef _SOC_INTERRUPT_H_
#define _SOC_INTERRUPT_H_

#define AIC_BASE 0x3F000B000


#define AIC_IRQ_NUM_FIQ 0
#define AIC_IRQ_NUM_SWI 1
#define AIC_IRQ_NUM_USART0 2
#define AIC_IRQ_NUM_USART1 3

#define AIC_IRQ_NUM_TIMER0 4
#define AIC_IRQ_NUM_TIMER1 5
#define AIC_IRQ_NUM_TIMER2 6
#define AIC_IRQ_NUM_WD 7
#define AIC_IRQ_NUM_PIO 8

#define AIC_IRQ_NUM_IRQ0 16
#define AIC_IRQ_NUM_IRQ1 17
#define AIC_IRQ_NUM_IRQ2 18

//TODO: Correct entries
typedef struct {
    uint32_t smr[32];
    uint32_t svr[32];
    uint32_t ivr;
    uint32_t fvr;
    uint32_t isr;
    uint32_t ipr;
    uint32_t imr;
    uint32_t cisr;
    uint32_t unused2[2];
    uint32_t iecr;
    uint32_t idcr;
    uint32_t iccr;
    uint32_t iscr;
    uint32_t eoicr;
    uint32_t spu;    
} volatile aic_registers;

extern void soc_interrupt_set_configuration(int number, int priority, 
                                            BOOL polarity,
                                            BOOL level_sensitive);

extern void soc_interrupt_init();


#endif /* _SOC_INTERRUPT_H_ */

