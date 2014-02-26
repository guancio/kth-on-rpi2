
#include "hyper.h"
#include "dmmu.h"

extern virtual_machine *curr_vm;

#define ERR_MMU_RESERVED_VA                 (1)
#define ERR_MMU_ENTRY_UNMAPPED 		        (2)
#define ERR_MMU_OUT_OF_RANGE_PA             (3)
#define ERR_MMU_SECTION_NOT_UNMAPPED        (4)
#define ERR_MMU_PH_BLOCK_NOT_WRITABLE       (5)
#define ERR_MMU_AP_UNSUPPORTED              (6)
#define ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED (7)
#define ERR_MMU_ALREADY_L1_PT               (8)
#define ERR_MMU_ALREADY_L2_PT               (8)
#define ERR_MMU_SANITY_CHECK_FAILED         (9)
#define ERR_MMU_REFERENCED_OR_PT_REGION     (10)
#define ERR_MMU_NO_UPDATE                   (11)
#define ERR_MMU_IS_NOT_L2_PT                (12)
#define ERR_MMU_XN_BIT_IS_ON                (13)
#define ERR_MMU_PT_NOT_UNMAPPED             (14)
#define ERR_MMU_REF_OVERFLOW                (15)
#define ERR_MMU_INCOMPATIBLE_AP             (16)
#define ERR_MMU_L2_UNSUPPORTED_DESC_TYPE    (17)
#define ERR_MMU_REFERENCE_L2                (18)
#define ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED (19)
#define ERR_MMU_IS_NOT_L1_PT                (20)
#define ERR_MMU_UNIMPLEMENTED               (-1)

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

/* -------------------------------------------------------------------
 * L1 creation API it checks validity of created L1 by the guest
 -------------------------------------------------------------------*/
//l1_small_t *pg_desc = (l1_small_t *) (&l2_desc) ;
//    		dmmu_entry_t *bft_entry_pg = get_bft_entry_by_block_idx(pg_desc->addr);

BOOL l1PT_checker(uint32_t l1_desc)
{
	l1_pt_t  *pt = (l1_pt_t *) (&l1_desc) ;
	dmmu_entry_t *bft_entry_pt = get_bft_entry_by_block_idx(PT_PA_TO_PH_BLOCK(pt->addr));
	if((bft_entry_pt->type == PAGE_INFO_TYPE_L2PT)   &&
	   (bft_entry_pt->refcnt < (MAX_30BIT - 4096))   &&
	   !(pt->pxn)
	  )
		return TRUE;
	else
		return FALSE;
}
BOOL l1Sec_checker(uint32_t l1_desc, addr_t l1_base_pa_add)
{
	uint32_t ap;
    int err_flag = 0; // to be set when one of the pages in the section is not a data page
    int sec_idx;

	l1_sec_t  *sec = (l1_sec_t *) (&l1_desc) ;
	uint32_t ph_block = PA_TO_PH_BLOCK(START_PA_OF_SECTION(sec));
	dmmu_entry_t *bft_entry_sec = get_bft_entry_by_block_idx(ph_block);
	ap = GET_L1_AP(sec);

	if(sec->secIndic == 1) // l1_desc is a super section descriptor
		return FALSE;
	if((ap != 2) && (ap != 3))
		return FALSE;
	if((ap == 3) && (START_PA_OF_SECTION(sec) == (START_PA_OF_SECTION(sec) & L1_SEC_DESC_MASK))) // TODO: second seems to be superfluous
		for(sec_idx = 0; sec_idx < 256; sec_idx++)
		{
			uint32_t ph_block_in_sec = PA_TO_PH_BLOCK(START_PA_OF_SECTION(sec)) | (sec_idx); // Address of a page in the section
			dmmu_entry_t *bft_entry_in_sec = get_bft_entry_by_block_idx(ph_block_in_sec);

			if(bft_entry_in_sec->type !=  PAGE_INFO_TYPE_DATA)
			{
				err_flag = 1;
				//break;
			}
			// if one of the L1 page table's pages is in the section
			if( ((((uint32_t)ph_block_in_sec) << 12) & L1_BASE_MASK) == l1_base_pa_add )
			{
				err_flag = 1;
				//break;
			}
			if(bft_entry_in_sec->refcnt >= (MAX_30BIT - 4096))
			{
				err_flag = 1;
				//break;
			}

		}
	if(err_flag != 0)
		return FALSE;

	return TRUE;
}

BOOL l1Desc_validityChecker_dispatcher(unsigned int l1_type, unsigned int l1_desc, addr_t pgd)
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
	  dmmu_entry_t *bft_entry[4];
	  int i;

	  /*Check that the guest does not override the physical addresses outside its range*/
	  // TODO, where we take the guest assigned physical memory?
	  uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
	  uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
	  for(i = 0; i < 4; i++)
	  {
		  if(!((l1_base_pa_add + (i * 4096)) >= (guest_start_pa) && (l1_base_pa_add + (i * 4096)) <= guest_end_pa))
			  return ERR_MMU_OUT_OF_RANGE_PA;
		  uint32_t ph_block = PA_TO_PH_BLOCK(l1_base_pa_add) + i;
		  bft_entry[i] = get_bft_entry_by_block_idx(ph_block);
	  }

	  /* 16KB aligned ? */
	  if(l1_base_pa_add & (16 * 1024 -1))
		  return ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED;


    if(bft_entry[0]->type == PAGE_INFO_TYPE_L1PT &&
       bft_entry[1]->type == PAGE_INFO_TYPE_L1PT &&
       bft_entry[2]->type == PAGE_INFO_TYPE_L1PT &&
       bft_entry[3]->type == PAGE_INFO_TYPE_L1PT) {
        // TODO: active_guest_pt();
        return ERR_MMU_ALREADY_L1_PT;
    }

    /* try to allocate a PT in physical address */

    if(bft_entry[0]->type != PAGE_INFO_TYPE_DATA ||
       bft_entry[1]->type != PAGE_INFO_TYPE_DATA ||
       bft_entry[2]->type != PAGE_INFO_TYPE_DATA ||
       bft_entry[3]->type != PAGE_INFO_TYPE_DATA)
        return ERR_MMU_REFERENCED_OR_PT_REGION;

    if(bft_entry[0]->refcnt != 0 ||
       bft_entry[0]->refcnt != 0 ||
       bft_entry[0]->refcnt != 0 ||
       bft_entry[0]->refcnt != 0)
        return ERR_MMU_REFERENCED_OR_PT_REGION;
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
    if(sanity_check)
    {
    	create_L1_refs_update(l1_base_pa_add);
        for(pt_idx = 0; pt_idx < 4; pt_idx++)
        {
        	bft_entry[pt_idx]->type = PAGE_INFO_TYPE_L1PT;
        }
    }
    else
    	return ERR_MMU_SANITY_CHECK_FAILED;
    // TODO: setup_hyper_address(pgd);
    return 0; // TODO
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

    uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
    uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
    /*Check that the guest does not override the physical addresses outside its range*/
    if(!(l2_base_pa_add >= (guest_start_pa) && l2_base_pa_add <= guest_end_pa))
    	return ERR_MMU_OUT_OF_RANGE_PA;

    if(!(page_pa_add >= (guest_start_pa) && page_pa_add <= guest_end_pa))
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

    if((ap == 3) && ((bft_entry_pg->refcnt == MAX_30BIT) || (bft_entry_pg->type != PAGE_INFO_TYPE_DATA)))
    	return ERR_MMU_INCOMPATIBLE_AP;

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

	uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
	uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
	/*Check that the guest does not override the physical addresses outside its range*/
	if(!(l2_base_pa_add >= (guest_start_pa) && l2_base_pa_add <= guest_end_pa))
		return ERR_MMU_OUT_OF_RANGE_PA;

	uint32_t ph_block = PA_TO_PH_BLOCK(l2_base_pa_add);
	dmmu_entry_t *bft_entry = get_bft_entry_by_block_idx(ph_block);

	if(bft_entry->type != PAGE_INFO_TYPE_L2PT)
		return ERR_MMU_IS_NOT_L2_PT;

    l2_desc_pa_add = L2_IDX_TO_PA(l2_base_pa_add, l2_idx);
    l2_desc_va_add = mmu_guest_pa_to_va(l2_desc_pa_add, (curr_vm->config));
    l2_desc = *((uint32_t *) l2_desc_va_add);
    l2_type = l2_desc & DESC_TYPE_MASK;

    if((l2_type != 2) && (l2_type != 3))
    	return ERR_MMU_L2_UNSUPPORTED_DESC_TYPE;

	l1_small_t *pg_desc = (l1_small_t *) (&l2_desc) ;
	dmmu_entry_t *bft_entry_pg = get_bft_entry_by_block_idx(pg_desc->addr);

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

	uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
	uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
	/*Check that the guest does not override the physical addresses outside its range*/
	if(!(l2_base_pa_add >= (guest_start_pa) && l2_base_pa_add <= guest_end_pa))
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
    		dmmu_entry_t *bft_entry_pg = get_bft_entry_by_block_idx(pg_desc->addr);
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
	dmmu_entry_t *bft_entry[4];
	int i;

	/*Check that the guest does not override the physical addresses outside its range*/
	// TODO, where we take the guest assigned physical memory?
	uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
	uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
	for(i = 0; i < 4; i++)
	{
		if(!((l1_base_pa_add + (i * 4096)) >= (guest_start_pa) && (l1_base_pa_add + (i * 4096)) <= guest_end_pa))
			return ERR_MMU_OUT_OF_RANGE_PA;
		uint32_t ph_block = PA_TO_PH_BLOCK(l1_base_pa_add) + i;
		bft_entry[i] = get_bft_entry_by_block_idx(ph_block);
	}

	  /* 16KB aligned ? */
	if(l1_base_pa_add & (16 * 1024 -1))
		return ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED;

	if(bft_entry[0]->type != PAGE_INFO_TYPE_L1PT)
		return ERR_MMU_IS_NOT_L1_PT;

	// Switch the TTB and set context ID
	COP_WRITE(COP_SYSTEM,COP_CONTEXT_ID_REGISTER, 0); //Set reserved context ID
	isb();
	COP_WRITE(COP_SYSTEM,COP_SYSTEM_TRANSLATION_TABLE0, l1_base_pa_add); // Set TTB0
	isb();
	return 0;
}

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
	dmmu_entry_t *bft_entry[4];
	int i;

	/*Check that the guest does not override the physical addresses outside its range*/
	// TODO, where we take the guest assigned physical memory?
	uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
	uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
	for(i = 0; i < 4; i++)
	{
		if(!((l1_base_pa_add + (i * 4096)) >= (guest_start_pa) && (l1_base_pa_add + (i * 4096)) <= guest_end_pa))
			return ERR_MMU_OUT_OF_RANGE_PA;
		uint32_t ph_block = PA_TO_PH_BLOCK(l1_base_pa_add) + i;
		bft_entry[i] = get_bft_entry_by_block_idx(ph_block);
	}

	  /* 16KB aligned ? */
	if(l1_base_pa_add & (16 * 1024 -1))
		return ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED;


	if(bft_entry[0]->type != PAGE_INFO_TYPE_L1PT  ||
		bft_entry[1]->type != PAGE_INFO_TYPE_L1PT ||
		bft_entry[2]->type != PAGE_INFO_TYPE_L1PT ||
		bft_entry[3]->type != PAGE_INFO_TYPE_L1PT) {
      // TODO: active_guest_pt();
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
    for(pt_idx = 0; pt_idx < 4; pt_idx++)
    {
    	bft_entry[pt_idx]->type = PAGE_INFO_TYPE_DATA;
    }

    return 0;
}
// ----------------------------------------------------------------
// TEMP STUFF
enum dmmu_command {
	CMD_MAP_L1_SECTION, CMD_UNMAP_L1_PT_ENTRY, CMD_CREATE_L2_PT, CMD_MAP_L1_PT, CMD_MAP_L2_ENTRY, CMD_UNMAP_L2_ENTRY, CMD_FREE_L2, CMD_CREATE_L1_PT, CMD_SWITCH_ACTIVE_L1, CMD_FREE_L1
};

int dmmu_handler(uint32_t p03, uint32_t p1, uint32_t p2)
{
	uint32_t p0 = p03 & 0xF;
	uint32_t p3 = p03 >> 4;

    printf("DMMU %x %x %x\n", p1, p2, p3);
    
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
     	return dmmu_l2_map_entry(p1, p2, curr_vm->current_mode_state->ctx.reg[3], curr_vm->current_mode_state->ctx.reg[4]);
    case CMD_UNMAP_L2_ENTRY:
    	return dmmu_l2_unmap_entry(p1, p2);
    case CMD_FREE_L2:
    	return dmmu_unmap_L2_pt(p1);
    case CMD_SWITCH_ACTIVE_L1:
    	return dmmu_switch_mm(p1);
    case CMD_FREE_L1:
    	return dmmu_unmap_L1_pt(p1);
    default:
        return ERR_MMU_UNIMPLEMENTED;
    }
}
