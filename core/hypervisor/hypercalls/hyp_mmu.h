/*
 * hyp_mmu.h
 *
 *  Created on: May 14, 2013
 *      Author: viktordo
 */

#ifndef HYP_MMU_H_
#define HYP_MMU_H_

void hypercall_create_section(addr_t va, addr_t pa, uint32_t page_attr);
void hypercall_switch_mm(addr_t table_base, uint32_t context_id);
void hypercall_free_pgd(addr_t *pgd);
void hypercall_new_pgd(addr_t *pgd);
void hypercall_set_pmd(addr_t *pmd, uint32_t val);
void hypercall_set_pte(addr_t *va, uint32_t linux_pte, uint32_t phys_pte);

#endif /* HYP_MMU_H_ */
