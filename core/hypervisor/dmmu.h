
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


/* in tranelate.c */
int mmu_lookup_guest(addr_t vadr, addr_t *padr, int user_write);
int mmu_lookup_hv(addr_t vadr, addr_t *padr, int hv_write);

#endif /* _DMMU_H_ */
