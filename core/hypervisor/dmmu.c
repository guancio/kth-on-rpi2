
#include "hyper.h"
#include "dmmu.h"

#define ERR_MMU_RESERVED_VA (1)
#define ERR_MMU_ENTRY_UNMAPPED (2)
#define ERR_MMU_OUT_OF_RANGE_PA (3)
#define ERR_MMU_SECTION_NOT_UNMAPPED (4)
#define ERR_MMU_PH_BLOCK_NOT_WRITABLE (5)
#define ERR_MMU_AP_UNSUPPORTED (6)
#define ERR_MMU_UNIMPLEMENTED (-1)

#if 0
int dmmu_create_L1_pt(addr_t pgd_va, addr_t pgd_py)
{
    addr_t pgd_py[4];
    dmmu_entry_t *pgd_de[4];
    int n;
    
    // XXX: activate_hyper_pt();
    
    
    /* 16KB aligned ? */
    if(pgd_va & (16 * 1024 -1))
        return 1;
    
    /* see if in active memory, assume 4KB pages */
    for(n = 0; n < 4; n++) {
        addr_t va = pgd_va + i * 4096;
        if(! mmu_lookup_guest(va, & pgd_py[i], 1))  /* guest writable? */
            return 2;
        pgd_de[i] = get_bft_entry(va);
    }
    
    /* is it already marked as L1 ? */
    if(pgd_de[0].type == DMMU_TYPE_L1PT && 
       pgd_de[1].type == DMMU_TYPE_L1PT && 
       pgd_de[2].type == DMMU_TYPE_L1PT &&                      
       pgd_de[3].type == DMMU_TYPE_L1PT) {
        // XXX: activatee_guest_pt();
        return 0;
    }
    
    // TODO: the remaining stuff
           
    n = mmu_lookup_hv(pgd_va, &tmp, 0);    
    printf("H-ro: %x -> %x/%d\n", pgd_va, tmp, n);
    
    n = mmu_lookup_hv(pgd_va, &tmp, 1);
    printf("H-rw: %x -> %x/%d\n", pgd_va, tmp, n);        
    
    n = mmu_lookup_guest(pgd_va, &tmp, 0);    
    printf("G-ro: %x -> %x/%d\n", pgd_va, tmp, n);
    
    n = mmu_lookup_guest(pgd_va, &tmp, 1);
    printf("G-rw: %x -> %x/%d\n", pgd_va, tmp, n);        
    
    
    return 1;    
}
#endif

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
// ----------------------------------------------------------------
dmmu_entry_t *get_bft_entry(addr_t adr_py)
{
    dmmu_entry_t * bft = (dmmu_entry_t *) DMMU_BFT_BASE_VA;
    return & bft[ adr_py >> 12];
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
