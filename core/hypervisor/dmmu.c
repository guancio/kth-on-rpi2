
#include "hyper.h"
#include "dmmu.h"

extern virtual_machine *curr_vm;

#define ERR_MMU_RESERVED_VA (1)
#define ERR_MMU_ENTRY_UNMAPPED (2)
#define ERR_MMU_OUT_OF_RANGE_PA (3)
#define ERR_MMU_SECTION_NOT_UNMAPPED (4)
#define ERR_MMU_PH_BLOCK_NOT_WRITABLE (5)
#define ERR_MMU_AP_UNSUPPORTED (6)
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
  // TODO, where we take the guest assigned phisical memory?
  uint32_t guest_start_pa = curr_vm->config->pa_for_pt_access_start;
  uint32_t guest_end_pa = curr_vm->config->pa_for_pt_access_end;
  if(!(sec_base_add >= (guest_start_pa) && sec_base_add <= guest_end_pa))
    return ERR_MMU_OUT_OF_RANGE_PA;

  COP_READ(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, (uint32_t)l1_base_add);
  l1_idx = VA_TO_L1_IDX(va);
  l1_desc_pa_add = L1_IDX_TO_PA(l1_base_add, l1_idx);
  l1_desc_va_add = mmu_guest_pa_to_va(l1_desc_pa_add, curr_vm->config);
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

// ----------------------------------------------------------------
// TEMP STUFF
enum dmmu_command {
    CMD_CREATE_L1
};

int dmmu_handler(uint32_t p0, uint32_t p1, uint32_t p2)
{
    printf("DMMU %x %x %x\n", p0, p1, p2);
    
    switch(p0) {
    default:
        return ERR_MMU_UNIMPLEMENTED;
    }
}
