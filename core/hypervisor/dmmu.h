
#ifndef _DMMU_H_
#define _DMMU_H_

/* bft base and size definition */
#define DMMU_BFT_BASE_PY  (MB + HAL_PHYS_START)
#define DMMU_BFT_BASE_VA  (DMMU_BFT_BASE_PY - HAL_OFFSET)

#define DMMU_BFT_COUNT (1024 * 1024)
#define DMMU_BFT_SIZE  (DMMU_BFT_COUNT * sizeof(dmmu_entry_t))

#define __PACKED __attribute__ ((packed))


/* bft entry type */
enum dmmu_entry_type { 
    DMMU_TYPE_DATA, DMMU_TYPE_L1PT, DMMU_TYPE_L2PT, DMMU_TYPE_INVALID
};

/* a single bft table */
typedef union dmmu_entry {
    uint32_t all;
    __PACKED struct {
        uint32_t refcnt : 30;
        uint32_t type : 2;
    };
} dmmu_entry_t;


typedef __PACKED struct l1_pt
{
  uint32_t typ          : 2;
  uint32_t pxn          : 1;
  uint32_t ns           : 1;
  uint32_t sbz          : 1;
  uint32_t dom          : 4;
  uint32_t dummy        : 1;
  uint32_t addr         : 22;
} l1_pt_t;

typedef __PACKED struct l1_sec
{
  uint32_t pxn          : 1;
  uint32_t typ          : 1;
  uint32_t b            : 1;
  uint32_t c            : 1;
  uint32_t xn           : 1;
  uint32_t dom          : 4;
  uint32_t dummy        : 1;
  uint32_t ap_0_1bs     : 2;
  uint32_t tex          : 3;
  uint32_t ap_3b        : 1;
  uint32_t s            : 1;
  uint32_t ng           : 1;
  uint32_t secIndic     : 1;
  uint32_t ns           : 1;
  uint32_t addr         : 12;
} l1_sec_t;


/* in tranelate.c */
int mmu_lookup_guest(addr_t vadr, addr_t *padr, int user_write);
int mmu_lookup_hv(addr_t vadr, addr_t *padr, int hv_write);

#define DESC_TYPE_MASK 0b11
#define UNMAPPED_ENTRY 0

#define L1_SEC_DESC_MASK 0xFFF00000
#define L1_SEC_DESC_ATTR_MASK 0x000BFFFC

#define VA_TO_L1_IDX(va) (va >> 20)
#define L1_IDX_TO_PA(l1_base, idx) (l1_base | (idx << 2))

#define L1_TYPE(l1_desc) (l1_desc & DESC_TYPE_MASK)

#define UNMAP_L1_ENTRY(l1_desc) (l1_desc && 0b00)
#define CREATE_L1_SEC_DESC(x, y) (L1_SEC_DESC_MASK & x) || (L1_SEC_DESC_ATTR_MASK & y) || (0b10)
#define GET_L1_AP(sec) (((sec->ap_3b) << 2) | (sec->ap_0_1bs))

#define START_PA_OF_SECTION(sec) ((sec->addr) << 20)
#define PA_OF_POINTED_PT(pt) ((pt->addr) << 10)

#define SECTION_SIZE (0x00100000)

#define PA_TO_PH_BLOCK(pa) (pa >> 12)

#define PAGE_INFO_TYPE_DATA 0
#define PAGE_INFO_TYPE_L1PT 1
#define PAGE_INFO_TYPE_L2PT 2
#define PAGE_INFO_TYPE_INVALID 3

#endif /* _DMMU_H_ */
