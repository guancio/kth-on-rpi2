
#include "hyper.h"
#include "dmmu.h"

// DEBUG FLAGS
#define DEBUG_DMMU_L1_CHECKERS 1

extern virtual_machine *curr_vm;
extern uint32_t *flpt_va;


dmmu_entry_t *get_bft_entry_by_block_idx(addr_t ph_block)
{
    dmmu_entry_t * bft = (dmmu_entry_t *) DMMU_BFT_BASE_VA;
    return & bft[ ph_block];
}

dmmu_entry_t *get_bft_entry(addr_t adr_py)
{
  return get_bft_entry_by_block_idx(PA_TO_PH_BLOCK(adr_py));
}


void dmmu_init()
{
    uint32_t i;    
    dmmu_entry_t * bft = (dmmu_entry_t *) DMMU_BFT_BASE_VA;
    
    /* clear all entries in the table */
    for(i = 0; i < DMMU_BFT_COUNT ; i++) {
        bft[i].all = 0;
    }    
}

BOOL guest_pa_range_checker(pa, size) {
	uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
	uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
	if (!(pa >= (guest_start_pa)) && (pa + size - 1 <= guest_end_pa))
		return FALSE;
	return TRUE;
}


/* -------------------------------------------------------------------
 * L1 creation API it checks validity of created L1 by the guest
 -------------------------------------------------------------------*/
BOOL l1PT_checker(uint32_t l1_desc)
{
	l1_pt_t  *pt = (l1_pt_t *) (&l1_desc) ;
	dmmu_entry_t *bft_entry_pt = get_bft_entry_by_block_idx(PT_PA_TO_PH_BLOCK(pt->addr));

	uint32_t err_flag = 0; // to be set when one of the pages in the section is not a data page
	if (bft_entry_pt->type != PAGE_INFO_TYPE_L2PT) {
		err_flag = ERR_MMU_IS_NOT_L2_PT;
	}
	else if (bft_entry_pt->refcnt >= (MAX_30BIT - 4096)) {
		err_flag = ERR_MMU_REF_OVERFLOW;
	}
	else if (pt->pxn) {
		err_flag = ERR_MMU_AP_UNSUPPORTED;
	}
	else {
		return TRUE;
	}
#ifdef DEBUG_DMMU_L1_CHECKERS
	printf("l1PT_checker failed: %x %d\n", l1_desc, err_flag);
#endif
	return FALSE;
}

BOOL l1Sec_checker(uint32_t l1_desc, addr_t l1_base_pa_add)
{
	uint32_t ap;
	uint32_t err_flag = 0; // to be set when one of the pages in the section is not a data page
    uint32_t sec_idx;

	l1_sec_t  *sec = (l1_sec_t *) (&l1_desc) ;
	ap = GET_L1_AP(sec);

	if(sec->secIndic == 1) // l1_desc is a super section descriptor
		err_flag = ERR_MMU_SUPERSECTION;
	// TODO: (ap != 1) condition need to be added to proof of API
	else if((ap != 1) && (ap != 2) && (ap != 3))
		err_flag = ERR_MMU_AP_UNSUPPORTED;
	// TODO: Check also that the guest can not read into the hypervisor memory
	// TODO: in general we need also to prevent that it can read from the trusted component, thus identifying a more fine grade control
	//		 e.g. using domain
	// TODO: e.g. if you can read in user mode and the domain is the guest user domain or kernel domain then the pa must be in the guest memory
	else if (ap == 3) {
		uint32_t max_kernel_ac = (curr_vm->config->guest_modes[HC_GM_KERNEL]->domain_ac | curr_vm->config->guest_modes[HC_GM_TASK]->domain_ac);
		uint32_t page_domain_mask = (0b11 << (2 * sec->dom));
		uint32_t kernel_ac = max_kernel_ac & page_domain_mask;
		if (kernel_ac != 0) {
			if (!guest_pa_range_checker(START_PA_OF_SECTION(sec), SECTION_SIZE))
				err_flag = ERR_MMU_OUT_OF_RANGE_PA;
		}

		for(sec_idx = 0; sec_idx < 256; sec_idx++)
		{
			uint32_t ph_block_in_sec = PA_TO_PH_BLOCK(START_PA_OF_SECTION(sec)) | (sec_idx); // Address of a page in the section
			dmmu_entry_t *bft_entry_in_sec = get_bft_entry_by_block_idx(ph_block_in_sec);

			if(bft_entry_in_sec->type !=  PAGE_INFO_TYPE_DATA)
			{
				err_flag = ERR_MMU_PH_BLOCK_NOT_WRITABLE;
			}
			// if one of the L1 page table's pages is in the section
			if( ((((uint32_t)ph_block_in_sec) << 12) & L1_BASE_MASK) == l1_base_pa_add )
			{
				err_flag = ERR_MMU_NEW_L1_NOW_WRITABLE;
			}
			if(bft_entry_in_sec->refcnt >= (MAX_30BIT - 4096))
			{
				err_flag = ERR_MMU_REF_OVERFLOW;
			}
		}
	}
	if(err_flag != 0) {
#ifdef DEBUG_DMMU_L1_CHECKERS
	    printf("l1Sec_checker failed: %x %x %d\n", l1_desc, l1_base_pa_add, err_flag);
#endif
		return FALSE;
	}

	return TRUE;
}

BOOL l1Desc_validityChecker_dispatcher(uint32_t l1_type, uint32_t l1_desc, addr_t pgd)
{
	if(l1_type == 0)
		return TRUE;
	if (l1_type == 1)
		return l1PT_checker(l1_desc);
	if (l1_type == 2 )
		return l1Sec_checker(l1_desc, pgd);
	return FALSE;
}

void create_L1_refs_update(addr_t l1_base_pa_add)
{
	int l1_idx, sec_idx;
	for(l1_idx = 0; l1_idx < 4096; l1_idx++)
	{
		uint32_t l1_desc_pa_add = L1_IDX_TO_PA(l1_base_pa_add, l1_idx); // base address is 16KB aligned
		uint32_t l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, curr_vm->config);
		uint32_t l1_desc = *((uint32_t *) l1_desc_va_add);
		uint32_t l1_type = l1_desc & DESC_TYPE_MASK;
		if(l1_type == 1)
		{
			l1_pt_t  *pt = (l1_pt_t *) (&l1_desc) ;
			dmmu_entry_t *bft_entry_pt = get_bft_entry_by_block_idx(PT_PA_TO_PH_BLOCK(pt->addr));
			bft_entry_pt->refcnt += 1;
		}
		else if(l1_type == 2)
		{
			l1_sec_t  *sec = (l1_sec_t *) (&l1_desc) ;
			uint32_t ap = GET_L1_AP(sec);
			if(ap == 3)
			{
				for(sec_idx = 0; sec_idx < 256; sec_idx++)
				{
					uint32_t ph_block = PA_TO_PH_BLOCK(START_PA_OF_SECTION(sec)) | (sec_idx);
					dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);
					bft_entry->refcnt += 1;
				}
			}
		}
	}
}

int dmmu_create_L1_pt(addr_t l1_base_pa_add)
{
	  uint32_t l1_idx, pt_idx;
	  uint32_t l1_desc;
	  uint32_t l1_desc_va_add;
	  uint32_t l1_desc_pa_add;
	  uint32_t l1_type;
	  uint32_t ap;
	  uint32_t ph_block;
	  int i;

	  /*Check that the guest does not override the physical addresses outside its range*/
	  // TODO, where we take the guest assigned physical memory?
	  if (!guest_pa_range_checker(l1_base_pa_add, 4*PAGE_SIZE))
		  return ERR_MMU_OUT_OF_RANGE_PA;

	  /* 16KB aligned ? */
	  if (l1_base_pa_add != (l1_base_pa_add & 0xFFFFC000))
		  return ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED;

	  ph_block = PA_TO_PH_BLOCK(l1_base_pa_add);

    if(get_bft_entry_by_block_idx(ph_block)->type == PAGE_INFO_TYPE_L1PT &&
 		get_bft_entry_by_block_idx(ph_block+1)->type == PAGE_INFO_TYPE_L1PT &&
   		get_bft_entry_by_block_idx(ph_block+2)->type == PAGE_INFO_TYPE_L1PT &&
   		get_bft_entry_by_block_idx(ph_block+3)->type == PAGE_INFO_TYPE_L1PT) {
        return ERR_MMU_ALREADY_L1_PT;
    }

    /* try to allocate a PT in physical address */

    if(get_bft_entry_by_block_idx(ph_block)->type != PAGE_INFO_TYPE_DATA ||
    	get_bft_entry_by_block_idx(ph_block+1)->type != PAGE_INFO_TYPE_DATA ||
    	get_bft_entry_by_block_idx(ph_block+2)->type != PAGE_INFO_TYPE_DATA ||
    	get_bft_entry_by_block_idx(ph_block+3)->type != PAGE_INFO_TYPE_DATA)
        return ERR_MMU_PT_REGION;

    if(get_bft_entry_by_block_idx(ph_block)->refcnt != 0 ||
    		get_bft_entry_by_block_idx(ph_block+1)->refcnt != 0 ||
    		get_bft_entry_by_block_idx(ph_block+2)->refcnt != 0 ||
    		get_bft_entry_by_block_idx(ph_block+3)->refcnt != 0)
        return ERR_MMU_REFERENCED;


    // copies  the reserved virtual addresses from the master page table
    // each virtual page non-unmapped in the master page table is considered reserved
#ifdef DEBUG_PG_CONTENT
    for (l1_idx = 0; l1_idx < 4096; l1_idx++) {
    	l1_desc = *(guest_pt_va + l1_idx);
    	if(l1_desc != 0x0)
    		printf("pg %d %x \t\t", l1_idx, l1_desc);
    }
#endif
    for (l1_idx = 0; l1_idx < 4096; l1_idx++) {
    	l1_desc = *(flpt_va + l1_idx);
    	if (L1_TYPE(l1_desc) != UNMAPPED_ENTRY) {
        	l1_desc_pa_add = L1_IDX_TO_PA(l1_base_pa_add, l1_idx); // base address is 16KB aligned
        	l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, curr_vm->config);
        	*((uint32_t *) l1_desc_va_add) = l1_desc;
    	}
    }


    uint32_t sanity_check = TRUE;
    for(l1_idx = 0; l1_idx < 4096; l1_idx++)
    {
    	l1_desc_pa_add = L1_IDX_TO_PA(l1_base_pa_add, l1_idx); // base address is 16KB aligned
    	l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, curr_vm->config);
    	l1_desc = *((uint32_t *) l1_desc_va_add);
    	l1_type = l1_desc & DESC_TYPE_MASK;
        if(!(l1Desc_validityChecker_dispatcher(l1_type, l1_desc, l1_base_pa_add)))
        {
        	sanity_check = FALSE;
        }
    }
    if(!sanity_check)
    	return ERR_MMU_SANITY_CHECK_FAILED;

    create_L1_refs_update(l1_base_pa_add);
    get_bft_entry_by_block_idx(ph_block)->type = PAGE_INFO_TYPE_L1PT;
    get_bft_entry_by_block_idx(ph_block+1)->type = PAGE_INFO_TYPE_L1PT;
    get_bft_entry_by_block_idx(ph_block+2)->type = PAGE_INFO_TYPE_L1PT;
    get_bft_entry_by_block_idx(ph_block+3)->type = PAGE_INFO_TYPE_L1PT;

    return 0;
}
/* -------------------------------------------------------------------
 * Mapping a given section base to the specified entry of L1
 *  -------------------------------------------------------------------*/

uint32_t dmmu_map_L1_section(addr_t va, addr_t sec_base_add, uint32_t attrs)
{
  uint32_t l1_base_add;
  uint32_t l1_idx;
  uint32_t l1_desc;
  uint32_t l1_desc_va_add;
  uint32_t l1_desc_pa_add;
  uint32_t ap;
    
  /*Check that the guest does not override the virtual addresses used by the hypervisor */
  // HAL_VIRT_START is usually 0xf0000000, where the hypervisor code/data structures reside
#if 0
  if( va >= HAL_VIRT_START)
    return ERR_MMU_RESERVED_VA;

  if( va >= curr_vm->config->reserved_va_for_pt_access_start && va <= curr_vm->config->reserved_va_for_pt_access_end)
    return ERR_MMU_RESERVED_VA;
#else
  // user the master page table to discover if the va is reserved
  // WARNING: we can currently reserve only blocks of 1MB and non single blocks
  l1_idx = VA_TO_L1_IDX(va);
  l1_desc = *(flpt_va + l1_idx);
  if (L1_TYPE(l1_desc) != UNMAPPED_ENTRY) {
	  return ERR_MMU_RESERVED_VA;
  }
#endif

  /*Check that the guest does not override the physical addresses outside its range*/
  // TODO, where we take the guest assigned physical memory?
  if (!guest_pa_range_checker(sec_base_add, SECTION_SIZE))
	  return ERR_MMU_OUT_OF_RANGE_PA;

  COP_READ(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, (uint32_t)l1_base_add);
  l1_idx = VA_TO_L1_IDX(va);
  l1_desc_pa_add = L1_IDX_TO_PA(l1_base_add, l1_idx);
  l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, (curr_vm->config));
  l1_desc = *((uint32_t *) l1_desc_va_add);
  if (L1_TYPE(l1_desc) != UNMAPPED_ENTRY)
    return ERR_MMU_SECTION_NOT_UNMAPPED;

  // Access permission from the give attribute
  l1_desc = CREATE_L1_SEC_DESC(sec_base_add, attrs);

  l1_sec_t *l1_sec_desc = (l1_sec_t *) (&l1_desc);
  ap = GET_L1_AP(l1_sec_desc); 

  if((ap != 2) && (ap != 3))
    return ERR_MMU_AP_UNSUPPORTED;
  if(ap == 2)
    {
      // Updating memory with the new descriptor
      *((uint32_t *) l1_desc_va_add) = l1_sec_desc;
    }
  else if(ap == 3)
    {
      int sec_idx;
      BOOL sanity_check = TRUE;
      for(sec_idx = 0; sec_idx < 256; sec_idx++)
        {
	  uint32_t ph_block = PA_TO_PH_BLOCK(START_PA_OF_SECTION(l1_sec_desc)) | (sec_idx);
	  dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);
	  if((bft_entry->refcnt == MAX_30BIT) || (bft_entry->type != PAGE_INFO_TYPE_DATA)) {
	    sanity_check = FALSE;
	  }
	}
      if(!sanity_check)
    	  return ERR_MMU_PH_BLOCK_NOT_WRITABLE;
      for(sec_idx = 0; sec_idx < 256; sec_idx++)
	{
	  uint32_t ph_block = PA_TO_PH_BLOCK(START_PA_OF_SECTION(l1_sec_desc)) | (sec_idx);
	  dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);
	  bft_entry->refcnt += 1;
	}
      *((uint32_t *) l1_desc_va_add) = l1_desc;
    }
  return 0;     	
}

/* -------------------------------------------------------------------
 * Freeing an entry of the given L1 page table
 *  ------------------------------------------------------------------- */
uint32_t dmmu_unmap_L1_pageTable_entry (addr_t  va)
{
    uint32_t l1_base_add;
    uint32_t l1_idx;
	uint32_t l1_desc_pa_add;
	uint32_t l1_desc_va_add;
	uint32_t l1_desc;
	uint32_t l1_type;

     /*Check that the guest does not override the virtual addresses used by the hypervisor */
	 // HAL_VIRT_START is usually 0xf0000000, where the hypervisor code/data structures reside
#if 0
  if( va >= HAL_VIRT_START)
    return ERR_MMU_RESERVED_VA;

  if( va >= curr_vm->config->reserved_va_for_pt_access_start && va <= curr_vm->config->reserved_va_for_pt_access_end)
    return ERR_MMU_RESERVED_VA;
#else
  // user the master page table to discover if the va is reserved
  // WARNING: we can currently reserve only blocks of 1MB and non single blocks
  l1_idx = VA_TO_L1_IDX(va);
  l1_desc = *(flpt_va + l1_idx);
  if (L1_TYPE(l1_desc) != UNMAPPED_ENTRY) {
	  return ERR_MMU_RESERVED_VA;
  }
#endif

    COP_READ(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, (uint32_t)l1_base_add);
    l1_idx = VA_TO_L1_IDX(va);
	l1_desc_pa_add = L1_IDX_TO_PA(l1_base_add, l1_idx);
	l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, curr_vm->config);//PA_PT_ADD_VA(l1_desc_pa_add);
	l1_desc = *((uint32_t *) l1_desc_va_add);
	l1_type = L1_TYPE(l1_desc);
	// We are unmapping a PT
	if (l1_type == 1) {
	  l1_pt_t *l1_pt_desc = (l1_pt_t *) (&l1_desc);
	  uint32_t ph_block = PA_TO_PH_BLOCK(PA_OF_POINTED_PT(l1_pt_desc));
	  dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);
	  bft_entry->refcnt -= 1;
	  *((uint32_t *) l1_desc_va_add) = UNMAP_L1_ENTRY(l1_desc);
	}
	// We are unmapping a section
	if ((l1_type == 2) && (((l1_sec_t *) (&l1_desc))->secIndic == 0)) {
		l1_sec_t *l1_sec_desc = (l1_sec_t *) (&l1_desc);
		uint32_t ap = GET_L1_AP(l1_sec_desc);
		int sec_idx;
		if(ap == 3)
			for(sec_idx = 0; sec_idx < 256; sec_idx++)  {
				uint32_t ph_block = PA_TO_PH_BLOCK(START_PA_OF_SECTION(l1_sec_desc)) | (sec_idx);
				dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);
				bft_entry->refcnt -= 1;
			}
		*((uint32_t *) l1_desc_va_add) = UNMAP_L1_ENTRY(l1_desc);
	}
	// nothing, since the entry was already unmapped
	else {
	  return ERR_MMU_ENTRY_UNMAPPED;
	}

	isb();
	mem_mmu_tlb_invalidate_all(TRUE, TRUE);
	mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
	mem_cache_set_enable(TRUE);

    return 0;
}

/* -------------------------------------------------------------------
 * L2 creation API it checks validity of created L2 by the guest
 -------------------------------------------------------------------*/
BOOL l2Pt_desc_ap(addr_t l2_base_pa_add, l1_small_t *pg_desc)
{
	uint32_t ap = ((pg_desc->ap_3b) << 2) | (pg_desc->ap_0_1bs);
	dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(PA_TO_PH_BLOCK(START_PA_OF_SPT(pg_desc)));

	if (ap == 1 || ap == 2)
		return TRUE;
	if (ap == 3) {
		if (pg_desc->addr == (l2_base_pa_add >> 12))
			return FALSE;
		if (bft_entry->type != PAGE_INFO_TYPE_DATA)
			return FALSE;
		// TODO: Check also that the guest can not read into the hypervisor memory
		// TODO: in general we need also to prevent that it can read from the trusted component, thus identifying a more fine grade control
		//		 e.g. using domain
		// TODO: e.g. if you can read in user mode and the domain is the guest user domain or kernel domain then the pa must be in the guest memory
		  if (!guest_pa_range_checker(START_PA_OF_SPT(pg_desc), PAGE_SIZE))
			  return FALSE;
		return TRUE;
	}
	return FALSE;
}

BOOL l2PT_checker(addr_t l2_base_pa_add, uint32_t l2_desc)
{
	l1_small_t *pg_desc = (l1_small_t *) (&l2_desc) ;
	dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(PA_TO_PH_BLOCK(START_PA_OF_SPT(pg_desc)));

	if((bft_entry->refcnt < (MAX_30BIT - 1024)) && l2Pt_desc_ap(l2_base_pa_add, pg_desc))
		return TRUE;
	else
		return FALSE;
}

BOOL l2Desc_validityChecker_dispatcher(uint32_t l2_type, uint32_t l2_desc, addr_t l2_base_pa_add)
{
	if(l2_type == 0)
		return TRUE;
	if ((l2_type == 2) || (l2_type == 3))
		return l2PT_checker(l2_base_pa_add, l2_desc);
	return FALSE;
}

void create_L2_refs_update(addr_t l2_base_pa_add)
{
	uint32_t l2_desc_pa_add;
	uint32_t l2_desc_va_add;
	int l2_idx;
	for(l2_idx = 0; l2_idx < 1024; l2_idx++)
	{
		l2_desc_pa_add = L2_DESC_PA(l2_base_pa_add, l2_idx); // base address is 4KB aligned
		l2_desc_va_add = mmu_guest_pa_to_va(l2_desc_pa_add, curr_vm->config);
		uint32_t l2_desc = *((uint32_t *) l2_desc_va_add);
		uint32_t l2_type = l2_desc & DESC_TYPE_MASK;
		l1_small_t *pg_desc = (l1_small_t *) (&l2_desc) ;
		if((l2_type == 2) || (l2_type == 3))
		{
			uint32_t ap = ((pg_desc->ap_3b) << 2) | (pg_desc->ap_0_1bs);
		    uint32_t ph_block = PA_TO_PH_BLOCK(START_PA_OF_SPT(pg_desc));
		    dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);

			if((bft_entry->type == PAGE_INFO_TYPE_DATA) && (ap == 3))
				bft_entry->refcnt += 1;
		}
	}
}

void create_L2_pgtype_update(uint32_t l2_base_pa_add)
{

    uint32_t ph_block = PA_TO_PH_BLOCK(l2_base_pa_add);
    dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);
	bft_entry->type = PAGE_INFO_TYPE_L2PT;
}



uint32_t dmmu_create_L2_pt(addr_t l2_base_pa_add)
{
    uint32_t l2_desc_pa_add;
    uint32_t l2_desc_va_add;
    uint32_t l2_desc;
    uint32_t l2_type;
    uint32_t l2_idx;
    uint32_t l2_base_va_add = mmu_guest_pa_to_va(l2_base_pa_add, curr_vm->config);

    /*Check that the guest does not override the physical addresses outside its range*/
    // TODO, where we take the guest assigned physical memory?
    if (!guest_pa_range_checker(l2_base_pa_add, PAGE_SIZE))
		  return ERR_MMU_OUT_OF_RANGE_PA;

     //not 4KB aligned ?
    if(l2_base_pa_add != (l2_base_pa_add & L2_BASE_MASK))
        return ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED;

    uint32_t ph_block = PA_TO_PH_BLOCK(l2_base_pa_add);
    dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);

    if(bft_entry->type == PAGE_INFO_TYPE_L2PT)
        return ERR_MMU_ALREADY_L2_PT;

    // try to allocate a PT in either a PT page physical address or a referenced data page physical address
    if(bft_entry->type == PAGE_INFO_TYPE_L1PT)
    	return ERR_MMU_PT_REGION;
    if ((bft_entry->type == PAGE_INFO_TYPE_DATA) && (bft_entry->refcnt != 0))
    	return ERR_MMU_REFERENCED;

    uint32_t sanity_checker = TRUE;
    for(l2_idx = 0; l2_idx < 1024; l2_idx++)
        {
        	l2_desc_pa_add = L2_DESC_PA(l2_base_pa_add, l2_idx); // base address is 4KB aligned
        	l2_desc_va_add = mmu_guest_pa_to_va(l2_desc_pa_add, curr_vm->config);
        	l2_desc = *((uint32_t *) l2_desc_va_add);
        	l2_type = l2_desc & DESC_TYPE_MASK;
            if(!(l2Desc_validityChecker_dispatcher(l2_type, l2_desc, l2_base_pa_add)))
            	sanity_checker = FALSE;
        }

    if(sanity_checker)
    {
    	create_L2_refs_update(l2_base_pa_add);
    	create_L2_pgtype_update(l2_base_pa_add);
    }
    else
    	return ERR_MMU_SANITY_CHECK_FAILED;
    return 0;
}

/* -------------------------------------------------------------------
 * Mapping a given L2 to the specified entry of L1
 *  -------------------------------------------------------------------*/
int dmmu_l1_pt_map(addr_t va, addr_t l2_base_pa_add, uint32_t attrs)
{
    uint32_t l1_base_add;
    uint32_t l1_idx;
    uint32_t l1_desc_pa_add;
    uint32_t l1_desc_va_add;
    uint32_t l1_desc;
    uint32_t page_desc;

    // HAL_VIRT_START is usually 0xf0000000, where the hypervisor code/data structures reside
    /*Check that the guest does not override the virtual addresses used by the hypervisor */
#if 0
  if( va >= HAL_VIRT_START)
    return ERR_MMU_RESERVED_VA;

  if( va >= curr_vm->config->reserved_va_for_pt_access_start && va <= curr_vm->config->reserved_va_for_pt_access_end)
    return ERR_MMU_RESERVED_VA;
#else
  // user the master page table to discover if the va is reserved
  // WARNING: we can currently reserve only blocks of 1MB and non single blocks
  l1_idx = VA_TO_L1_IDX(va);
  l1_desc = *(flpt_va + l1_idx);
  if (L1_TYPE(l1_desc) != UNMAPPED_ENTRY) {
	  return ERR_MMU_RESERVED_VA;
  }
#endif
  	if (!guest_pa_range_checker(l2_base_pa_add, PAGE_SIZE))
  		return ERR_MMU_OUT_OF_RANGE_PA;

    uint32_t ph_block = PA_TO_PH_BLOCK(l2_base_pa_add);
    dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);

    if(bft_entry->type != PAGE_INFO_TYPE_L2PT)
        return ERR_MMU_IS_NOT_L2_PT;

    COP_READ(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, (uint32_t)l1_base_add);
    l1_idx = VA_TO_L1_IDX(va);
    l1_desc_pa_add = L1_IDX_TO_PA(l1_base_add, l1_idx);
    l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, (curr_vm->config));
    l1_desc = *((uint32_t *) l1_desc_va_add);
    if(L1_DESC_PXN(attrs))
    	return ERR_MMU_XN_BIT_IS_ON;
    //checks if the L1 entry is unmapped or not
    if((l1_desc & DESC_TYPE_MASK) != 0)
    	return ERR_MMU_PT_NOT_UNMAPPED;

    if(bft_entry->refcnt == MAX_30BIT)
    	return ERR_MMU_REF_OVERFLOW;
    bft_entry->refcnt += 1;
    // Updating memory with the new descriptor
    l1_desc = CREATE_L1_PT_DESC(l2_base_pa_add, attrs);
    *((uint32_t *) l1_desc_va_add) = l1_desc;
	return 0;
}

/* -------------------------------------------------------------------
 * Mapping a given page to the specified entry of L2
 *  -------------------------------------------------------------------*/
#define L2_DESC_ATTR_MASK 0x00000FFD
#define CREATE_L2_DESC(x, y) (L2_BASE_MASK & x) | (L2_DESC_ATTR_MASK & y) | (0b10)

int dmmu_l2_map_entry(addr_t l2_base_pa_add, uint32_t l2_idx, addr_t page_pa_add, uint32_t attrs)
{
	uint32_t l2_desc_pa_add;
	uint32_t l2_desc_va_add;
	uint32_t l2_desc;
    uint32_t ap;  // access permission

    /*Check that the guest does not override the physical addresses outside its range*/
  	if (!guest_pa_range_checker(l2_base_pa_add, PAGE_SIZE))
  		return ERR_MMU_OUT_OF_RANGE_PA;
  	if (!guest_pa_range_checker(page_pa_add, PAGE_SIZE))
  		return ERR_MMU_OUT_OF_RANGE_PA;

    l2_desc_pa_add = L2_IDX_TO_PA(l2_base_pa_add, l2_idx);
    l2_desc_va_add = mmu_guest_pa_to_va(l2_desc_pa_add, (curr_vm->config));
    l2_desc = *((uint32_t *) l2_desc_va_add);


    // Finding the corresponding entry for the page_pa_add and l2_base_pa_add in BFT
    uint32_t ph_block_pg = PA_TO_PH_BLOCK(page_pa_add);
    dmmu_entry_t *bft_entry_pg = get_bft_entry_by_block_idx(ph_block_pg);

    uint32_t ph_block_pt = PA_TO_PH_BLOCK(l2_base_pa_add);
    dmmu_entry_t *bft_entry_pt = get_bft_entry_by_block_idx(ph_block_pt);

    // Extracting access permission from the give page attribute
    ap = GET_L2_AP(attrs);
    if((ap != 1) && (ap != 2) && (ap != 3))
    	return ERR_MMU_AP_UNSUPPORTED;

    if (ap == 3) {
    	if (bft_entry_pg->refcnt == MAX_30BIT)
    		return ERR_MMU_INCOMPATIBLE_AP;
    	if (bft_entry_pg->type != PAGE_INFO_TYPE_DATA)
    		return ERR_MMU_INCOMPATIBLE_AP;
    	// The above check on page_pa_add already guarantee that we are not
    	// mapping to the hypervisor memory
    }

    if(bft_entry_pt->type != PAGE_INFO_TYPE_L2PT)
    	return ERR_MMU_IS_NOT_L2_PT;

    //checks if the L2 entry is unmapped or not
    if((l2_desc & DESC_TYPE_MASK) != 0)
    	return ERR_MMU_PT_NOT_UNMAPPED;

    //Updating page reference counter
    if(ap == 3)
    	bft_entry_pg->refcnt += 1;

    //Updating page table in memory
    l2_desc = CREATE_L2_DESC(page_pa_add, attrs);
    *((uint32_t *) l2_desc_va_add) = l2_desc;

    return 0;
}

/* -------------------------------------------------------------------
 * Unmapping an entry of given L2 page table
 *  -------------------------------------------------------------------*/
int dmmu_l2_unmap_entry(addr_t l2_base_pa_add, uint32_t l2_idx)
{
	uint32_t l2_desc_pa_add;
	uint32_t l2_desc_va_add;
	uint32_t l2_desc;
	uint32_t l2_type;
	uint32_t ap;  // access permission

  	if (!guest_pa_range_checker(l2_base_pa_add, PAGE_SIZE))
  		return ERR_MMU_OUT_OF_RANGE_PA;

	uint32_t ph_block = PA_TO_PH_BLOCK(l2_base_pa_add);
	dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);

	if(bft_entry->type != PAGE_INFO_TYPE_L2PT)
		return ERR_MMU_IS_NOT_L2_PT;

    l2_desc_pa_add = L2_IDX_TO_PA(l2_base_pa_add, l2_idx);
    l2_desc_va_add = mmu_guest_pa_to_va(l2_desc_pa_add, (curr_vm->config));
    l2_desc = *((uint32_t *) l2_desc_va_add);
    l2_type = l2_desc & DESC_TYPE_MASK;

	l1_small_t *pg_desc = (l1_small_t *) (&l2_desc) ;
	dmmu_entry_t *bft_entry_pg = get_bft_entry_by_block_idx(PA_TO_PH_BLOCK(START_PA_OF_SECTION(pg_desc)));

	ap = ((uint32_t)pg_desc->ap_3b) << 2 | pg_desc->ap_0_1bs;
	if(ap == 3)
		bft_entry_pg->refcnt -= 1;

	 //Updating page table in memory
	 l2_desc = UNMAP_L2_ENTRY(l2_desc);
	 *((uint32_t *) l2_desc_va_add) = l2_desc;

	return 0;
}

/* -------------------------------------------------------------------
 * Freeing a given L2 page table
 *  -------------------------------------------------------------------*/
int dmmu_unmap_L2_pt(addr_t l2_base_pa_add)
{
	uint32_t l2_desc_pa_add;
	uint32_t l2_desc_va_add;
	uint32_t l2_desc;
	uint32_t l2_type;
	uint32_t ap;  // access permission
	int l2_idx;

  	if (!guest_pa_range_checker(l2_base_pa_add, PAGE_SIZE))
  		return ERR_MMU_OUT_OF_RANGE_PA;

	uint32_t ph_block = PA_TO_PH_BLOCK(l2_base_pa_add);
	dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);

	// not 4KB aligned ?
	if((bft_entry->type != PAGE_INFO_TYPE_L2PT) || (l2_base_pa_add != (l2_base_pa_add & L2_BASE_MASK)))
		return ERR_MMU_IS_NOT_L2_PT;

	// There should be no reference to the page in the time of unmapping
    if(bft_entry->refcnt > 0)
    	return ERR_MMU_REFERENCE_L2;

    //updating the entries of L2
    for(l2_idx = 0; l2_idx < 1024; l2_idx++)
        {
    		l2_desc_pa_add = L2_DESC_PA(l2_base_pa_add, l2_idx); // base address is 4KB aligned
    		l2_desc_va_add = mmu_guest_pa_to_va(l2_desc_pa_add, curr_vm->config);
    		l2_desc = *((uint32_t *) l2_desc_va_add);
    		l1_small_t *pg_desc = (l1_small_t *) (&l2_desc) ;
    		dmmu_entry_t *bft_entry_pg = get_bft_entry_by_block_idx(PA_TO_PH_BLOCK(START_PA_OF_SPT(pg_desc)));
    		ap = ((uint32_t)pg_desc->ap_3b) << 2 | pg_desc->ap_0_1bs;
    		l2_type = l2_desc & DESC_TYPE_MASK;

        	if(l2_type == 0)
        		continue;
        	if ((l2_type == 2) || (l2_type == 3))
        	{
        		if(ap == 3)
        			bft_entry_pg->refcnt -= 1;
        	}
        }

    //Changing the type of the L2 page table to data page
    bft_entry->type = PAGE_INFO_TYPE_DATA;

    return 0;
}

/* -------------------------------------------------------------------
 * Switching active L1 page table
 *  -------------------------------------------------------------------*/
int dmmu_switch_mm(addr_t l1_base_pa_add)
{
	int i;
	uint32_t ph_block;

	/*Check that the guest does not override the physical addresses outside its range*/
	// TODO, where we take the guest assigned physical memory?
  	if (!guest_pa_range_checker(l1_base_pa_add, 4*PAGE_SIZE))
  		return ERR_MMU_OUT_OF_RANGE_PA;

	  /* 16KB aligned ? */
	if (l1_base_pa_add != (l1_base_pa_add & 0xFFFFC000))
		return ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED;

	ph_block = PA_TO_PH_BLOCK(l1_base_pa_add);

	if(get_bft_entry_by_block_idx(ph_block)->type != PAGE_INFO_TYPE_L1PT)
		return ERR_MMU_IS_NOT_L1_PT;

	// Switch the TTB and set context ID
	COP_WRITE(COP_SYSTEM,COP_CONTEXT_ID_REGISTER, 0); //Set reserved context ID
	isb();
    /* activate the guest page table */
    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
	COP_WRITE(COP_SYSTEM,COP_SYSTEM_TRANSLATION_TABLE0, l1_base_pa_add); // Set TTB0
	isb();
	mem_mmu_tlb_invalidate_all(TRUE, TRUE);
	mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
	mem_cache_set_enable(TRUE);
	return 0;
}
// ----------------------------------------------------------------

/* -------------------------------------------------------------------
 * Freeing a given L1 page table
 *  ------------------------------------------------------------------- */
int dmmu_unmap_L1_pt(addr_t l1_base_pa_add)
{
	uint32_t l1_idx, pt_idx, sec_idx;
	uint32_t l1_desc;
	uint32_t l1_desc_va_add;
	uint32_t l1_desc_pa_add;
	uint32_t l1_type;
	uint32_t ap;
	uint32_t ph_block;
	addr_t curr_l1_base_pa_add;
	int i;

	// checking to see

	  /*Check that the guest does not override the physical addresses outside its range*/
	  // TODO, where we take the guest assigned physical memory?
	if (!guest_pa_range_checker(l1_base_pa_add, 4*PAGE_SIZE))
  		return ERR_MMU_OUT_OF_RANGE_PA;

	  /* 16KB aligned ? */
	  if (l1_base_pa_add != (l1_base_pa_add & 0xFFFFC000))
		  return ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED;

	  // You can not free the current L1
	  COP_READ(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, (uint32_t)curr_l1_base_pa_add);
	  if (curr_l1_base_pa_add & 0xFFFFC000 == l1_base_pa_add & 0xFFFFC000)
		  return ERR_MMU_FREE_ACTIVE_L1;

	  ph_block = PA_TO_PH_BLOCK(l1_base_pa_add);


	if(get_bft_entry_by_block_idx(ph_block)->type != PAGE_INFO_TYPE_L1PT  ||
		get_bft_entry_by_block_idx(ph_block+1)->type != PAGE_INFO_TYPE_L1PT ||
		get_bft_entry_by_block_idx(ph_block+2)->type != PAGE_INFO_TYPE_L1PT ||
		get_bft_entry_by_block_idx(ph_block+3)->type != PAGE_INFO_TYPE_L1PT) {
		return ERR_MMU_IS_NOT_L1_PT;
	}

    //unmap_L1_pt_ref_update
	for(l1_idx = 0; l1_idx < 4096; l1_idx++)
	{
		uint32_t l1_desc_pa_add = L1_IDX_TO_PA(l1_base_pa_add, l1_idx); // base address is 16KB aligned
		uint32_t l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, curr_vm->config);
		uint32_t l1_desc = *((uint32_t *) l1_desc_va_add);
		uint32_t l1_type = l1_desc & DESC_TYPE_MASK;
		if(l1_type == 0)
			continue;
		if(l1_type == 1)
		{
			l1_pt_t  *pt = (l1_pt_t *) (&l1_desc) ;
			dmmu_entry_t *bft_entry_pt = get_bft_entry_by_block_idx(PT_PA_TO_PH_BLOCK(pt->addr));
			bft_entry_pt->refcnt -= 1;
		}
		if(l1_type == 2)
		{
			l1_sec_t  *sec = (l1_sec_t *) (&l1_desc) ;
			uint32_t ap = GET_L1_AP(sec);
			if(ap == 3)
			{
				for(sec_idx = 0; sec_idx < 256; sec_idx++)
				{
					uint32_t ph_block = PA_TO_PH_BLOCK(START_PA_OF_SECTION(sec)) | (sec_idx);
					dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);
					bft_entry->refcnt -= 1;
				}
			}
		}
	}
	//unmap_L1_pt_pgtype_update
    get_bft_entry_by_block_idx(ph_block)->type = PAGE_INFO_TYPE_DATA;
    get_bft_entry_by_block_idx(ph_block+1)->type = PAGE_INFO_TYPE_DATA;
    get_bft_entry_by_block_idx(ph_block+2)->type = PAGE_INFO_TYPE_DATA;
    get_bft_entry_by_block_idx(ph_block+3)->type = PAGE_INFO_TYPE_DATA;

    return 0;
}

enum dmmu_command {
	CMD_MAP_L1_SECTION, CMD_UNMAP_L1_PT_ENTRY, CMD_CREATE_L2_PT, CMD_MAP_L1_PT, CMD_MAP_L2_ENTRY, CMD_UNMAP_L2_ENTRY, CMD_FREE_L2, CMD_CREATE_L1_PT, CMD_SWITCH_ACTIVE_L1, CMD_FREE_L1
};

int dmmu_handler(uint32_t p03, uint32_t p1, uint32_t p2)
{
	uint32_t p0 = p03 & 0xF;
	uint32_t p3 = p03 >> 4;

    printf("dmmu_handler: DMMU %x %x %x\n", p1, p2, p3);
    
    switch(p0) {
    case CMD_CREATE_L1_PT:
    	return dmmu_create_L1_pt(p1);
    case CMD_MAP_L1_SECTION:
    	return dmmu_map_L1_section(p1,p2,p3);
    case CMD_UNMAP_L1_PT_ENTRY:
    	return dmmu_unmap_L1_pageTable_entry(p1);
    case CMD_CREATE_L2_PT:
    	return dmmu_create_L2_pt(p1);
    case CMD_MAP_L1_PT:
    	return dmmu_l1_pt_map(p1, p2, p3);
    case CMD_MAP_L2_ENTRY:
    	p3 = p03 & 0xFFFFFFF0;
    	uint32_t idx = p2 >> 20;
    	uint32_t attrs = p2 & 0xFFF;
    	return dmmu_l2_map_entry(p1, idx, p3, attrs);
     	//return dmmu_l2_map_entry(p1, p2, curr_vm->current_mode_state->ctx.reg[3], curr_vm->current_mode_state->ctx.reg[4]);
    case CMD_UNMAP_L2_ENTRY:
    	return dmmu_l2_unmap_entry(p1, p2);
    case CMD_FREE_L2:
    	return dmmu_unmap_L2_pt(p1);
    case CMD_FREE_L1:
    	return dmmu_unmap_L1_pt(p1);
    case CMD_SWITCH_ACTIVE_L1:
    	return dmmu_switch_mm(p1);
    default:
        return ERR_MMU_UNIMPLEMENTED;
    }
}
