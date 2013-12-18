#include "hw.h"
#include "hyper.h"

/*Cache and TLB operations*/


/*D Cache operations*/
void hypercall_dcache_flush_area(addr_t va, uint32_t size)
{
	mem_cache_dcache_area(va, size, FLUSH );
}

void hypercall_dcache_clean_area(addr_t va, uint32_t size)
{
	mem_cache_dcache_area(va, size, CLEAN );

}

void hypercall_dcache_invalidate_mva(addr_t va)
{

	COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, va );
	dsb();

}

/*I Cache operations*/
void hypercall_icache_invalidate_all()
{
	COP_WRITE(COP_SYSTEM, COP_ICACHE_INVALIDATE_ALL, 0);
}

/*Not implemented, nothing that uses it yet*/
void hypercall_icache_invalidate_mva(addr_t va)
{
	hyper_panic("Not implemented!", 1);
}


void hypercall_branch_invalidate_all()
{
	COP_WRITE(COP_SYSTEM, COP_BRANCH_PRED_INVAL_ALL, 0);
}


void hypercall_flush_all()
{
	mem_mmu_tlb_invalidate_all(TRUE, TRUE);
	mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
}


/* TLB Operations */

void hypercall_tlb_invalidate_mva(addr_t va)
{
	COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, va);
}

void hypercall_tlb_invalidate_asid(uint32_t asid)
{
	COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ASID, asid);

}

void hypercall_cache_op(enum hyp_cache_op op, addr_t va, uint32_t size)
{
	switch (op){
		case FLUSH_ALL:
			hypercall_flush_all();
			return;
		case FLUSH_D_CACHE_AREA:
			hypercall_dcache_flush_area(va, size);
			return;
		case CLEAN_D_CACHE_AREA:
			hypercall_dcache_clean_area(va, size);
			return;
		case INVAL_D_CACHE_MVA:
			hypercall_dcache_invalidate_mva(va);
			return;
		case FLUSH_I_CACHE_ALL:
			hypercall_icache_invalidate_all();
			return;
		case FLUSH_I_CACHE_MVA:
			hypercall_icache_invalidate_mva(va);
			return;
		case INVAL_ALL_BRANCH:
			hypercall_branch_invalidate_all();
			return;
		case INVAL_TLB_ALL:
			COP_WRITE(COP_SYSTEM,COP_TLB_INVALIDATE_ALL, 0);
			return;
		case INVAL_TLB_MVA:
			hypercall_tlb_invalidate_mva(va);
			return;
		case INVAL_TLB_ASID:
			hypercall_tlb_invalidate_asid(va);
			return;
		default:
			printf("Invalid cache operation\n");
	}
}

