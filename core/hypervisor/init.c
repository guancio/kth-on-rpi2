#include <hw.h>
#include "hyper.h"
#include "guest_blob.h"
#include "mmu.h"
#include "hw_core_mem.h"
#include "dmmu.h"



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

    uint32_t j, va_offset;
    
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
			va_offset = 0;
			if(list->type == MLT_HYPER_RAM || list->type == MLT_TRUSTED_RAM)
				va_offset = (uint32_t)HAL_OFFSET;
			for(; j < ((list->page_count) >> 8); j++ ){

				pt_create_section(flpt_va, (j << 20) - va_offset , j << 20, list->type);
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

    /* GUANCIO CHANGES */
    /* - The hypervisor must be always able to read/write the guest PTs */
    /*   we constraint that for the minimal guests, the page tables */
    /*   are between physical addresses 0x01000000 and 0x012FFFFF (that are the three megabytes of the guest) */
    /*   of memory reserved to the guest */
    /*   these address are mapped by the virtual address  0x00000000 and 0x002FFFFF */
    /*   TODO: this must be accessible only to the hypervisor */
    // this must be a loop
    uint32_t va_offset;
    for (va_offset = 0;
	 va_offset <= vm_0.config->reserved_va_for_pt_access_end - vm_0.config->reserved_va_for_pt_access_start;
	 va_offset += SECTION_SIZE) {
      uint32_t offset;
      uint32_t *pmd;
      uint32_t va = vm_0.config->reserved_va_for_pt_access_start + va_offset;
      pt_create_section(flpt_va,  va,
			vm_0.config->pa_for_pt_access_start + va_offset,
			MLT_HYPER_RAM);

      /* Invalidate the new created entries */
      offset = ((va >> MMU_L1_SECTION_SHIFT)*4);
      pmd = (uint32_t *)((uint32_t)flpt_va + offset);
      COP_WRITE(COP_SYSTEM,COP_DCACHE_INVALIDATE_MVA, pmd);
    }

    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    mem_mmu_tlb_invalidate_all(TRUE, TRUE);

    // now the master page table is ready
    // it contains
    // - the virtual mapping to the hypervisor code and data
    // - a fixed virtual mapping to the guest PT
    // - some reserved mapping that for now we ignore, e.g. IO‌REGS
    // - a 1-1 mapping to the guest memory (as defined in the board_mem.c) writable and readable by the user
    // - THIS‌ SETUP ‌MUST ‌BE ‌FIXED, SINCE ‌THE ‌GUEST ‌IS ‌NOT ‌ALLOWED ‌TO ‌WRITE ‌IN TO ‌ITS ‌WHOLE‌ MEMORY

    dmmu_entry_t * bft = (dmmu_entry_t *) DMMU_BFT_BASE_VA;

    /* - Create a copy of the master page table for the guest in the physical address: pa_initial_l1 */
    uint32_t index;
    uint32_t value;
    uint32_t *guest_pt_va;
    guest_pt_va = mmu_guest_pa_to_va(vm_0.config->pa_initial_l1, &(vm_0.config));
    for (index=0; index<4096; index++) {
    	// Hamed Changes , Creating a valid L1 according to the verified L1_create API
      value = *(flpt_va + index);
      if((value & 0b1) == 1 )
    	  bft[PA_TO_PH_BLOCK(value)].type = PAGE_INFO_TYPE_L2PT;
	  if(((value & 0xFFFF0000) == 0x81200000))
		  *(guest_pt_va + index)  = (value & 0xFFFFFBFF);
	   // END Hamed Changes
      *(guest_pt_va + index) = value;

    }
  
    /* activate the guest page table */
    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    COP_WRITE(COP_SYSTEM,COP_SYSTEM_TRANSLATION_TABLE0, vm_0.config->pa_initial_l1); // Set TTB0
    isb();
    mem_mmu_tlb_invalidate_all(TRUE, TRUE);
    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    mem_cache_set_enable(TRUE);

    // Initialize the datastructures with the type for the initial L1
    // This should be done by MMU_CREATE_L1

    bft[PA_TO_PH_BLOCK(vm_0.config->pa_initial_l1) + 0].type = PAGE_INFO_TYPE_L1PT;
    bft[PA_TO_PH_BLOCK(vm_0.config->pa_initial_l1) + 1].type = PAGE_INFO_TYPE_L1PT;
    bft[PA_TO_PH_BLOCK(vm_0.config->pa_initial_l1) + 2].type = PAGE_INFO_TYPE_L1PT;
    bft[PA_TO_PH_BLOCK(vm_0.config->pa_initial_l1) + 3].type = PAGE_INFO_TYPE_L1PT;


    // Map one section for the guest
    // This must be changed to use the MMU_APIs
    // This also works, but it is an error due to the guest 1-1 mapping
    //pt_create_section(guest_pt_pa, 0xc0000000, 0x01000000 + HAL_PHYS_START, MLT_USER_RAM);
    /*
    pt_create_section(guest_pt_va, 0xc0000000, HAL_PHYS_START + 0x01000000, MLT_USER_RAM);
    for (index=0; index<256; index++) {
      bft[PA_TO_PH_BLOCK(HAL_PHYS_START + 0x01000000) + index].refcnt = 1;
      bft[PA_TO_PH_BLOCK(HAL_PHYS_START + 0x01000000) + index].type = PAGE_INFO_TYPE_DATA;
    }
    */
    // create the attribute that allow the guest to read/write/execute
    uint32_t attrs;
    attrs = 0x12; // 0b1--10
    attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
    attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
    dmmu_map_L1_section(0xc0000000, HAL_PHYS_START + 0x01000000, attrs);

    mem_mmu_tlb_invalidate_all(TRUE, TRUE);
    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    mem_cache_set_enable(TRUE);

    /* END GUANCIO CHANGES */

#endif
#ifdef TRUSTED
    get_guest(guest++);
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
    
    /* dmmu init */
    dmmu_init();
        
    /* Initialize hypervisor guest modes and data structures
     * according to config file in guest*/
    guests_init();
    /*Test crypto*/

    printf("Hypervisor initialized\n Entering Guest\n");
    start_guest();
}
