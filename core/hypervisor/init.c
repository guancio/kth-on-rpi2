#include <hw.h>
#include "hyper.h"
#include "guest_blob.h"
#include "mmu.h"
#include "hw_core_mem.h"
#include "dmmu.h"
//TODO: Note: Added these to avoid warnings.
extern void dmmu_init();
extern uint32_t dmmu_map_L1_section(addr_t va, addr_t sec_base_add, uint32_t attrs);
extern void debug_inf(void); //TODO. Remove after debugging

//#define DEBUG_PG_CONTENT
//#define DEBUG_L1_PG_TYPE
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

/* Pointers to start of first and second level page tables.
 * Defined in linker script.  */
uint32_t *flpt_va = (uint32_t *)(&__hyper_pt_start__);
uint32_t *slpt_va = (uint32_t *)((uint32_t)&__hyper_pt_start__ + 0x4000); //16 KiB Page offset

extern memory_layout_entry * memory_padr_layout;


//Static VM. TODO: Should change to dynamic in future
virtual_machine vm_0;
virtual_machine *curr_vm;

extern void start_();
extern uint32_t _interrupt_vector_table;

#ifdef LINUX //TODO: Remove ifdefs for something nicer
	extern hc_config linux_config;
#endif
#ifdef MINIMAL
	extern hc_config minimal_config;
#endif
#ifdef DTEST
	extern hc_config minimal_config;
#endif

/*****************************/
/* DEBUG */
void dump_mmu(addr_t adr)
{
    uint32_t *t = (uint32_t *) adr;    
    int i;
    
    printf("  (L1 is at %x)\n", adr);    
    for(i = 0; i < 4096; i++) {
        uint32_t x = t[i];
        switch(x & 3) {
        case 2:
            printf("SEC %x -> %x : %x DOM=%d C=%d B=%d AP=%d\n",
                   i << 20, x, (x & 0xFFF00000), (x >> 5) & 15, 
                   (x >> 3) & 1, (x >> 2) & 1, (x >> 10) & 3);
            break;
        case 1:
            printf("COR %x -> %x : %x DOM=%d C=%d B=%d\n",
                   i << 20, x, (x & 0xFFFFF000),
                   (x >> 5) & 15, (x >> 3) & 1, (x >> 2) & 1);            
            break;            
        case 3:
            printf("FIN %x -> %x\n", i << 20, x);
            break;            
        }
    }
    printf("\n");
}

/*****************************/

void memory_commit()
{
    mem_mmu_tlb_invalidate_all(TRUE, TRUE);
    mem_cache_invalidate(TRUE, TRUE, TRUE); //Instruction, data, writeback
}

void memory_init()
{
	/* Setup heap pointer. */
	core_mem_init();

    uint32_t j; //TODO: Note: Removed unused variable va_offset here.
    
    cpu_type type;
    cpu_model model;
    cpu_get_type(&type, &model);
    
    /* Start with simple access control
     * Only separation between hypervisor and user address space.
     *
     * Here, hypervisor already runs in virtual address since boot.S, now just setup guests.
     */

    // We clear the memory that contains the L2s that can be created in the 32KB of slpt_va
    memset(slpt_va, 0, 0x8000);
	memory_layout_entry *list = (memory_layout_entry*)(&memory_padr_layout);
	for(;;) {
		if(!list) break;
        switch(list->type) {
        case MLT_IO_RW_REG:
        case MLT_IO_RO_REG:
        case MLT_IO_HYP_REG:
            /*All IO get coarse pages*/
        	pt_create_coarse (flpt_va, IO_VA_ADDRESS(PAGE_TO_ADDR(list->page_start)), PAGE_TO_ADDR(list->page_start), (list->page_count - list->page_start) << PAGE_BITS, list->type);
            break;
        case MLT_USER_RAM:
            /* do this later */
            break;
        case MLT_HYPER_RAM:
        case MLT_TRUSTED_RAM:
            /* own memory */
            j = (list->page_start) >> 8; /* Get L1 Page index */
            for(; j < ((list->page_count) >> 8); j++ ){
                pt_create_section(flpt_va, (j << 20) - HAL_OFFSET, j << 20, list->type);
            }
            break;
        case MLT_NONE:
            break;
        }
		if(list->flags & MLF_LAST) break;
		list++;
	}
    /*map 0xffff0000 to Vector table, interrupt have been relocated to this address */
    pt_map(0xFFFF0000, (uint32_t)GET_PHYS(&_interrupt_vector_table),0x1000, MLT_USER_ROM);
    memory_commit();
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

//Initialize guests. There is a separate function in init_slave.c which
//initializes guests on more than one processor.
void guests_init()
{
    uint32_t i, guest = 0;
    vm_0.id = 0;
    vm_0.next = &vm_0; //Only one VM

    /*Start with VM_0 as the current VM */
    curr_vm = &vm_0;
  

    //printf("HV pagetable before guests initialization:\n"); //DEBUG
	//dump_mmu(flpt_va); //DEBUG
  
    
    /* Show guest information */
    printf("We have %d guests in physical memory area %x %x\n", 
        guests_db.count, guests_db.pstart, guests_db.pend);

    for(i = 0; i < guests_db.count; i++) {
        printf("Guest_%d: PA=%x+%x VA=%x FWSIZE=%x\n",
            i,
            //Physical starting address of the guest.
            guests_db.guests[i].pstart,
            //Size in bytes of the guest.
            guests_db.guests[i].psize,
            //Virtual starting address of the 1-to-1 mapping.
            guests_db.guests[i].vstart,
            //Size (in bytes) of the binary that has been copied.
            guests_db.guests[i].fwsize);            
    }


#ifdef LINUX
    vm_0.config = &linux_config;
    vm_0.config->firmware = get_guest(guest++);
    curr_vm->config->pa_initial_l2_offset += curr_vm->config->firmware->psize - 0x700000;
//  linux_init();

#else
    vm_0.config = &minimal_config;
    vm_0.config->firmware = get_guest(1 + guest++);
#endif
    addr_t guest_vstart = curr_vm->config->firmware->vstart;
    addr_t guest_pstart = curr_vm->config->firmware->pstart;
    addr_t guest_psize =  curr_vm->config->firmware->psize;
    /* KTH CHANGES 
     * The hypervisor must always be able to read from/write to the guest page
	 * tables. For now, the guest page tables can be written into the guest
	 * memory anywhere. In the future we probably need more master page tables,
	 * one for each guest that uses the memory management unit, so that the
	 * virtual reserved addresses can be different. 

	 * We place the constraint that for the minimal guests, the page tables 
     * are between physical addresses 0x01000000 and 0x014FFFFF (those are the
	 * five MiBs of the guest) of memory reserved to the guest. These
	 * addresses are mapped by the virtual addresses 0x00000000 to 0x004FFFFF.
     * TODO: This memory sub-space must be accessible only to the hypervisor */

    // this must be a loop
    uint32_t va_offset;
    for (va_offset = 0;
        va_offset + SECTION_SIZE <= guest_psize + SECTION_SIZE; /*+ 1 MiB at end for L1PT*/
        va_offset += SECTION_SIZE) {
          uint32_t offset, pmd;
          uint32_t va = vm_0.config->reserved_va_for_pt_access_start + va_offset;
          uint32_t pa = guest_pstart + va_offset;
          pt_create_section(flpt_va, va, pa, MLT_HYPER_RAM);

          /* Invalidate the newly created entries */
          offset = ((va >> MMU_L1_SECTION_SHIFT)*4);
          pmd = (uint32_t *)((uint32_t)flpt_va + offset);
          COP_WRITE(COP_SYSTEM,COP_DCACHE_INVALIDATE_MVA, pmd);
    }

    memory_commit();

    //printf("HV pagetable after guests initialization:\n"); //DEBUG
	//dump_mmu(flpt_va); //DEBUG

    // We pin the L2s that can be created in the 32KB area of slpt_va

    dmmu_entry_t * bft = (dmmu_entry_t *) DMMU_BFT_BASE_VA;
    for (i=0; i*4096<0x8000; i++) {
        bft[PA_TO_PH_BLOCK((uint32_t)GET_PHYS(slpt_va) + i*4096)].type = PAGE_INFO_TYPE_L2PT;
        bft[PA_TO_PH_BLOCK((uint32_t)GET_PHYS(slpt_va) + i*4096)].refcnt = 1;
    }

	/* At this point we are finished initializing the master page table, and can
	 * start initializing the first guest page table. 

	 * The master page table now contains
	 * 1) The virtual mapping to the hypervisor code and data.
	 * 2) A fixed virtual mapping to the guest PT.
	 * 3) Some reserved mapping that we ignore for now, e.g. IO‌REGS.
	 * 4) A 1-1 mapping to the guest memory (as defined in board_mem.c) writable
	 * 	  and readable by the user.
     * TODO: THIS‌ SETUP ‌MUST ‌BE ‌FIXED, SINCE ‌THE ‌GUEST ‌IS ‌NOT ‌ALLOWED ‌TO ‌WRITE 
	 * INTO ITS ‌WHOLE‌ MEMORY */

    /* - Create a copy of the master page table for the guest in the physical address: pa_initial_l1 */
    uint32_t *guest_pt_va;
    addr_t guest_pt_pa;
    guest_pt_pa = guest_pstart + vm_0.config->pa_initial_l1_offset;
    guest_pt_va = mmu_guest_pa_to_va(guest_pt_pa, vm_0.config);
    printf("COPY %x %x\n", guest_pt_va, flpt_va);
    memcpy(guest_pt_va, flpt_va, 1024 * 16);

    //printf("vm_0 pagetable:\n"); //DEBUG    
	//dump_mmu(guest_pt_va); //DEBUG
    
    /* activate the guest page table */
    memory_commit();
    COP_WRITE(COP_SYSTEM,COP_SYSTEM_TRANSLATION_TABLE0, guest_pt_pa); // Set TTB0
    isb();
    memory_commit();

   // Calling the create_L1_pt API to check the correctness of the L1 content and to change the page table type to 1
    uint32_t res = dmmu_create_L1_pt(guest_pt_pa);
    if (res != SUCCESS_MMU) {
      printf("XXXX We failed to create the initial PT with error %d XXXX\n", res);
      while (1) {
      }
    }

#ifdef DEBUG_L1_PG_TYPE
    uint32_t index;
    for(index=0; index < 4; index++)
      printf("Initial L1 page table's page type:%x \n", bft[PA_TO_PH_BLOCK(guest_pt_pa) + index].type);
#endif
    // Initialize the datastructures with the type for the initial L1
    // create the attribute that allow the guest to read/write/execute
    uint32_t attrs;

#ifdef LINUX
    /*Maps PA-PA for boot and VA-PA for kernel
     * First MB of pa mapped as L2PT with page 1-7 RO (private L2PT and master page)*/
    linux_init_dmmu();

#else
    attrs = 0x12; // 0b1--10
    attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
    attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
    // As default the guest has a 1-to-1 mapping to all its memory
    uint32_t offset;
    for (offset = 0;
    	 offset + SECTION_SIZE <= guest_psize;
    	 offset += SECTION_SIZE) {
	printf("-- creating initial mapping of %x to %x\n", guest_vstart+offset,guest_pstart+offset);
    	res = dmmu_map_L1_section(guest_vstart+offset,guest_pstart+offset, attrs);
	printf("-- result %d\n", res);
    }

#endif
    //printf("vm_0 pagetable after initialization:\n"); //DEBUG
	//dump_mmu(guest_pt_va); //DEBUG

    mem_mmu_tlb_invalidate_all(TRUE, TRUE);
    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    mem_cache_set_enable(TRUE);


#ifdef DEBUG_PG_CONTENT
    for (index=0; index<4096; index++) {
    	if(*(guest_pt_va + index) != 0x0)
         printf("add %x %x \n", index , *(guest_pt_va + index)); //(flpt_va + index)
    }
#endif
    /* END GUANCIO CHANGES */
    /* END KTH CHANGES */

#ifdef TRUSTED
    get_guest(guest++);
    curr_vm->mode_states[HC_GM_TRUSTED].ctx.sp = curr_vm->config->rpc_handlers->sp;
    curr_vm->mode_states[HC_GM_TRUSTED].ctx.psr= ARM_INTERRUPT_MASK | ARM_MODE_USER;
#endif
    
    guest = 0;

    // Init the context with the physical addresses
    do{
    	/* Init default values */
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
        
        // let guest know where it is located
        curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[3] =
              curr_vm->config->firmware->pstart;              
        curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[4] =
              curr_vm->config->firmware->vstart;
        // initial page table location
    }while(curr_vm != &vm_0);
    
    memory_commit();
    cpu_context_initial_set(&curr_vm->mode_states[HC_GM_KERNEL].ctx);

}



void start_guest()
{
    /* Change guest mode to KERNEL before going into guest. */
    change_guest_mode(HC_GM_KERNEL);

    /* Start guest! */
    start();
}

//Contains a function call to an assembly label above an infinite loop.
void debug_temp(){
	debug_inf_loop();
}

void start_()
{
	/* Flush and enable the cache, among other things. Defined in
	 * core/hw/cpu/family/model/cpu_init.c. */
    cpu_init();

    /* Set up pagetable rules - defined further up in this file. */
    memory_init();

    /* Initialize hardware - defined in core/hw/soc/platform and
	 * core/hw/board/platform, respectively. */
    soc_init();
    board_init();

    /* Set up exception handlers and starting timer - defined further up in this
	 * file. */
    setup_handlers();

    /* DMMU initialization - defined in dmmu.c. */
    dmmu_init();

    /* Initialize hypervisor guest modes and data structures
     * according to config file in guest - defined further up in this file. */
	//TODO: We should write a new version of guests_init which initializes
	//everything we need in order to have guests on several cores. This should
	//then be run here, by the main core. 
    guests_init();

    printf("Hypervisor initialized.\n Entering Guest...\n");
	
	//DEBUG: Try to wake some other core up here...
	typedef void (*fn)(void);
	//0x4000009C is be the mailbox of Core 1 -> VA is 0xFB00009C
	*(volatile fn *)(0xFB00009C) = GET_PHYS(debug_temp); //Try to move Core 1 to an
	//infinite loop. debug_temp is defined above. We use physical address
	//because we have not yet set up the MMU of the new core.
	
	//Move along, main core...
    start_guest();
}
