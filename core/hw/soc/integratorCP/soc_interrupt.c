
#include "types.h"

#include "soc_defs.h"

cpu_callback irq_function_table[IRQ_SIC_END] __attribute__ ((aligned (32)));

static void default_handler(uint32_t r0, uint32_t r1, uint32_t r2)
{
   printf("DEFAULT INTERRUPT HANDLER %x:%x:%x\n", r0, r1, r2);
}

/* 
 */

void cpu_irq_set_enable(int number, BOOL enable)
{
	BASE_REG pic = (BASE_REG) (PIC_VA_BASE);
    if(number < 0 || number >= IRQ_SIC_END) return;
    
    if(enable){
    	pic[IRQ_ENABLESET] = (1UL << number);
    }
    else {
    	pic[IRQ_ENABLECLR] = (1UL << number);
    }

}
void cpu_irq_set_handler(int number, cpu_callback handler)
{

    if(number < 0 || number >= IRQ_SIC_END) return;
    
    if(!handler)
        handler = (cpu_callback)default_handler;
    
    irq_function_table[number] = handler;
}


void cpu_irq_get_current()
{
    /* TODO */
}


void soc_interrupt_init()
{
    int i;
    BASE_REG pic = (BASE_REG) (PIC_VA_BASE);
    pic[IRQ_ENABLECLR] = 0xffffffff;
    pic[FIQ_ENABLECLR] = 0xffffffff;

    /* disable each individual interrupt and set the default handler */
    for (i = IRQ_PIC_START; i <= IRQ_PIC_END; i ++) {
    	if (i == 11)
    		i = 22;
    	if(i ==29)
    		break;
    	cpu_irq_set_enable(i, FALSE);
    	cpu_irq_set_handler(i, default_handler);
    }

    
    for (i = IRQ_SIC_START; i <= IRQ_SIC_END; i++) {
    	cpu_irq_set_enable(i, FALSE);
    	cpu_irq_set_handler(i, default_handler);

    }
    printf("Initialisation of SOC interrupt finished\n");

    /* TODO: set relocated interrupt base */
}

