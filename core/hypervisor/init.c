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

// We allocate only the blocks required to manage from
//0x01000000 + HAL_PHYS_START to
//0x012FFFFF + HAL_PHYS_START
struct page_info ph_block_state[3*256];

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


void temporary_create_hyper_section(addr_t *l1, addr_t va, addr_t pa) 
{
  uint32_t index = (va >> 20);
  uint32_t val;
  uint32_t domain, ap;

  val = pa | 0x12; // 0b1--10
  val |= MMU_AP_SUP_RW << MMU_SECTION_AP_SHIFT;
  val = (val & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT)  ;
  l1[index] = val;
  return;
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
#ifdef GUANCIO_BOOT_TEST
    /* GUANCIO CHANGES */
    /* - The hypervisor must be always able to read/write the guest PTs */
    /*   we constraint that for the minimal guests, the page tables */
    /*   are between 0x01000000 and 0x012FFFFF (that are the three megabytes of the guest) */
    /*   of memory reserved to the guest */
    /*   these address are mapped by the virtual address  0x00000000 and 0x002FFFFF */
    /*   TODO: this must be accessible only to the hypervisor */
    // this must be a loop
    temporary_create_hyper_section(flpt_va, INITIAL_PT_FIXED_MAP_VA + 0x00000000, 0x01000000 + HAL_PHYS_START);
    temporary_create_hyper_section(flpt_va, INITIAL_PT_FIXED_MAP_VA + 0x00100000, 0x01100000 + HAL_PHYS_START);
    temporary_create_hyper_section(flpt_va, INITIAL_PT_FIXED_MAP_VA + 0x00200000, 0x01200000 + HAL_PHYS_START);
    
    /* Invalidate the new created entries */
    uint32_t offset;
    uint32_t *pmd;
    offset = ((0x00000000 >> MMU_L1_SECTION_SHIFT)*4);
    pmd = (uint32_t *)((uint32_t)flpt_va + offset);
    COP_WRITE(COP_SYSTEM,COP_DCACHE_INVALIDATE_MVA, pmd);
    offset = ((0x00100000 >> MMU_L1_SECTION_SHIFT)*4);
    pmd = (uint32_t *)((uint32_t)flpt_va + offset);
    COP_WRITE(COP_SYSTEM,COP_DCACHE_INVALIDATE_MVA, pmd);
    offset = ((0x00200000 >> MMU_L1_SECTION_SHIFT)*4);
    pmd = (uint32_t *)((uint32_t)flpt_va + offset);
    COP_WRITE(COP_SYSTEM,COP_DCACHE_INVALIDATE_MVA, pmd);

    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    mem_mmu_tlb_invalidate_all(TRUE, TRUE);

    /* - Create a copy of the master page table for the guest in the physical address: 0x01200000 + HAL_PHYS_START*/
    uint32_t *guest_pt_va;
    uint32_t *guest_pt_pa;
    uint32_t index;
    uint32_t value;
    guest_pt_pa = 0x01200000 + HAL_PHYS_START;
    guest_pt_va = ((uint32_t)guest_pt_pa) - (0x01000000 + HAL_PHYS_START);
    for (index=0; index<4096; index++) {
      value = *(flpt_va + index);
      *(guest_pt_va + index) = value;
    }
    
    /* activate the guest page table */
    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    COP_WRITE(COP_SYSTEM,COP_SYSTEM_TRANSLATION_TABLE0, guest_pt_pa); // Set TTB0
    isb();
    mem_mmu_tlb_invalidate_all(TRUE, TRUE);
    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    mem_cache_set_enable(TRUE);

    vm_0.config = &minimal_config;
    get_guest(guest++);
    //pt_create_section(guest_pt_pa, 0xc0000000, 0x01000000 + HAL_PHYS_START, MLT_USER_RAM);
    pt_create_section(guest_pt_va, 0xc0000000, 0x01000000 + HAL_PHYS_START, MLT_USER_RAM);
      for (index=0; index<256; index++) {
      ph_block_state[PA_TO_PH_BLOCK(0x01000000 + HAL_PHYS_START) + index].refs = 1;
      ph_block_state[PA_TO_PH_BLOCK(0x01000000 + HAL_PHYS_START) + index].type = PAGE_INFO_TYPE_DATA;
    }

    //pt_create_section(guest_pt_pa, 0xc0100000, 0x01100000 + HAL_PHYS_START, MLT_USER_RAM);
    pt_create_section(guest_pt_va, 0xc0100000, 0x01100000 + HAL_PHYS_START, MLT_USER_RAM);
    for (index=0; index<256; index++) {
      ph_block_state[PA_TO_PH_BLOCK(0x01100000 + HAL_PHYS_START) + index].refs = 1;
      ph_block_state[PA_TO_PH_BLOCK(0x01100000 + HAL_PHYS_START) + index].type = PAGE_INFO_TYPE_DATA;
    }

    // map the third page, but only readable by the hypervisor
    temporary_create_hyper_section(guest_pt_va, 0xc0200000, 0x01200000 + HAL_PHYS_START);
    for (index=0; index<256; index++) {
      ph_block_state[PA_TO_PH_BLOCK(0x01200000 + HAL_PHYS_START) + index].refs = 0;
      ph_block_state[PA_TO_PH_BLOCK(0x01200000 + HAL_PHYS_START) + index].type = PAGE_INFO_TYPE_DATA;
    }
    ph_block_state[PA_TO_PH_BLOCK(0x01200000 + HAL_PHYS_START) + 0].type = PAGE_INFO_TYPE_L1PT;
    ph_block_state[PA_TO_PH_BLOCK(0x01200000 + HAL_PHYS_START) + 1].type = PAGE_INFO_TYPE_L1PT;
    ph_block_state[PA_TO_PH_BLOCK(0x01200000 + HAL_PHYS_START) + 2].type = PAGE_INFO_TYPE_L1PT;
    ph_block_state[PA_TO_PH_BLOCK(0x01200000 + HAL_PHYS_START) + 3].type = PAGE_INFO_TYPE_L1PT;
#else
	pt_create_section(flpt_va,
                      0xc0000000,
                      0x01000000 + HAL_PHYS_START,
                      MLT_USER_RAM);
#endif
    // test the hypercall
    //uint32_t res;
    // I can not unmap 0, since it is reserved by the hypervisor to access the guest page tables
    //res = hypercall_unmap_L1_pageTable_entry(0);
    // I can not unmap 0xf0000000, since it is reserved by the hypervisor code
    //res = hypercall_unmap_L1_pageTable_entry(0xf0000000);
    // Unmapping 0xc0300000 has no effect, since this page is unmapped
    //res = hypercall_unmap_L1_pageTable_entry(0xc0300000);
    // Unmapping 0xc0200000 is ok, since it is the page containing the active page table
    //res = hypercall_unmap_L1_pageTable_entry(0xc0200000);
    // Unmapping 0xc0100000 is ok, but the guest will not be able to write in this part of the memory
    //res = hypercall_unmap_L1_pageTable_entry(0xc0100000);
    // Unmapping 0xc0000000 is ok, but this is the page where the guest code resides
    //res = hypercall_unmap_L1_pageTable_entry(0xc0000000);


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
