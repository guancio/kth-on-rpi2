
#include "hyper.h"
#include "dmmu.h"
#include "mmu.h"

extern virtual_machine *curr_vm;

/*In ARM linux pmd refers to pgd, ARM L1 Page table
 *Linux maps 2 pmds at a time  */
void hypercall_dyn_set_pmd(addr_t *pmd, uint32_t desc)
{
	addr_t l1_entry, *l1_pt_entry_for_desc;
	addr_t curr_pgd_pa, *curr_pgd_va, attrs;
	uint32_t l1_pt_idx_for_desc, l1_desc_entry, phys_start;

	phys_start = curr_vm->config->firmware->pstart;
	uint32_t page_offset = curr_vm->guest_info.page_offset;
	uint32_t page_offset_idx = (page_offset >> 20) *4;

	/*Get current page table*/
	COP_READ(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, (uint32_t)curr_pgd_pa);
	curr_pgd_va = (addr_t *) (curr_pgd_pa - phys_start + page_offset);

	/*Page table entry to set*/
	if(desc != 0)
		l1_entry = desc;
	else
		l1_entry = *pmd;

	l1_pt_idx_for_desc = ((l1_entry - phys_start) >> MMU_L1_SECTION_SHIFT)*4;
	l1_pt_entry_for_desc =  (addr_t *)((addr_t)curr_pgd_va + l1_pt_idx_for_desc + page_offset_idx);
	l1_desc_entry = *l1_pt_entry_for_desc;

	addr_t *linux_va = MMU_L2_SMALL_ADDR(l1_entry) - phys_start + page_offset;

	if(l1_desc_entry & MMU_L1_TYPE_SECTION) { /*SECTION page, replace with lvl2 pages */
		/*Clear the PT entry mapping and replace it with smaller pages*/
		dmmu_unmap_L1_pageTable_entry((addr_t)linux_va);
		uint32_t table2_pa = linux_pt_get_empty_l2(); /*pointer to private L2PTs in guest*/

		/*Small page with cache and buffer RW*/
		attrs = MMU_L1_TYPE_PT;
        attrs |= (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
		dmmu_l1_pt_map((addr_t)linux_va, table2_pa, attrs);

		/*Remap each individual small page to the same address*/
        uint32_t page_pa = MMU_L1_SECTION_ADDR(l1_desc_entry);
        /*Small page with CB on and RW*/
        attrs = MMU_L2_TYPE_SMALL;
        attrs |= (MMU_FLAG_B | MMU_FLAG_C);
        attrs |= MMU_AP_USER_RW <<  MMU_L2_SMALL_AP_SHIFT ;
        int i ;
        /*This is the idx of the physical address that needs to be RO future new (L2PT)*/
        uint32_t l2_idx = ((uint32_t)desc << 12) >> 24;
        for(i = 0; i < 256;i++, page_pa+=0x1000){
        	if(i == l2_idx){
        		uint32_t ro_attrs = 0xE | (MMU_AP_USER_RO <<  MMU_L2_SMALL_AP_SHIFT);
        		dmmu_l2_map_entry(table2_pa, i, page_pa,  ro_attrs);
        	}
        	else
        		dmmu_l2_map_entry(table2_pa, i, page_pa,  attrs);
        }

		/*Invalidate updated entry*/
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA,l1_pt_entry_for_desc);
		dsb();
		l1_desc_entry = *l1_pt_entry_for_desc;
	}

	COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, linux_va);
	COP_WRITE(COP_SYSTEM, COP_BRANCH_PRED_INVAL_ALL, linux_va);
	dsb();
	isb();

	/*We need to make sure the new L2 PT is unreferenced*/
	dmmu_create_L2_pt(MMU_L2_SMALL_ADDR(desc));


	attrs = MMU_L1_TYPE_PT;
	attrs |= (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	/*Get virtual address of the translation for pmd*/
	addr_t virt_transl_for_pmd = (addr_t)((pmd - curr_pgd_va) << MMU_L1_SECTION_SHIFT);

	if(desc == 0){
		dmmu_unmap_L1_pageTable_entry(virt_transl_for_pmd);
		dmmu_unmap_L1_pageTable_entry(virt_transl_for_pmd+SECTION_SIZE);
	}
	else{
		dmmu_l1_pt_map(virt_transl_for_pmd, MMU_L2_SMALL_ADDR(desc), attrs);
		dmmu_l1_pt_map(virt_transl_for_pmd + SECTION_SIZE, MMU_L2_SMALL_ADDR(desc) + 0x400, attrs);

	}

	/*Flush entry*/
	COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, (uint32_t)pmd);
	dsb();

}
