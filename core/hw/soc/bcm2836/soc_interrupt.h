#ifndef _SOC_INTERRUPT_H_
#define _SOC_INTERRUPT_H_

#define AIC_BASE 0x3F00B200

#define AIC_IRQ_NUM_TIMER 0
#define AIC_IRQ_NUM_MAILBOX 1
#define AIC_IRQ_NUM_DOORBELL 2
#define AIC_IRQ_NUM_DOORBELL 3

#define AIC_IRQ_NUM_GPU0HALT 4
#define AIC_IRQ_NUM_GPU1HALT 5
#define AIC_IRQ_NUM_ILLEGALACCESS1 6
#define AIC_IRQ_NUM_ILLEGALACCESS0 7

#define AIC_IRQ_NUM_UART_INT 57

typedef struct {
    uint32_t ibp; //IRQ basic pending
    uint32_t ip1; //IRQ pending 1
    uint32_t ip2; //IRQ pending 2
    uint32_t fc; //FIQ control
    uint32_t ei1; //Enable IRQs 1
    uint32_t ei2; //Enable IRQs 2
    uint32_t ebi; //Enable Basic IRQs
    uint32_t di1; //Disable IRQs 1
    uint32_t di2; //Disable IRQs 2
    uint32_t dbi; //Disable Basic IRQs
} volatile interrupt_registers;

extern void soc_interrupt_set_configuration(int number, int priority, 
                                            BOOL polarity,
                                            BOOL level_sensitive);

extern void soc_interrupt_init();


#endif /* _SOC_INTERRUPT_H_ */

