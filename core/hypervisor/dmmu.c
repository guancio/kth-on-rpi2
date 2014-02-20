
#include "hyper.h"
#include "dmmu.h"

extern virtual_machine *curr_vm;

#define ERR_MMU_RESERVED_VA (1)
#define ERR_MMU_ENTRY_UNMAPPED (2)
#define ERR_MMU_OUT_OF_RANGE_PA (3)
#define ERR_MMU_SECTION_NOT_UNMAPPED (4)
#define ERR_MMU_PH_BLOCK_NOT_WRITABLE (5)
#define ERR_MMU_AP_UNSUPPORTED (6)
#define ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED (7)
#define ERR_MMU_ALREADY_L1_PT (9)
#define ERR_MMU_ALREADY_L2_PT (9)
#define ERR_MMU_REFERENCED_OR_PT_REGION (10)
#define ERR_MMU_NO_UPDATE (11)
#define ERR_MMU_IS_NOT_L2_PT (12)
#define ERR_MMU_XN_BIT_IS_ON (13)
#define ERR_MMU_PT_NOT_UNMAPPED (14)
#define ERR_MMU_REF_OVERFLOW (15)
#define ERR_MMU_UNIMPLEMENTED (-1)

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

// ----------------------------------------------------------------

#if 0
int dmmu_create_L1_pt(addr_t pgd_va, addr_t pgd_py)
{
    addr_t pgd_py4[4];
    dmmu_entry_t *pgd_de[4];
    int i;
    
    // XXX: activate_hyper_pt();
    
    
    /* 16KB aligned ? */
    if(pgd_va & (16 * 1024 -1))
        return 1;
    
    /* see if in active memory, assume 4KB pages */
    for(i = 0; i < 4; i++) {
        addr_t va = pgd_va + i * 4096;
        if(! mmu_lookup_guest(va, & pgd_py4[i], 1))  /* guest writable? */
            return 2;
        pgd_de[i] = get_bft_entry(pgd_py4[i]);
    }
    
    /* is it already marked as L1 ? */
    if(pgd_de[0]->type == DMMU_TYPE_L1PT && 
       pgd_de[1]->type == DMMU_TYPE_L1PT && 
       pgd_de[2]->type == DMMU_TYPE_L1PT &&                      
       pgd_de[3]->type == DMMU_TYPE_L1PT) {
        // XXX: activate_guest_pt();
        return 0;
    }
          
    
    // TODO: the remaining stuff
    
    
    // DEBUG:
    {
        int n;
        addr_t tmp;
        
        n = mmu_lookup_hv(pgd_va, &tmp, 0);    
        printf("H-ro: %x -> %x/%d\n", pgd_va, tmp, n);
        
        n = mmu_lookup_hv(pgd_va, &tmp, 1);
        printf("H-rw: %x -> %x/%d\n", pgd_va, tmp, n);        
        
        n = mmu_lookup_guest(pgd_va, &tmp, 0);    
        printf("G-ro: %x -> %x/%d\n", pgd_va, tmp, n);
        
        n = mmu_lookup_guest(pgd_va, &tmp, 1);
        printf("G-rw: %x -> %x/%d\n", pgd_va, tmp, n);        
    }
    
    
    return 1;    
}
#endif


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
  if( va >= HAL_VIRT_START)
    return ERR_MMU_RESERVED_VA;

  if( va >= curr_vm->config->reserved_va_for_pt_access_start && va <= curr_vm->config->reserved_va_for_pt_access_end)
    return ERR_MMU_RESERVED_VA;

  /*Check that the guest does not override the physical addresses outside its range*/
  // TODO, where we take the guest assigned physical memory?
  uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
  uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
  if(!(sec_base_add >= (guest_start_pa) && sec_base_add <= guest_end_pa))
    return ERR_MMU_OUT_OF_RANGE_PA;

  COP_READ(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, (uint32_t)l1_base_add);
  l1_idx = VA_TO_L1_IDX(va);
  l1_desc_pa_add = L1_IDX_TO_PA(l1_base_add, l1_idx);
  l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, (curr_vm->config));
  l1_desc = *((uint32_t *) l1_desc_va_add);
  if(L1_TYPE(l1_desc) != UNMAPPED_ENTRY)
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
    if( va >= HAL_VIRT_START)
    	return ERR_MMU_RESERVED_VA;

    if( va >= curr_vm->config->reserved_va_for_pt_access_start && va <= curr_vm->config->reserved_va_for_pt_access_end)
      return ERR_MMU_RESERVED_VA;


    COP_READ(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, (uint32_t)l1_base_add);
    l1_idx = VA_TO_L1_IDX(va);
	l1_desc_pa_add = L1_IDX_TO_PA(l1_base_add, l1_idx);
	l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, curr_vm->config);//PA_PT_ADD_VA(l1_desc_pa_add);
	l1_desc = *((uint32_t *) l1_desc_va_add);
	l1_type = L1_TYPE(l1_desc);
	// We are unmapping a PT
	if (l1_type == 1) {
	  l1_pt_t *l1_pt_desc = (l1_pt_t *) (&l1_desc);
	  *((uint32_t *) l1_desc_va_add) = UNMAP_L1_ENTRY(l1_desc);
	  uint32_t ph_block = PA_TO_PH_BLOCK(PA_OF_POINTED_PT(l1_pt_desc));
	  dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);
	  bft_entry->refcnt -= 1;
	}
	// We are unmapping a section
	if ((l1_type == 2) && (((l1_sec_t *) (&l1_desc))->secIndic == 0)) {
		l1_sec_t *l1_sec_desc = (l1_sec_t *) (&l1_desc);
		*((uint32_t *) l1_desc_va_add) = UNMAP_L1_ENTRY(l1_desc);
		uint32_t ap = GET_L1_AP(l1_sec_desc);
		int sec_idx;
		if(ap == 3)
			for(sec_idx = 0; sec_idx < 256; sec_idx++)  {
				// TODO: fix also for negative numbers
				uint32_t ph_block = PA_TO_PH_BLOCK(START_PA_OF_SECTION(l1_sec_desc)) | (sec_idx);
				dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);
				bft_entry->refcnt -= 1;
			}
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
#define L2_DESC_PA(l2_base_add, l2_idx) (l2_base_add | (l2_idx << 2) | 0)

BOOL l2Pt_desc_ap(addr_t l2_base_pa_add, l1_small_t *pg_desc)
{
	uint32_t ap = ((pg_desc->ap_3b) << 2) | (pg_desc->ap_0_1bs);
	dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(pg_desc->addr);
	if((bft_entry->type != PAGE_INFO_TYPE_DATA) ||
		(pg_desc->addr == (l2_base_pa_add >> 12)))
		return ((ap == 1) || (ap == 2));
	else
		return ((ap == 1) || (ap == 2) || (ap == 3));
}

BOOL l2PT_checker(addr_t l2_base_pa_add, uint32_t l2_desc)
{
	l1_small_t *pg_desc = (l1_small_t *) (&l2_desc) ;
	dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(pg_desc->addr);
	if((bft_entry->refcnt < (MAX_30BIT - 1024)) && l2Pt_desc_ap(l2_base_pa_add, pg_desc)
	  )
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
		    uint32_t ph_block = PA_TO_PH_BLOCK(pg_desc->addr);
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

      //TODO: do we need the following check over vritual addresses range??
//    /*Check that the guest does not override the virtual addresses used by the hypervisor */
//    // HAL_VIRT_START is usually 0xf0000000, where the hypervisor code/data structures reside
//    if( l2_base_va_add >= HAL_VIRT_START)
//    	return ERR_MMU_RESERVED_VA;
//
//    if( l2_base_va_add >= curr_vm->config->reserved_va_for_pt_access_start && l2_base_va_add <= curr_vm->config->reserved_va_for_pt_access_end)
//    	return ERR_MMU_RESERVED_VA;

    /*Check that the guest does not override the physical addresses outside its range*/
     // TODO, where we take the guest assigned physical memory?
     uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
     uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
     if(!(l2_base_pa_add >= (guest_start_pa) && l2_base_pa_add <= guest_end_pa))
    	 return ERR_MMU_OUT_OF_RANGE_PA;

     //not 4KB aligned ?
    if(l2_base_pa_add != (l2_base_pa_add & L2_BASE_MASK))
        return ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED;

    uint32_t ph_block = PA_TO_PH_BLOCK(l2_base_pa_add);
    dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);

    if(bft_entry->type == PAGE_INFO_TYPE_L2PT)
        return ERR_MMU_ALREADY_L2_PT;

    // try to allocate a PT in either a PT page physical address or a referenced data page physical address
    if(bft_entry->type == PAGE_INFO_TYPE_L1PT ||
      ((bft_entry->type == PAGE_INFO_TYPE_DATA) && (bft_entry->refcnt != 0)))
    	return ERR_MMU_REFERENCED_OR_PT_REGION;
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
    	return ERR_MMU_NO_UPDATE;
    return 0;
}

/* -------------------------------------------------------------------
 * Mapping a given L2 to the specified entry of L1
 *  -------------------------------------------------------------------*/
#define L1_PT_DESC_MASK 0xFFFFFC00
#define L1_PT_DESC_ATTR_MASK 0x000003FC
#define CREATE_L1_PT_DESC(x, y) (L1_PT_DESC_MASK & x) | (L1_PT_DESC_ATTR_MASK & y) | (0b01)
#define L1_DESC_PXN(x) ((x & 0x4) >> 2)

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
    if( va >= HAL_VIRT_START)
    	return ERR_MMU_RESERVED_VA;

    if( va >= curr_vm->config->reserved_va_for_pt_access_start && va <= curr_vm->config->reserved_va_for_pt_access_end)
    	return ERR_MMU_RESERVED_VA;

    uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
    uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
    /*Check that the guest does not override the physical addresses outside its range*/
    if(!(l2_base_pa_add >= (guest_start_pa) && l2_base_pa_add <= guest_end_pa))
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


// ----------------------------------------------------------------
// TEMP STUFF
enum dmmu_command {
	CMD_MAP_L1_SECTION, CMD_UNMAP_L1_PT_ENTRY, CMD_CREATE_L2_PT, CMD_MAP_L1_PT
};

int dmmu_handler(uint32_t p03, uint32_t p1, uint32_t p2)
{
	uint32_t p0 = p03 & 0xF;
	uint32_t p3 = p03 >> 4;

    printf("DMMU %x %x %x\n", p1, p2, p3);
    
    switch(p0) {
    case CMD_MAP_L1_SECTION:
    	return dmmu_map_L1_section(p1,p2,p3);
    case CMD_UNMAP_L1_PT_ENTRY:
    	return dmmu_unmap_L1_pageTable_entry(p1);
    case CMD_CREATE_L2_PT:
    	return dmmu_create_L2_pt(p1);
    case CMD_MAP_L1_PT:
    	return dmmu_l1_pt_map(p1, p2, p3);
    default:
        return ERR_MMU_UNIMPLEMENTED;
    }
}
