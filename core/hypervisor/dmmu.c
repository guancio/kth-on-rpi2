
#include "hyper.h"
#include "dmmu.h"

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

// ----------------------------------------------------------------

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

// ----------------------------------------------------------------
// TEMP STUFF
enum dmmu_command {
    CMD_CREATE_L1
};

int dmmu_handler(uint32_t p0, uint32_t p1, uint32_t p2)
{
    printf("DMMU %x %x %x\n", p0, p1, p2);
    
    switch(p0) {
    case CMD_CREATE_L1:
        return dmmu_create_L1_pt(p1, p2);
    default:
        return -1; // ??
    }
}
