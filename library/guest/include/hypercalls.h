
#ifndef GUEST_HYPERCALLS_H_
#define GUEST_HYPERCALLS_H_

/* these must match core/hypervisor/hypercalls.h */
#define STR(x) #x
#define HYPERCALL_NUM(n) "#"STR(n)

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


#endif /* GUEST_HYPERCALLS_H_ */
