#include <hw.h>
#include <mmu.h>
#include "soc_interrupt.h"

//Technically 64 IRQs, but far from all are used.
#define IRQ_COUNT 64

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

extern void _interrupt_vector_table();

static interrupt_registers *aic = 0;
cpu_callback interrupt_handlers[IRQ_COUNT];


static void default_handler(uint32_t r0, uint32_t r1, uint32_t r2){
   printf("DEFAULT INTERRUPT HANDLER %x:%x:%x\n", r0, r1, r2);
}

int cpu_irq_get_count(){
    return IRQ_COUNT;
}
void cpu_irq_set_enable(int number, BOOL enable){
    if(number < 0 || number >= IRQ_COUNT) return;
    
    if(enable)
        aic->iecr = (1UL << number);
    else 
        aic->idcr = (1UL << number);
    
    /* OVP fix */
    aic->iecr = 0;    
    aic->idcr = 0;    
}

void cpu_irq_set_handler(int number, cpu_callback handler){
    if(number < 0 || number >= IRQ_COUNT) return;
    
    if(!handler)
        handler = (cpu_callback)default_handler;
    
    aic->svr[number] = GET_PHYS( (uint32_t) handler);
}

void cpu_irq_acknowledge(int number){
    if(number < 0 || number >= IRQ_COUNT) return;
    aic->eoicr = (1UL << number);
}

void soc_interrupt_set_configuration(int number, int priority, 
                                     BOOL polarity,
                                     BOOL level_sensitive){
    uint32_t tmp;
    
    if(number < 0 || number >= IRQ_COUNT) return;
    
    tmp = priority & 7;
    if(polarity) tmp |= (1UL << 6);
    if(!level_sensitive) tmp |= (1UL << 5);
    
    aic->smr[number] = tmp;
    
}

void cpu_irq_get_current(){
    /* TODO */
}


void soc_interrupt_init(){
    /*Needs to be rewritten*/
#if 0
    int i;
    memspace_t * ms_expv, *ms_aic;
    
    /* allocate the relocated exception page */
    /* TODO: mark zero page ro, supervisor and remove it from available from */
    ms_expv = env_memspace_create_physical(PROC_TYPE_HYPERVISOR, "__soc_expv",
                                           GET_PHYS(_interrupt_vector_table), 
                                           0, PAGE_SIZE, TRUE);
    
    /* configure the AIC */
    ms_aic = env_map_from(PROC_TYPE_HYPERVISOR, PROC_TYPE_HYPERVISOR,
                           "__soc_aic", AIC_BASE, sizeof(aic_registers) , TRUE);
    aic = (aic_registers *)ms_aic->vadr;
    
    /* disable and clear interrupts */
    aic->idcr = -1; /* disable all */
    aic->iccr = -1; /* clear all */
    
    /* disable each individual interrupt and set the default handler */
    for(i = 0; i < cpu_irq_get_count(); i++) {
        cpu_irq_set_enable(i, FALSE);
        cpu_irq_set_handler(i, (cpu_callback)default_handler);
    }
#endif
}

