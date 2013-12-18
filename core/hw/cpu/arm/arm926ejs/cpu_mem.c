
#include "types.h"
#include "arm_common.h"
#include "cpu_cop.h"

/*
 * TLB
 * */

void mem_mmu_tlb_invalidate_all(BOOL inst, BOOL data)
{
    uint32_t tmp = 0;
    if(inst && data)
        COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ALL, tmp);   
    else if(inst)
        COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ALL_INST, tmp);
    else if(data)
        COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ALL_DATA, tmp);      
}

void mem_mmu_tlb_invalidate_one(BOOL inst, BOOL data, uint32_t virtual_addr)
{
    uint32_t tmp = virtual_addr;
    
    if(inst && data)
        COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ONE, tmp);
    else if(inst)
        COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ONE_INST, tmp);      
    else if(data)
        COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ONE_DATA, tmp);
}


/*
 * CACHES 
 */


void mem_cache_set_enable(BOOL enable)
{
    uint32_t control_register;
    
    COP_READ( COP_SYSTEM, COP_SYSTEM_CONTROL, control_register);
    if(enable)
    	control_register |= COP_SYSTEM_CONTROL_CACHE_ENABLE;
    else
    	control_register &= ~COP_SYSTEM_CONTROL_CACHE_ENABLE;
    COP_WRITE( COP_SYSTEM, COP_SYSTEM_CONTROL, control_register);
    
}

static void mem_icache_clean_and_invalidate()
{

    /* method 2: test and clean (ARM926EJ-S only) */
    asm(".dcache_clean:\n"
        "mrc p15, 0, pc, c7, c14, 3\n"
        "bne .dcache_clean"
        );
}
void mem_cache_invalidate(BOOL inst_inv, BOOL data_inv, BOOL data_writeback)
{
    uint32_t tmp = 1;
    
    /* first data caches (needed for self-modifying code?) */
    if(data_inv) {
        if(data_writeback) {
            /* write back modified lines */
            mem_icache_clean_and_invalidate();            
        } else {
            /* just flush the cache */
            COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_ALL, tmp);            
        }
    }
    
    
    /* just flush the instruction cache */
    if(inst_inv) {
        COP_WRITE(COP_SYSTEM, COP_ICACHE_INVALIDATE_ALL, tmp);
    } 
}
