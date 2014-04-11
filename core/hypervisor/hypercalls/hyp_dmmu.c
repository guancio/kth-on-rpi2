
#include "hyper.h"
#include "dmmu.h"
#include "mmu.h"

extern virtual_machine *curr_vm;

#define DEBUG_MMU

/*Get physical address from Linux virtual address*/
#define LINUX_PA(va) ((va) - (curr_vm->config->firmware->vstart) + (curr_vm->config->firmware->pstart))

void hypercall_dyn_switch_mm(addr_t table_base, uint32_t context_id)
{
#ifdef DEBUG_MMU
	printf("\n\t\t\tHypercall switch PGD\n\t\t table_base:%x ", table_base);
#endif

	/*Switch the TTB and set context ID*/
	if(dmmu_switch_mm(table_base & L1_BASE_MASK))
		printf("\n\tCould not switch MM\n");
	COP_WRITE(COP_SYSTEM,COP_CONTEXT_ID_REGISTER, context_id); //Set context ID
	isb();

}


/*New pages for processes, copys kernel space from master pages table
 *and cleans the cache, set these pages read only for user */
void hypercall_dyn_new_pgd(addr_t *pgd_va)
{
#ifdef DEBUG_MMU
	printf("\n\t\t\tHypercall new PGD\n\t\t pgd:%x ", pgd_va);
#endif

	/*If the requested page is in a section page, we need to modify it to lvl 2 pages
	 *so we can modify the access control granularity */

	addr_t *master_pgd_va;
	addr_t phys_start = curr_vm->config->firmware->pstart;
	addr_t page_offset = curr_vm->guest_info.page_offset;
	addr_t linux_va;
	/*Get master page table*/
	master_pgd_va = (addr_t *)(curr_vm->config->pa_initial_l1_offset + page_offset);
	addr_t *l1_pt_entry_for_desc = (addr_t *)&master_pgd_va[(addr_t)pgd_va >> MMU_L1_SECTION_SHIFT];
	uint32_t l1_desc_entry = *l1_pt_entry_for_desc;

	if(l1_desc_entry & MMU_L1_TYPE_SECTION) { /*Section page, replace it with lvl 2 pages*/
		linux_va = MMU_L1_SECTION_ADDR(l1_desc_entry) - phys_start + page_offset;
    	COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, linux_va);
		COP_WRITE(COP_SYSTEM, COP_BRANCH_PRED_INVAL_ALL, linux_va);
		dsb();
		isb();

		/*Clear the SECTION entry mapping and replace it with a L2PT */
		if(dmmu_unmap_L1_pageTable_entry((addr_t)linux_va))
			printf("\n\tCould not unmap L1 entry in new pgd\n");
		uint32_t table2_pa = linux_pt_get_empty_l2(); /*pointer to private L2PTs in guest*/

		/*Small page with cache and buffer RW*/
		uint32_t attrs = MMU_L1_TYPE_PT;
        attrs |= (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
		if(dmmu_l1_pt_map((addr_t)linux_va, table2_pa, attrs))
			printf("\n\tCould not map L1PT in new PGD\n");

		/*Remap each individual small page to the same address*/
        uint32_t page_pa = MMU_L1_SECTION_ADDR(l1_desc_entry);
        /*Small page with CB on and RW*/
        attrs = MMU_L2_TYPE_SMALL;
        attrs |= (MMU_FLAG_B | MMU_FLAG_C);
        attrs |= MMU_AP_USER_RW <<  MMU_L2_SMALL_AP_SHIFT ;
        int i, end, table2_idx ;

        /*Get index of physical L2PT */
        table2_idx = (table2_pa - (table2_pa & L2_BASE_MASK)) >> MMU_L1_PT_SHIFT;
        table2_idx *= 0x100; /*256 pages per L2PT*/
        end = table2_idx + 0x100;

    	/*Get the index of the page entry to make read only*/
    	uint32_t l2_entry_idx = (((uint32_t)pgd_va << 12) >> 24) + table2_idx;

        for(i = table2_idx; i < end;i++, page_pa+=0x1000){
        	if(i >= l2_entry_idx && i < l2_entry_idx + 4){ /*4 small pages RO 16KB PGD*/
        		uint32_t ro_attrs = 0xE | (MMU_AP_USER_RO <<  MMU_L2_SMALL_AP_SHIFT);
        		if(dmmu_l2_map_entry(table2_pa, i, page_pa,  ro_attrs))
        			printf("\n\tCould not map L2 entry in new pgd\n");
        	}
        	else
        		if(dmmu_l2_map_entry(table2_pa, i, page_pa,  attrs))
        			printf("\n\tCould not map L2 entry in new pgd\n");
        }

		/*Invalidate updated entry*/
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA,l1_pt_entry_for_desc);
		dsb();
	}
	/*Here we already got a L2PT*/
	else{
		//TODO
		printf("\n\tNot implemented!\n");
	}


	/* Page table 0x0 - 0x4000
		 * Reset user space 0-0x2fc0
		 * 0x2fc0 = 0xBF000000 END OF USER SPACE
		 * Copy kernel, IO and hypervisor mappings
		 * 0x2fc0 - 0x4000
		 * */
	memset((void *)pgd_va, 0, 0x2fc0);
	memcpy((void *)((uint32_t)pgd_va + 0x2fC0), (uint32_t *)((uint32_t)(master_pgd_va) + 0x2fc0), 0x1040);

	/*Clean dcache on whole table*/
	hypercall_dcache_clean_area((uint32_t)pgd_va, 0x4000);
	if(dmmu_create_L1_pt(LINUX_PA((addr_t)pgd_va)))
		printf("\n\tCould not create L1 pt in new pgd\n");

}


/*In ARM linux pmd refers to pgd, ARM L1 Page table
 *Linux maps 2 pmds at a time  */
void hypercall_dyn_set_pmd(addr_t *pmd, uint32_t desc)
{
#ifdef DEBUG_MMU
	printf("\n\t\t\tHypercall set PMD\n\t\t pmd:%x val:%x ", pmd, desc);
#endif

	addr_t l1_entry, *l1_pt_entry_for_desc;
	addr_t curr_pgd_pa, *pgd_va, attrs;
	uint32_t l1_pt_idx_for_desc, l1_desc_entry, phys_start;

	phys_start = curr_vm->config->firmware->pstart;
	addr_t page_offset = curr_vm->guest_info.page_offset;
	uint32_t page_offset_idx = (page_offset >> 20) *4;

	addr_t master_pgd_va = (curr_vm->config->pa_initial_l1_offset + page_offset);

	/*Page attributes*/
	uint32_t l2_rw_attrs = MMU_L2_TYPE_SMALL;
    l2_rw_attrs |= (MMU_FLAG_B | MMU_FLAG_C);
    l2_rw_attrs |= MMU_AP_USER_RW <<  MMU_L2_SMALL_AP_SHIFT ;

	/*Get page table for pmd*/
	pgd_va = (addr_t ) (pmd ) & L1_BASE_MASK; /*Mask with 16KB alignment to get PGD*/


	/*Page table entry to set, if the desc is 0 we have to
	 * get it from the pgd*/
	if(desc != 0)
		l1_entry = desc;
	else
		l1_entry = *pmd;

	l1_pt_idx_for_desc = ((l1_entry - phys_start) >> MMU_L1_SECTION_SHIFT)*4;
	l1_pt_entry_for_desc =  (addr_t *)((addr_t)pgd_va + l1_pt_idx_for_desc + page_offset_idx);
	l1_desc_entry = *l1_pt_entry_for_desc;

	addr_t *linux_va = MMU_L2_SMALL_ADDR(l1_entry) - phys_start + page_offset;

	if(l1_desc_entry & MMU_L1_TYPE_SECTION) { /*SECTION page, replace with lvl2 pages */
		/*Clear the SECTION entry mapping and replace it with a L2PT */
		if(dmmu_unmap_L1_pageTable_entry((addr_t)linux_va))
			printf("\n\tCould not unmap L1 entry in set PMD\n");
		uint32_t table2_pa = linux_pt_get_empty_l2(); /*pointer to private L2PTs in guest*/

		/*Small page with cache and buffer RW*/
		attrs = MMU_L1_TYPE_PT;
        attrs |= (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
		if(dmmu_l1_pt_map((addr_t)linux_va, table2_pa, attrs))
			printf("\n\tCould not map L1 PT in set PMD\n");

		/*Remap each individual small page to the same address*/
        uint32_t page_pa = MMU_L1_SECTION_ADDR(l1_desc_entry);
        /*Small page with CB on and RW*/

        int i, end, table2_idx ;
        /*This is the idx of the physical address that needs to be RO future new (L2PT)*/
        uint32_t l2_idx = ((uint32_t)l1_entry << 12) >> 24;

        /*Get index of physical L2PT */
        table2_idx = (table2_pa - (table2_pa & L2_BASE_MASK)) >> MMU_L1_PT_SHIFT;
        table2_idx *= 0x100; /*256 pages per L2PT*/
        end = table2_idx + 0x100;

        for(i = table2_idx; i < end;i++, page_pa+=0x1000){
        	if(i == l2_idx +( table2_idx)){
        		uint32_t ro_attrs = 0xE | (MMU_AP_USER_RO <<  MMU_L2_SMALL_AP_SHIFT);
        		if(dmmu_l2_map_entry(table2_pa, i, page_pa,  ro_attrs))
        			printf("\n\tCould not map L2 entry in set PMD\n");
        	}
        	else
        		if(dmmu_l2_map_entry(table2_pa, i, page_pa,  l2_rw_attrs))
        			printf("\n\tCould not map L2 entry in set PMD\n");
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

	uint32_t desc_va = MMU_L2_SMALL_ADDR(l1_entry) - phys_start + page_offset;
	uint32_t desc_va_idx = MMU_L1_SECTION_IDX(desc_va);

	uint32_t l2pt_pa = MMU_L1_PT_ADDR(pgd_va[desc_va_idx]);
	uint32_t *l2pt_va = (addr_t *)(l2pt_pa- phys_start + page_offset);
	uint32_t l2_idx = ((uint32_t)l1_entry << 12) >> 24;
	uint32_t l2entry_desc = l2pt_va[l2_idx];

    /*Get index of physical L2PT */
     uint32_t table2_idx = (l2pt_pa - (l2pt_pa & L2_BASE_MASK)) >> MMU_L1_PT_SHIFT;
     table2_idx *= 0x100; /*256 pages per L2PT*/

     /*If page entry for L2PT is RW, unmap it and make it RO so we can create a L2PT*/
	if(((l2entry_desc >> 4) & 0xff) == 3 ){
		if(dmmu_l2_unmap_entry((uint32_t)l2pt_pa & L2_BASE_MASK, table2_idx+l2_idx))
			printf("\n\tCould not unmap L2 entry in set PMD\n");
		uint32_t desc_pa = MMU_L2_SMALL_ADDR(desc);
		uint32_t ro_attrs = 0xE | (MMU_AP_USER_RO <<  MMU_L2_SMALL_AP_SHIFT);

		if(dmmu_l2_map_entry((uint32_t)l2pt_pa & L2_BASE_MASK, table2_idx+l2_idx, desc_pa,  ro_attrs))
			printf("\n\tCould not map L2 entry in set PMD\n");
	}
	if( desc != 0 ){
		if(dmmu_create_L2_pt(MMU_L2_SMALL_ADDR(desc)))
			printf("\n\tCould not create L2PT in set pmd\n");
	}

	attrs = desc & 0x3FF; /*Mask out addresss*/

	/*Get virtual address of the translation for pmd*/
	addr_t virt_transl_for_pmd = (addr_t)((pmd - pgd_va) << MMU_L1_SECTION_SHIFT);

	/*Get current page table*/
	COP_READ(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, (uint32_t)curr_pgd_pa);

	uint32_t switch_back = 0;

	if((LINUX_PA((addr_t)pmd & L1_BASE_MASK)) != (curr_pgd_pa)){
		/*This means that we are setting a pmd on another pgd, current
		 * API does not allow that, so we have to switch the physical ttb0
		 * back and forth */
			COP_WRITE(COP_SYSTEM,COP_SYSTEM_TRANSLATION_TABLE0, (addr_t)pmd - page_offset + phys_start); // Set TTB0
			isb();
			switch_back = 1;
	}

	if(desc == 0){
		/*Before unmapping the L1 entry, we need to make it RW again so that
		 *OS can reuse the address */
		if(dmmu_l2_unmap_entry((uint32_t)l2pt_pa & L2_BASE_MASK, table2_idx+l2_idx))
					printf("\n\tCould not unmap L2 entry in set PMD\n");
		/*TODO Unmap L2 does not work*/
		if(dmmu_unmap_L2_pt(MMU_L2_SMALL_ADDR((uint32_t)*pmd)))
			printf("\n\tCould not unmap L2 pt in set PMD\n");
		if(dmmu_l2_map_entry((uint32_t)l2pt_pa & L2_BASE_MASK, table2_idx+l2_idx, MMU_L2_SMALL_ADDR((uint32_t)*pmd),  l2_rw_attrs))
					printf("\n\tCould not map L2 entry in set PMD\n");


		if(dmmu_unmap_L1_pageTable_entry(virt_transl_for_pmd))
			printf("\n\tCould not unmap L1 entry in set PMD\n");
		if(dmmu_unmap_L1_pageTable_entry(virt_transl_for_pmd+SECTION_SIZE))
			printf("\n\tCould not unmap L1 entry in set PMD\n");
	}
	else{

		if(dmmu_l1_pt_map(virt_transl_for_pmd, MMU_L2_SMALL_ADDR(desc), attrs))
			printf("\n\tCould not map L1 PT in set PMD\n");
		if(dmmu_l1_pt_map(virt_transl_for_pmd + SECTION_SIZE, MMU_L2_SMALL_ADDR(desc) + 0x400, attrs))
			printf("\n\tCould not map L1 PT in set PMD\n");
	}
	if(switch_back){
		COP_WRITE(COP_SYSTEM,COP_SYSTEM_TRANSLATION_TABLE0, curr_pgd_pa); // Set TTB0
		isb();
	}

	/*Flush entry*/
	COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, (uint32_t)pmd);
	dsb();

}

/*va is the virtual address of the page table entry for linux pages
 *the physical pages are located 0x800 below */
hypercall_dyn_set_pte(addr_t *l2pt_linux_entry_va, uint32_t linux_pte, uint32_t phys_pte)
{
#ifdef DEBUG_MMU
	printf("\n\t\t\tHypercall set PTE\n\t\t va:%x linux_pte:%x phys_pte:%x ", l2pt_linux_entry_va, phys_pte, linux_pte);
#endif
	addr_t phys_start = curr_vm->config->firmware->pstart;
	uint32_t page_offset = curr_vm->guest_info.page_offset;

	uint32_t *l2pt_hw_entry_va = (addr_t *)((addr_t ) l2pt_linux_entry_va - 0x800);
	addr_t l2pt_hw_entry_pa = ((addr_t)l2pt_hw_entry_va - page_offset + phys_start );

    /*Get index of physical L2PT */
    uint32_t entry_idx = ((addr_t )l2pt_hw_entry_va & 0xFFF) >> 2;

	/*Small page with CB on and RW*/
    uint32_t attrs = phys_pte & 0xFFF; /*Mask out address*/

    if(phys_pte != 0) {
    	if(dmmu_l2_map_entry(l2pt_hw_entry_pa & L2_BASE_MASK, entry_idx, MMU_L1_PT_ADDR(phys_pte),attrs))
    		printf("\n\tCould not map l2 entry in set pte hypercall\n");
    	/*Cannot map linux entry, ap = 0 generates error*/
    	//dmmu_l2_map_entry(l2pt_hw_entry_pa & L2_BASE_MASK, entry_idx + (256*2), MMU_L1_PT_ADDR(phys_pte),linux_pte & 0xFFF);
    }
    else {
    	/*Unmap*/
    	if(dmmu_l2_unmap_entry(l2pt_hw_entry_pa & L2_BASE_MASK, entry_idx))
    		printf("\n\tCould not unmap l2 entry in set pte hypercall\n");
    }
	/*Do we need to use the DMMU API to set Linux pages?*/
	*l2pt_linux_entry_va = linux_pte;

	COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, (uint32_t)l2pt_hw_entry_va );

}

