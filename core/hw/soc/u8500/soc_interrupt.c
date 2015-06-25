
#include <hw.h>
#include <soc_defs.h>
#include <cpu_gic.h>



#define IRQ_COUNT 256

extern uint32_t * _interrupt_vector_table;
cpu_callback interrupt_handlers[IRQ_COUNT];

/* static */ return_value default_handler(uint32_t r0, uint32_t r1, uint32_t r2)
{
    printf("DIH %x:%x:%x\n", r0, r1, r2);
    return RV_OK;
}

/* 
 */
int cpu_irq_get_count()
{
    return IRQ_COUNT;
}
void cpu_irq_set_enable(int number, BOOL enable)
{
    BASE_REG gicd = (BASE_REG) GICD_BASE;
    BASE_REG gicc = (BASE_REG) GICC_BASE;    
    
    if(number < 0 || number >= IRQ_COUNT) return;
    
    if(enable) {                
        gicd[GICD_ISENABLERn + number / 32] = 1UL << (number & 31);
    } else {
        gicd[GICD_ICENABLERn + number / 32] &= ~ (1UL << (number & 31));
        
    }
}


void cpu_irq_set_handler(int number, cpu_callback handler)
{
    if(number < 0 || number >= IRQ_COUNT) return;
    
    if(!handler)
        handler = (cpu_callback)default_handler;
        
    interrupt_handlers[number] = (uint32_t) handler;
}

#if 0

void cpu_irq_acknowledge(int number)
{
    aic_registers *aic = (aic_registers *) AIC_BASE;
    if(number < 0 || number >= IRQ_COUNT) return;
    aic->eoicr = (1UL << number);
}

void soc_interrupt_set_configuration(int number, int priority, 
                                     BOOL polarity,
                                     BOOL level_sensetive)
{
    /* TODO */
}

void cpu_irq_get_current()
{
    /* TODO */
}

#endif

void soc_interrupt_init()
{
    int i;
    BASE_REG gicd = (BASE_REG) GICD_BASE;    
    BASE_REG gicc = (BASE_REG) GICC_BASE;    
        
    /* relocate the exception vector vector */
    COP_WRITE(COP_SYSTEM, COP_SYSTEM_RELOCATE_EXCEPTION_VECTOR, &_interrupt_vector_table);
    
    /* disable all interrupts for now */
    gicd[GICD_CTLR] = 0;
    gicc[GICC_CTLR] = 0;
    
    /* CPU0 gets all interrupts for now :) */
    for(i = 0; i < IRQ_COUNT / 4; i++)
        gicd[GICD_ITARGETSRn + i] = 0x010101;
    
    /* set default interrupts */
    for(i = 0; i < IRQ_COUNT; i++) {
        cpu_irq_set_enable(i, FALSE);
        cpu_irq_set_handler(i, default_handler);
    }
    
    /* enable interrupts again */
    gicd[GICD_CTLR] |= 3;
    gicc[GICC_CTLR] |= 3;

}

#ifdef DEBUG
void soc_interrupt_dump()
{
    int i, j;
    BASE_REG gicd = (BASE_REG) GICD_BASE;    
    BASE_REG gicc = (BASE_REG) GICC_BASE;        
    
    printf(" -- GICD DEBUG --\n");
    
    for(j = 0; j < 1024; j+= 32) {
        printf("%x: ", gicd + j );
        for(i = 0; i < 4; i++)  printf("%x ", gicd[i+ j]);
        printf("\n");
    }
    
    printf(" -- GICC DEBUG --\n");
    for(j = 0; j < 64; j+= 8) {
        printf("%x: ", gicc + j);
        for(i = 0; i < 4; i++)  printf("%x ", gicc[i+ j]);
        printf("\n");
    }
    
    printf(" -- GIC DEBUG --\n");
}
#endif
