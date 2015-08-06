#include <hw.h>
#include "hyper.h"
#include "guest_blob.h"
#include "mmu.h"
#include "hw_core_mem.h"
#include "dmmu.h"

extern int __hyper_pt_start__;
extern uint32_t l2_index_p;

extern memory_layout_entry * memory_padr_layout;
extern hc_config minimal_config;

extern uint32_t *flpt_va;
extern uint32_t *slpt_va; //16 KiB Page offset

//We will need one virtual machine for each core.
//TODO: Why not use an array?
virtual_machine vm_0;
virtual_machine vm_1;
virtual_machine vm_2;
virtual_machine vm_3;
virtual_machine vms[4];
virtual_machine *curr_vm;

//Add what you need here in terms of external functions and imports to be able
//to do what you want.

//TODO: This is a proof-of-concept guest initialization that is featured in the 
//code mutually exclusively with the ordinary guest_init().
//For example, all Linux stuff is cut out from this version.
//Let us assume we want to run four instances of some minimal-type guest, one
//on each core.
void guests_init_multicore(){
    uint32_t i, guest = 0;

	//Set the ID of the virtual machines.
    vm_0.id = 0;
	vm_1.id = 1;
	vm_2.id = 2;
	vm_3.id = 3;

	//The virtual machines are stored in some sort of linked list.
    vm_0.next = &vm_1;
	vm_1.next = &vm_2;
	vm_2.next = &vm_3;
	vm_3.next = &vm_0;

    //printf("HV pagetable before guests initialization:\n"); //DEBUG
	//dump_mmu(flpt_va); //DEBUG
    
    /* Show guest information. Note that this displays information about all
	 * guests and not just the ones for this particular core. */
    printf("We have %d guests in physical memory area %x %x\n", 
        guests_db.count, guests_db.pstart, guests_db.pend);

	/* Output some vital debug information about the guests. */
    for(i = 0; i < guests_db.count; i++) {
        printf("Guest_%d: \n    Physical starting address: %x+%x\n    Physical ending address: %x\n    Occupied physical space in bytes: %x\n    Virtual starting address: %x\n    Size of binary: %x\n",
            i,
            //Physical starting address of the guest.
            guests_db.guests[i].pstart,
            //Physical ending address of the guest.
            guests_db.guests[i].pstart + guests_db.guests[i].psize,
			//Physical size of the guest.
			guests_db.guests[i].psize,
            //Virtual starting address of the guest.
            guests_db.guests[i].vstart,
            //Size (in bytes) of the binary that has been copied.
            guests_db.guests[i].fwsize);            
    }

    /* We start with vm_0 as the current virtual machine. */
    curr_vm = &vm_0;

	//The configuration of the guest contains location of the always cacheable
	//region inside the guest, among other things. This can be found inside the
	//sub-directory guest_config.
    vm_0.config = &minimal_config;

	//Information like virtual and physical starting address is fetched from
	//guest ID (be get_guest) and stored in vm_id.config->firmware.
    vm_0.config->firmware = get_guest(1 + guest++); //Guest incremented by one.

	//TODO: Why do we store these values in temporary variables???
	//TODO: Note: later on, we can't use the same vstart and pstart for the other guests.
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
     * TODO: This memory sub-space must be accessible only to the hypervisor. */

    //TODO: Note: We want to do this for every guest. For each loop, we want
	//va_offset to start from a number which is guest_psize higher than the
	//previous.
    uint32_t va_offset;
    for (va_offset = 0;
		//TODO: Mathematically speaking, there is no reason to add section_size
		//on both sides on the below row. Is this a bug or pedagogic code in some way?
        va_offset + SECTION_SIZE <= guest_psize + SECTION_SIZE; /* +1 MiB at end for L1PT */
        va_offset += SECTION_SIZE){
		uint32_t offset, pmd;
		uint32_t va = vm_0.config->reserved_va_for_pt_access_start + va_offset;
		uint32_t pa = guest_pstart + va_offset;
		pt_create_section(flpt_va, va, pa, MLT_HYPER_RAM);

		/* Invalidate the newly created entries. */
		offset = ((va >> MMU_L1_SECTION_SHIFT)*4);
		pmd = (uint32_t *)((uint32_t)flpt_va + offset);
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, pmd);
    }

    memory_commit();

    //printf("HV pagetable after guests initialization:\n"); //DEBUG
	//dump_mmu(flpt_va); //DEBUG

    //We pin the L2s that can be created in the 32 KiB area of slpt_va.
	//TODO: Presumably we need to do this once for each guest. We might want i
	//to start from different numbers depending on the index of the guest.
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

    //Create a copy of the master page table for the guest in the physical
	//address pa_initial_l1.
	//TODO: This needs to be done once for each guest, with different addresses.
    uint32_t *guest_pt_va;
    addr_t guest_pt_pa;
    guest_pt_pa = guest_pstart + vm_0.config->pa_initial_l1_offset;
    guest_pt_va = mmu_guest_pa_to_va(guest_pt_pa, vm_0.config);
    printf("COPY %x %x\n", guest_pt_va, flpt_va);
    memcpy(guest_pt_va, flpt_va, 1024 * 16);

    //printf("vm_0 pagetable:\n"); //DEBUG    
	//dump_mmu(guest_pt_va); //DEBUG
    
    /* Activate the guest page table. */
	//TODO: Also needs to be done once for each guest.
    memory_commit();
    COP_WRITE(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, guest_pt_pa); //Set TTB0
    isb();
    memory_commit();

   	//Calling the create_L1_pt API to check the correctness of the L1 content and
	//to change the page table type to 1.
	//TODO: Also needs to be done once for each guest.
    uint32_t res = dmmu_create_L1_pt(guest_pt_pa);
    if (res != SUCCESS_MMU){
		printf("Error: Failed to create the initial PT with error code %d.\n",
			res);
		while (1) {

		}
    }

#ifdef DEBUG_L1_PG_TYPE
    uint32_t index;
    for(index=0; index < 4; index++){
		printf("Initial L1 page table's page type:%x \n",
			bft[PA_TO_PH_BLOCK(guest_pt_pa) + index].type);
	}
#endif

    //Initialize the datastructures with the type for the initial L1.
    //Create the attribute that allow the guest to read/write/execute.
    uint32_t attrs;
    attrs = 0x12; // 0b1--10
    attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
    attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

    //As default the guest has a 1-to-1 mapping to all its memory
	//TODO: This loop needs to be done for all the guests.
    uint32_t offset;
    for (offset = 0;
		//TODO: This loop really does leave 1 MiB at end, unlike the loop above.
		offset + SECTION_SIZE <= guest_psize;
		offset += SECTION_SIZE){
		printf("   Creating initial mapping of %x to %x...\n",
			guest_vstart+offset,guest_pstart+offset);
		res = dmmu_map_L1_section(guest_vstart+offset, guest_pstart+offset, attrs);
		printf("    Result: %d\n", res);
    }

    //printf("vm_0 pagetable after initialization:\n"); //DEBUG
	//dump_mmu(guest_pt_va); //DEBUG

    mem_mmu_tlb_invalidate_all(TRUE, TRUE);
    mem_cache_invalidate(TRUE, TRUE, TRUE); //Instruction, data, writeback
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
	//Gets the second guest, then increments guest counter by one.
    get_guest(guest++);
    curr_vm->mode_states[HC_GM_TRUSTED].ctx.sp = curr_vm->config->rpc_handlers->sp;
    curr_vm->mode_states[HC_GM_TRUSTED].ctx.psr= ARM_INTERRUPT_MASK | ARM_MODE_USER;
#endif
    
    guest = 0;

    //Initialize the context with the physical addresses.
	//TODO: This appears to be the only part of this function which is already
	//written for several VMs.
    do{
    	/* Initialize default values */
        for(i = 0; i < HC_NGUESTMODES;i++){
            curr_vm->mode_states[i].mode_config = (curr_vm->config->guest_modes[i]);
            curr_vm->mode_states[i].rpc_for = MODE_NONE;
            curr_vm->mode_states[i].rpc_to  = MODE_NONE;
        }
        curr_vm->current_guest_mode = MODE_NONE;
        curr_vm->interrupted_mode = MODE_NONE;
        curr_vm->current_mode_state = 0;
        curr_vm->mode_states[HC_GM_INTERRUPT].ctx.psr= ARM_MODE_USER;
        
        //Let the guest know where it is located - write to the third and fourth
		//registers in the context of that guest for physical and virtual
		//address, respectively.
        curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[3] =
              curr_vm->config->firmware->pstart;              
        curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[4] =
              curr_vm->config->firmware->vstart;
		curr_vm = curr_vm->next; //TODO: Moved this line from above the previous
								 //two. Should have no other effect apart from
								 //that this entire loops works for several VMs.
    } while(curr_vm != &vm_0);
    
	//TODO: This obviously needs to be done once for every VM.
    memory_commit();
    cpu_context_initial_set(&curr_vm->mode_states[HC_GM_KERNEL].ctx);

}


void slave_start_(){
	/* Flush and enable the cache, among other things. Defined in
	 * core/hw/cpu/family/model/cpu_init.c. Since caches are separate among
	 * cores, this should be run once for each core. */
    cpu_init();

    /* Initialize hypervisor guest modes and data structures according to config
	 * file in guest - defined in init.c. */
    //guests_init(); //TODO: This should be run by the main core, and initialize
					 //		 all secondary cores.
    
	start_guest(); //TODO: Currently we have a shared curr_vm, which includes
				   //	   among other things the state of processor registers. 
}
