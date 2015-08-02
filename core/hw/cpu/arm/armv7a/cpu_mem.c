#include "types.h"
#include "arm_common.h"
#include "cpu_cop.h"
#include "cpu.h"

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
        COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, tmp);
    else if(inst)
        COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA_INST, tmp);
    else if(data)
        COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA_DATA, tmp);
}


/*
 * CACHES 
 */

/* XXX: this can be simplified a lot! /AV */
static inline signed int log_2_n_round_up(uint32_t n)
{
    signed int log2n = -1;
    uint32_t temp = n;
    
    while (temp) {
        log2n++;
        temp >>= 1;
    }
    
    if (n & (n - 1))
        return log2n + 1; /* not power of 2 - round up */
    else
        return log2n; /* power of 2 */
}

static void setway(uint32_t level, uint32_t clean_it)
{
    uint32_t csselr, ccsidr; // cache size selection/id register
    uint32_t num_sets, num_ways, log2_line_len, log2_num_ways;
    uint32_t way_shift;
    
    //Write level and type you want to extract from cache size selection register
    csselr = level << 1 | ARMV7_CSSELR_IND_DATA_UNIFIED ;
    COP_WRITE2(COP_SYSTEM, "2", COP_ID_CPU, csselr);
    
    // Get size details from current cache size id register
    COP_READ2(COP_SYSTEM, "1", COP_ID_CPU, ccsidr);
    
    
    log2_line_len = (ccsidr & CCSIDR_LINE_SIZE_MASK) + 2;
    /* Converting from words to bytes */
    log2_line_len += 2;
    
    num_ways  = ((ccsidr & CCSIDR_ASSOCIATIVITY_MASK) >>
                 CCSIDR_ASSOCIATIVITY_OFFSET) + 1;
    num_sets  = ((ccsidr & CCSIDR_NUM_SETS_MASK) >>
                 CCSIDR_NUM_SETS_OFFSET) + 1;
    
    log2_num_ways = log_2_n_round_up(num_ways);
    
    way_shift = (32 - log2_num_ways);
    
    int way, set, setway;
    
    for (way = num_ways - 1; way >= 0 ; way--) {
        for (set = num_sets - 1; set >= 0; set--) {
            setway = (level << 1) | (set << log2_line_len) |
                  (way << way_shift);
            
            if(clean_it)
                COP_WRITE(COP_SYSTEM, COP_DCACHE_CLEAN_INVALIDATE_SW, setway);
            else
                COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_SW, setway);
            
        }
    }
    
    /* Data synchronization barrier operation
     *  to make sure the operation is complete 
     */
    dsb();
}

// Viktor
static void mem_icache_invalidate(BOOL clean_it)
{
    uint32_t level, cache_type, level_start_bit = 0;
    uint32_t clidr;
    
    /* Get cache level ID register */
    COP_READ2(COP_SYSTEM, "1", COP_ID_CACHE_CONTROL_ARMv7_CLIDR, clidr);
    
    for (level = 0; level < 8; level++) {
        cache_type = (clidr >> level_start_bit) & 0x7;
        
        if ((cache_type == ARMV7_CLIDR_CTYPE_DATA_ONLY) ||
            (cache_type == ARMV7_CLIDR_CTYPE_INSTRUCTION_DATA) ||
            (cache_type == ARMV7_CLIDR_CTYPE_UNIFIED))
        {
            setway(level, clean_it);
        }
        
        level_start_bit += 3;
    }    
}

void mem_cache_set_enable(BOOL enable)
{
    uint32_t tmp;
    
    COP_READ( COP_SYSTEM, COP_SYSTEM_CONTROL, tmp);
    if(enable)
        tmp |= (COP_SYSTEM_CONTROL_ICACHE_ENABLE | COP_SYSTEM_CONTROL_DCACHE_ENABLE);
    else
        tmp &=~(COP_SYSTEM_CONTROL_ICACHE_ENABLE | COP_SYSTEM_CONTROL_DCACHE_ENABLE);
    COP_WRITE( COP_SYSTEM, COP_SYSTEM_CONTROL, tmp);            
}

void mem_cache_invalidate(BOOL inst_inv, BOOL data_inv, BOOL data_writeback)
{
    uint32_t tmp = 1;
    /* First, handle the data cache. */
    if(data_inv) {
        mem_icache_invalidate(data_writeback);
    }    
    
    /* Then, the instruction cache. */    
    if(inst_inv) {
        COP_WRITE(COP_SYSTEM, COP_ICACHE_INVALIDATE_ALL, tmp);
    }
}

void mem_cache_dcache_area(addr_t va, uint32_t size, uint32_t op)
{
	uint32_t csidr;
	uint32_t cache_size;

	uint32_t end, next;
	COP_READ2(COP_SYSTEM,"1", COP_ID_CPU, csidr);
	csidr &= 7; 	 // Cache line size encoding
	cache_size = 16; //Size offset
	cache_size = (cache_size << csidr);

	next = va;
	end = va + size;
	do {
		if(op == FLUSH)
			COP_WRITE(COP_SYSTEM, COP_DCACHE_CLEAN_INVALIDATE_MVA, next );
		else if(op == CLEAN)
			COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, next );
		next += cache_size;

	} while(next < end );

	dsb();
}

