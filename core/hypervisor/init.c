#include <hw.h>
#include "hyper.h"
#include "guest_blob.h"
#include "mmu.h"
#include "hw_core_mem.h"


/*
 * Function prototypes
 */

void change_guest_mode();
void start();
void board_init();
/*Handlers */
void prefetch_abort_handler();
void data_abort_handler();
void swi_handler();
void irq_handler();
void undef_handler();

/*Init guest*/
void linux_init();
/****************************/
/*
 * Globals
 */


extern int __hyper_pt_start__;
extern uint32_t l2_index_p;

/*Pointers to start of  first and second level Page tables
 *Defined in linker script  */
uint32_t *flpt_va = (uint32_t *)(&__hyper_pt_start__);
uint32_t *slpt_va = (uint32_t *)((uint32_t)&__hyper_pt_start__ + 0x4000); //16k Page offset


extern memory_layout_entry * memory_padr_layout;


//Static VM - May change to dynamic in future
virtual_machine vm_0;
virtual_machine *curr_vm;

extern void start_();
extern uint32_t _interrupt_vector_table;

#ifdef LINUX //TODO remove ifdefs for something nicer
	extern hc_config linux_config;
#endif
#ifdef MINIMAL
	extern hc_config minimal_config;
#endif
/*****************************/

void memory_init()
{
	/*Setup heap pointer*/
	core_mem_init();

    uint32_t j;
    
    cpu_type type;
    cpu_model model;
    
    cpu_get_type(&type, &model);
    
    /* Start with simple access control
     * Only seperation between hypervisor and user address space
     *
     * Here hypervisor already runs in virtual address since boot.S, now just setup guests
     */

	memory_layout_entry *list = (memory_layout_entry*)(&memory_padr_layout);

	for(;;) {
		if(!list) break;
		/*All IO get coarse pages*/
		if(list->type == MLT_IO_RW_REG || list->type == MLT_IO_RO_REG || list->type == MLT_IO_HYP_REG)
			pt_create_coarse (flpt_va,IO_VA_ADDRESS(PAGE_TO_ADDR(list->page_start)) , PAGE_TO_ADDR(list->page_start), (list->page_count - list->page_start) << PAGE_BITS, list->type);
		else if (list->type != MLT_NONE){
			j = (list->page_start) >> 8; /*Get L1 Page index */
			for(; j < ((list->page_count) >> 8); j++ ){
				/*Creates 1:1 Mapping */
				pt_create_section(flpt_va, (j << 20) , j << 20, list->type);
			}
		}
		if(list->flags & MLF_LAST) break;
		list++;
	}

    /*map 0xffff0000 to Vector table, interrupt have been relocated to this address */
    pt_map(0xFFFF0000,(uint32_t)GET_PHYS(&_interrupt_vector_table),0x1000, MLT_USER_ROM);

    mem_mmu_tlb_invalidate_all(TRUE, TRUE);
    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    mem_cache_set_enable(TRUE);
    mem_mmu_set_domain(0x55555555); //Start with access to all domains
}

void setup_handlers()
{
    /*Direct the exception to the hypervisor handlers*/
    cpu_set_abort_handler((cpu_callback)prefetch_abort_handler, (cpu_callback)data_abort_handler);
    cpu_set_swi_handler((cpu_callback)swi_handler);
    cpu_set_undef_handler((cpu_callback)undef_handler);

    /* Start the timer and direct interrupts to hypervisor irq handler */
    timer_tick_start((cpu_callback)irq_handler);
}


void guests_init()
{
	uint32_t i, guest = 0;
    vm_0.id = 0;
    vm_0.next = &vm_0; //Only one VM

    /*Start with VM_0 as the current VM */
    curr_vm = &vm_0;
        

#ifdef LINUX
    vm_0.config = &linux_config;
    get_guest(guest++);
    linux_init();
#else
    vm_0.config = &minimal_config;
    get_guest(guest++);
    pt_create_section(flpt_va,
                      0xc0000000,
                      0x01000000 + HAL_PHYS_START,
                      MLT_USER_RAM);
#endif
#ifdef TRUSTED
    get_guest(guest++);
    pt_create_section(flpt_va,
                      0xF0100000,
                      0x00100000 + HAL_PHYS_START,
                      MLT_TRUSTED_RAM);
    curr_vm->mode_states[HC_GM_TRUSTED].ctx.sp = curr_vm->config->rpc_handlers->sp;
    curr_vm->mode_states[HC_GM_TRUSTED].ctx.psr= ARM_INTERRUPT_MASK | ARM_MODE_USER;
#endif
    
    guest = 0;

    do{
    	/*Init default values*/
        for(i = 0; i < HC_NGUESTMODES;i++){
            curr_vm->mode_states[i].mode_config = (curr_vm->config->guest_modes[i]);
            curr_vm->mode_states[i].rpc_for = MODE_NONE;
            curr_vm->mode_states[i].rpc_to  = MODE_NONE;
        }
        curr_vm->current_guest_mode = MODE_NONE;
        curr_vm->interrupted_mode = MODE_NONE;
        curr_vm->current_mode_state = 0;
        curr_vm->mode_states[HC_GM_INTERRUPT].ctx.psr= ARM_MODE_USER;
        curr_vm = curr_vm->next;

    }while(curr_vm != &vm_0);
    cpu_context_initial_set(&curr_vm->mode_states[HC_GM_KERNEL].ctx);
}



void start_guest()
{

    /*Change guest mode to KERNEL before going into guest*/
    change_guest_mode(HC_GM_KERNEL);

    /*Starting Guest*/
    printf("Branching to address: %x\n",curr_vm->config->guest_entry_point );
    start();

}

void start_()
{
    cpu_init();

    /*Setting up pagetable rules */
    memory_init();

    /* Initialize hardware */
    soc_init();
    board_init();
    /* Setting up exception handlers and starting timer*/
    setup_handlers();

    /* Initialize hypervisor guest modes and data structures
     * according to config file in guest*/
    guests_init();
    /*Test crypto*/

    printf("Hypervisor initialized\n Entering Guest\n");
    start_guest();
}
