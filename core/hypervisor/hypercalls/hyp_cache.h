#ifndef CACHE_H_
#define CACHE_H_

/*************************************************
 * Enums used to specify hypercall operations
 * **********************************************/

/*HYPERCALL_CACHE_OP
 * */
enum hyp_cache_op {
	FLUSH_ALL 			=0,
	FLUSH_D_CACHE_AREA	=1,
	CLEAN_D_CACHE_AREA	=2,
	INVAL_D_CACHE_MVA	=3,
	FLUSH_I_CACHE_ALL	=4,
	FLUSH_I_CACHE_MVA	=5,
	INVAL_ALL_BRANCH	=6,
	INVAL_TLB_ALL		=7,
	INVAL_TLB_MVA		=8,
	INVAL_TLB_ASID		=9,
};

void hypercall_dcache_clean_area(addr_t va, uint32_t size);
void hypercall_cache_op(enum hyp_cache_op op, addr_t va, uint32_t size);
void hypercall_flush_dcache_area(uint32_t reg0, uint32_t reg1);
void hypercall_inval_tlb_mva(uint32_t uaddr);
void hypercall_inval_all_branch();
void hypercall_clean_dcache_area(uint32_t reg0, uint32_t reg1);
void hypercall_flush_entry(uint32_t entry);
void hypercall_inval_icache();


#endif /* CACHE_H_ */
