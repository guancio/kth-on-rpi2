#ifndef HYPERCALLS_H_
#define HYPERCALLS_H_

#ifndef __ASSEMBLER__
	#include "hyper.h"
#endif

/*Function prototypes*/

void hyper_panic(char* msg, uint32_t exit_code);
void hypercall_num_error (uint32_t hypercall_num);

void change_guest_mode (uint32_t mode);

void hypercall_guest_init();
void hypercall_interrupt_set(uint32_t interrupt, uint32_t op);
void hypercall_end_interrupt ();

void hypercall_set_tls(uint32_t thread_id);

void hypercall_restore_linux_regs(uint32_t return_value, BOOL syscall);
void hypercall_restore_regs(uint32_t *regs);

#include "hyp_cache.h"
#include "hyp_mmu.h"

void hypercall_rpc(uint32_t rpc_op, void *arg);
void hypercall_end_rpc();
/***************************/

/**************
// ASM macros
**************/

#define ISSUE_HYPERCALL(num) \
	asm volatile ( \
		"SWI " HYPERCALL_NUM((num)) "         \n\t" \
	);

#define ISSUE_HYPERCALL_REG1(num, reg0) \
		asm volatile ("mov R0, %0 			\n\t"  	\
					  "SWI " HYPERCALL_NUM((num)) "\n\t" \
					  ::"r" (reg0) : "memory", "r0" \
);

#define ISSUE_HYPERCALL_REG2(num, reg0, reg1) \
		asm volatile ("mov R0, %0 			\n\t"  	\
					  "mov R1, %1			\n\t"   \
					  "SWI " HYPERCALL_NUM((num)) "\n\t" \
					  ::"r" (reg0), "r" (reg1) : "memory", "r0", "r1" \
);

/*
 * Hypercalls
 */

#define HYPERCALL_GUEST_INIT			1000

/*INTERRUPT */
#define HYPERCALL_INTERRUPT_SET     	1001
#define HYPERCALL_END_INTERRUPT     	1002
//#define HYPERCALL_REGISTER_HANDLER	1003

/*CACHE */
#define HYPERCALL_CACHE_OP				1004
#define HYPERCALL_TLB_OP				1005

/*PAGE TABLE*/
#define HYPERCALL_NEW_PGD				1006
#define HYPERCALL_FREE_PGD				1007
#define HYPERCALL_SWITCH_MM				1008
#define HYPERCALL_CREATE_SECTION    	1009
#define HYPERCALL_SET_PMD				1010
#define HYPERCALL_SET_PTE				1011

/*CONTEXT*/
#define HYPERCALL_RESTORE_LINUX_REGS 	1012
#define HYPERCALL_RESTORE_REGS			1013
#define HYPERCALL_SET_CTX_ID			1014
#define HYPERCALL_SET_TLS_ID			1015

/*RPC*/
#define HYPERCALL_RPC					1020
#define HYPERCALL_END_RPC				1021


/*NEW MEMORY MANAGEMENT*/
#define HYPERCALL_MMU_L1_UNMAP				1022
#define HYPERCALL_MMU_L1_SEC_MAP                        1023

/* WARNING: TODO, this stuff must be in a configuration file */
#define INITIAL_PT_FIXED_MAP_VA (0x00000000)
#define END_PT_FIXED_MAP_VA     (0x002FFFFF)

#define PA_TO_PH_BLOCK(pa) ((pa - (0x01000000 + HAL_PHYS_START)) >> 12)

#define ERR_HYP_RESERVED_VA (1)
#define ERR_HYP_ENTRY_UNMAPPED (2)
#define ERR_HYP_OUT_OF_RANGE_PA (3)
#define ERR_HYP_SECTION_NOT_UNMAPPED (4)
#define ERR_HYP_PH_BLOCK_NOT_WRITABLE (5)
#define ERR_HYP_AP_UNSUPPORTED (6)

#define PAGE_INFO_TYPE_DATA 0
#define PAGE_INFO_TYPE_L1PT 1
#define PAGE_INFO_TYPE_L2PT 2
#define PAGE_INFO_TYPE_INVALID 3

struct page_info {
    unsigned int refs : 30;
    unsigned int type : 2;
};


#endif /* HYPERCALLS_H_ */
