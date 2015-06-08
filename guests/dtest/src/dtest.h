
#ifndef _DTEST_H_
#define _DTEST_H_

#include "hypercalls.h"
#include "print_err.h"

#define SECTION_SIZE (0x00100000)
#define PAGE_SIZE (0x00001000)

enum mmu_ap { MMU_AP_NONE = 0, MMU_AP_SUP_RW, MMU_AP_USER_RO, MMU_AP_USER_RW };
#define MMU_SECTION_AP_SHIFT 10
#define MMU_PT_AP_SHIFT 4
#define HC_DOM_DEFAULT 	0
#define HC_DOM_KERNEL 	1
#define HC_DOM_TASK 	2
#define HC_DOM_TRUSTED 	3
#define MMU_L1_DOMAIN_SHIFT 5


uint32_t l1[4096] __attribute__ ((aligned (16 * 1024)));
uint32_t l2[1024] __attribute__ ((aligned (4 * 1024)));

enum dmmu_command
{
  CMD_MAP_L1_SECTION, CMD_UNMAP_L1_PT_ENTRY, CMD_CREATE_L2_PT, CMD_MAP_L1_PT, CMD_MAP_L2_ENTRY, CMD_UNMAP_L2_ENTRY, CMD_FREE_L2, CMD_CREATE_L1_PT, CMD_SWITCH_ACTIVE_L1, CMD_FREE_L1
};


extern uint32_t syscall_dmmu(uint32_t r0, uint32_t r1, uint32_t r2);
#define ISSUE_DMMU_HYPERCALL(type, p0, p1, p2) \
		syscall_dmmu(type | (p2 << 4), p0, p1);

#define ISSUE_DMMU_HYPERCALL_(type, p0, p1, p2, p3) \
		syscall_dmmu((type | (p2 & 0xFFFFFFF0)), p0, ((p1 << 20) | p3));


addr_t pstart;
addr_t vstart;
size_t psize;
size_t fwsize;


uint32_t va_base;


uint32_t va2pa(uint32_t va) {
	return va - vstart + pstart;
}

#endif /* _DTEST_H_ */
