
/*
 * implementation of the PT code for ARM
 */

#include <hw.h>
#include <mmu.h>
#include <utillib.h>
#include "hyper_config.h"

#include "soc_defs.h" //Maybe put IO_VA_ADDRESS() macro somewhere else?

#define GET_VIRT_ARRAY(phy)  ((uint32_t *) GET_VIRT(phy))

/* for 1MB pages */
#define MEG_BITS 20
#define MEG_SIZE (1UL << MEG_BITS)
#define MEG_MASK (MEG_SIZE - 1)


extern uint32_t __hyper_pt_start__;
extern uint32_t *slpt_va;

/*index 0 reserved for hypervisor
 *Each index holds 256 small pages (totalling 1 MB of space)
 * */
uint32_t l2_index_p = 0;

void map_section(void *page_dir, uint32_t va, uint32_t pa, BOOL cache)
{
	uint32_t page_attr = 0;
	if (cache)
		page_attr = (MMU_AP_SUP_RW << MMU_SECTION_AP_SHIFT) | MMU_FLAG_C | MMU_FLAG_B
					| MMU_L1_TYPE_SECTION;
	else
		page_attr = ((3 << MMU_SECTION_AP_SHIFT) | MMU_L1_TYPE_SECTION);
	uint32_t *p = (uint32_t*) page_dir;
	p[va >> 20] = (pa & 0xFFF00000) | page_attr;


}

addr_t pt_get_empty_l2()
{
	if((l2_index_p * 0x400) > 0x8000) // Set max size of L2 pages
		return 0;
	else{
		addr_t index = l2_index_p * 0x400;
		memset( (uint32_t *)((uint32_t)slpt_va + index), 0, 0x400);
		l2_index_p++;
		return (uint32_t)(GET_PHYS(slpt_va) + index);
	}

}

BOOL pt_map(addr_t va, addr_t pa, uint32_t size, uint32_t mem_type)
{

	if(!pt_create_coarse(&__hyper_pt_start__,va, pa, size, mem_type)){
		printf("Unable to create coarse page\n");
		return FALSE;
	}
	return TRUE;
}

BOOL pt_create_section( addr_t *l1, addr_t va, addr_t pa, uint32_t mem_type)
{
	uint32_t index = (va >> 20);
    uint32_t val = l1[index];
    uint32_t type = MMU_L1_TYPE( val);
    uint32_t domain, ap;


    if((va & ~(MMU_L1_SECTION_MASK)) == 0 && type == MMU_L1_TYPE_FAULT) {

        val = pa | 0x12; // 0b1--10
        val |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
        // if RAM, turn of XN and enable cache and buffer
        if(mem_type == MLT_USER_RAM){
            val = (val & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT)  ;
            l1[index] = val;
//            printf("CREATED section for USER%d, val = %x\n", index, val);
            return TRUE;
        }
        if(mem_type == MLT_TRUSTED_RAM){
            val = (val & (~0x10)) | 0xC | (HC_DOM_TRUSTED << MMU_L1_DOMAIN_SHIFT);
            l1[index] = val;
//            printf("CREATED section for TRUSTED%d, val = %x\n", index, val);
            return TRUE;
        }
        if(mem_type == MLT_HYPER_RAM){
            val = (val & (~0x10)) | 0xC | (HC_DOM_DEFAULT << MMU_L1_DOMAIN_SHIFT);
            l1[index] = val;
//          printf("CREATED section for HYPER%d, val = %x\n", index, val);
            return TRUE;
        }

    }
    printf("Could not allocate section, index=%d va adr=%x pa adr=%x type=%d\n", index, va,pa, type);
    return FALSE;

}

/*
 * functions below are used to build page table from data structure
 */
BOOL pt_create_coarse(addr_t *pt, addr_t va,addr_t pa, uint32_t size, uint32_t mem_type)
{
    uint32_t *table1 = pt;
    uint32_t index = MMU_L1_INDEX(va);
    uint32_t val = table1[index];
    uint32_t type_old = MMU_L1_TYPE( val);

    uint32_t domain, ap;
    uint32_t flags = MMU_FLAG_B | MMU_FLAG_C; /*Standard Cache and Buffer on*/
    switch(mem_type) {
    case MLT_TRUSTED_RAM:
    	domain = HC_DOM_TRUSTED;
        ap = MMU_AP_USER_RW;
        break;
    case MLT_IO_HYP_REG:
    	flags = 1; /*No cache or buffer in IO an XN = 1*/
    	domain = HC_DOM_DEFAULT;
    	ap = MMU_AP_SUP_RW;
    	break;
    case MLT_IO_RW_REG:
    	flags = 1;
        domain = HC_DOM_DEFAULT;
        ap = MMU_AP_USER_RW;
        break;
    case MLT_IO_RO_REG:
    	flags = 1;
        domain = HC_DOM_DEFAULT;
        ap = MMU_AP_USER_RO;
        break;
    case MLT_USER_RAM:
        domain = HC_DOM_KERNEL;
        ap = MMU_AP_USER_RW;
        break;
    case MLT_USER_ROM:
        domain = HC_DOM_DEFAULT;
        ap = MMU_AP_USER_RO;
    }
    uint32_t *table2, table2_pa;

    if(type_old == MMU_L1_TYPE_FAULT) {
    		/* allocate a new sub-page */
        	table2_pa = pt_get_empty_l2();
        		if(!table2_pa)  return FALSE;
        	table1[index] = ((uint32_t)(table2_pa) | (domain << MMU_L1_DOMAIN_SHIFT) | MMU_L1_TYPE_COARSE);
        }
    else {
    	/* There is already a mapping to the first level descriptor */
    	table2_pa = MMU_L1_PT_ADDR(table1[index]);
    }

    /*Coarse created, now hand out level 2 page tables for the coarse*/
    uint32_t pte;
    uint32_t count = (size >> 12);
    uint32_t slpt_index = ((va & 0x000FF000) >> 12);
    table2 = (uint32_t *)GET_VIRT(table2_pa);

    while(count){
    	pte = (pa | (ap << 4 ) | flags | MMU_COARSE_TYPE_SMALL);
    	table2[slpt_index] = pte;
    	slpt_index++;
    	pa += 0x1000;
    	count--;
    }
    return TRUE;
}
