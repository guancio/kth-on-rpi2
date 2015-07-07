#include <hw.h>
#include <mmu.h>

//Interrupts are mostly enabled/disabled by the interrupt controller (page 109 in BCM2835 documentation).
//Please note that messing with the wrong interrupts can adversely impact the behaviour of the GPU.

//Interrupt masks for the UART are set/cleared at the UART IMSC register.
//Interrupt masks for the SPI/BSC slave are set/cleared at the SPI/BSC slave IMSC register.
//The Timer control register has a bit which enables/disables timer interrupt.



//Technically 64 IRQs and 8 "basic" IRQs, but far from all are used.
#define IRQ_COUNT 72

typedef struct {
    uint32_t ibp; //IRQ basic pending
    uint32_t ip1; //IRQ pending 1
    uint32_t ip2; //IRQ pending 2
    uint32_t fc; //FIQ control
	uint32_t enable_irq[3];
    //uint32_t ei1; //Enable IRQs 1
    //uint32_t ei2; //Enable IRQs 2
    //uint32_t ebi; //Enable Basic IRQs
	uint32_t disable_irq[3];
    //uint32_t di1; //Disable IRQs 1
    //uint32_t di2; //Disable IRQs 2
    //uint32_t dbi; //Disable Basic IRQs
} volatile interrupt_registers;

extern uint32_t * _interrupt_vector_table;
//TODO: Why 128?
cpu_callback irq_function_table[128] __attribute__ ((aligned (32)));


static interrupt_registers *ireg = 0;
static cpu_callback interrupt_handler = 0;

static return_value interrupt_handler_stub(uint32_t irq, uint32_t r1, uint32_t r2 ){

    if(interrupt_handler){
    	interrupt_handler(irq, r1, r2);
	}

    return RV_OK;
}

/* static */ return_value default_handler(uint32_t r0, uint32_t r1, uint32_t r2){
	printf("DEFAULT INTERRUPT HANDLER %x:%x:%x\n", r0, r1, r2);
	return RV_OK;
}

//Returns the number of different interrupt requests.
int cpu_irq_get_count(){
    return IRQ_COUNT;
}

//Sets or clears interrupt mask for a particular IRQ.
void cpu_irq_set_enable(int number, BOOL enable){
	uint32_t register_a, register_b, register_c;

	//Check for invalid IRQ number.
    if(number < 0 || number >= IRQ_COUNT){
		return;
    }

	//Check which registry entry the IRQ number belongs to.
	register_a = number / 32;
	//Check which bit the IRQ number belongs to.
	register_b = number % 32;

    //Set or clear interrupt mask for this IRQ.
	//On the BCM2836, this means enabling/disabling the IRQ.
    if(enable) {
		//Set bit [register_b] to 1 and keep others as they are.
        ireg->enable_irq[register_a] |= 1 << register_b;
    } else {
		//Set bit [register_b] to 1 and keep others as they are.
		//(in effect, this actually clears the enable bit...)
		ireg->disable_irq[register_a] |= 1 << register_b;    
    }      
}

void cpu_irq_set_handler(int number, cpu_callback handler){
    //Check for invalid IRQ number.
    if(number < 0 || number >= IRQ_COUNT){
		return;
    }
    
	//If handler is not yet defined, use the default handler.
    if(!handler){
        handler = interrupt_handler_stub;
    }

    //Set handler
    irq_function_table[n] = handler; 
}

cpu_callback irq_handler();

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


    int i; //Loop index
    interrupt_handler = (cpu_callback)irq_handler;
    intc = (intc_registers *)IO_VA_ADDRESS(INTC_BASE);

    //TODO: No idea what this does.
    intc->intc_sysconfig = INTC_SYSCONFIG_RESET;
    while(!(intc->intc_sysstatus & INTC_SYSSTATUS_RESET_DONE)){
    	//Do nothing
    }
    //Turn off all interrupts for now.
	for(i = 0; i < (int)(IRQ_COUNT/32.0 < 0 ? (IRQ_COUNT/32.0 - 0.5) : (IRQ_COUNT/32.0 + 0.5)); ++i){
		disable_irq[i] = 0xFFFFFFFF;
	}
    for(i = 0; i < IRQ_COUNT; ++i) {
        //cpu_irq_set_enable(i, FALSE); <-- added loop above which is probably faster
        cpu_irq_set_handler(i, default_handler);
    }
	//I think I have identified IRQ 74 as the UART3 of the Beagleboard.
	//IRQ 54 is UART on the BCM2836.
    cpu_irq_set_enable(57, TRUE);
    
	//TODO: No idea what this does.
    intc->intc_control = INTC_CONTROL_NEWIRQAGR;
}

