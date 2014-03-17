
#ifndef _ARM_MMU_H_
#define _ARM_MMU_H_


/*
 * MMU definitions for ARMv5-7
 */

/* defines */
#define PAGE_BITS 12
#define PAGE_SIZE (1UL << PAGE_BITS)
#define PAGE_MASK (PAGE_SIZE-1) /*0xFFF*/


/* acces permissions */
enum mmu_ap { MMU_AP_NONE = 0, MMU_AP_SUP_RW, MMU_AP_USER_RO, MMU_AP_USER_RW };
enum mmu_flags { MMU_FLAG_C = 1UL << 3, MMU_FLAG_B = 1UL << 2};

#define MMU_FLAG_B (1UL << 2)
#define MMU_FLAG_C (1UL << 3)

/* MMU directory */
#define MMU_L1_SIZE (1024 * 1024)
#define MMU_L1_COUNT (1024 * 4)
#define MMU_L1_INDEX(adr) (adr >> 20)
#define MMU_L1_ADR(adr) ((adr) & (MMU_L1_SIZE-1))

#define MMU_L1_TYPE(val)  ((val) & 3)
#define MMU_L1_TYPE_FAULT 0
#define MMU_L1_TYPE_FINE  3
#define MMU_L1_TYPE_COARSE 1
#define MMU_L1_TYPE_SMALL  1
#define MMU_L1_TYPE_SECTION 2

#define MMU_L1_SECTION_ADR(adr)  ((adr) & 0xFFF00000)
#define MMU_L1_FINE_ADR(adr)     ((adr) & 0xFFFFF000)
#define MMU_L1_COARSE_ADR(adr)   ((adr) & 0xFFFFFC00)

#define MMU_L1_DOMAIN_MASK  (0xF << 5)
#define MMU_L1_DOMAIN_SHIFT 5


/* MMU section descriptor */

#define MMU_SECTION_AP_MASK 3
#define MMU_SECTION_AP_SHIFT 10

#define MMU_L1_SECTION_SHIFT 20
#define MMU_L1_SECTION_SIZE (1UL << MMU_L1_SECTION_SHIFT)
#define MMU_L1_SECTION_ADDR(addr) ((addr) & ~(MMU_L1_SECTION_SIZE - 1))
#define MMU_L1_SECTION_MASK (~(MMU_L1_SECTION_SIZE -1))

/*Page table descriptor (Coarse in v5)*/
#define MMU_L1_PT_SHIFT 10
#define MMU_L1_PT_SIZE (1UL << MMU_L1_PT_SHIFT)
#define MMU_L1_PT_ADDR(addr) ((addr) & ~(MMU_L1_PT_SIZE -1))

#define MMU_L2_SMALL_SHIFT 12
#define MMU_L2_SMALL_SIZE (1UL << MMU_L2_SMALL_SHIFT)
#define MMU_L2_SMALL_ADDR(addr) ((addr) & ~(MMU_L2_SMALL_SIZE -1))

#define MMU_L2_SMALL_AP_SHIFT 4
#define MMU_L2_SMALL_AP_MASK 3

/* MMU coarse tables (small pages, 4K) */
/*These are not correct, coarse address are 31-10 bits*/
#define MMU_COARSE_SIZE  4096
#define MMU_COARSE_COUNT 256
#define MMU_COARSE_ADR(adr)  ((adr) & ~( MMU_COARSE_SIZE -1))
#define MMU_COARSE_INDEX(adr) (((adr) >> 12) & (MMU_COARSE_COUNT-1))

#define MMU_COARSE_TYPE(val)  ((val) & 3)
#define MMU_COARSE_TYPE_SMALL 2
#define MMU_COARSE_TYPE_FAULT 0

#define MMU_COARSE_AP0_MASK 3
#define MMU_COARSE_AP1_MASK 3
#define MMU_COARSE_AP2_MASK 3
#define MMU_COARSE_AP3_MASK 3
#define MMU_COARSE_AP0_SHIFT 4
#define MMU_COARSE_AP1_SHIFT 6
#define MMU_COARSE_AP2_SHIFT 8
#define MMU_COARSE_AP3_SHIFT 10

/* TLB */

#define TTB_RGN_OC_WB	(3 << 3)
#define TTB_IRGN_WB	((1 << 0) | (1 << 6))

/* memspace flags */
#define MEMSPACE_FLAGS_DOMAIN_MASK 15
#define MEMSPACE_FLAGS_DOMAIN_SHIFT 0

#define MEMSPACE_FLAGS_AP_MASK 3
#define MEMSPACE_FLAGS_AP_SHIFT 5


uint32_t pt_create_coarse(addr_t *pt, addr_t va,addr_t pa, uint32_t size, uint32_t mem_type);
BOOL pt_create_section( addr_t *pt, addr_t va, addr_t pa, uint32_t mem_type);
BOOL pt_map(addr_t va, addr_t pa, uint32_t size, uint32_t mem_type);

#endif /* _ARM_MMU_H_ */
