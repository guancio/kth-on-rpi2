#include "linux_signal.h"
#include "hyper_config.h"
#include "hyper.h"
#include "mmu.h"

extern uint32_t *flpt_va;
extern uint32_t *slpt_va;

extern uint32_t l2_index_p;
extern virtual_machine *curr_vm;

/*We copy the signal codes to the vector table that Linux uses here */

const unsigned long sigreturn_codes[7] = {
	MOV_R7_NR_SIGRETURN,    SWI_SYS_SIGRETURN,    SWI_THUMB_SIGRETURN,
	MOV_R7_NR_RT_SIGRETURN, SWI_SYS_RT_SIGRETURN, SWI_THUMB_RT_SIGRETURN,
};

const unsigned long syscall_restart_code[2] = {
	SWI_SYS_RESTART,	/* swi	__NR_restart_syscall */
	0xe49df004,		/* ldr	pc, [sp], #4 */
};
void clear_linux_mappings()
{
	uint32_t PAGE_OFFSET   = curr_vm->guest_info.page_offset;
	uint32_t VMALLOC_END   = curr_vm->guest_info.vmalloc_end;
	uint32_t guest_size    = curr_vm->guest_info.guest_size;
	uint32_t MODULES_VADDR = (curr_vm->guest_info.page_offset - 16*1024*1024);
	uint32_t address;

	uint32_t offset = 0;
	uint32_t *pgd = flpt_va;

	/*
	 * Clear out all the mappings below the kernel image. Maps
	 */
	for(address = 0; address <  MODULES_VADDR; address += 0x200000 ){
		offset = address  >> 21;	//Clear pages 2MB at time
		pgd[offset] = 0;
		pgd[offset +1] = 0;
		COP_WRITE(COP_SYSTEM,COP_DCACHE_INVALIDATE_MVA, pgd);
	}

	for ( ; address < PAGE_OFFSET; address += 0x200000){
		offset= address >> 21;
		pgd[offset] = 0;
		pgd[offset+1] = 0;
		COP_WRITE(COP_SYSTEM,COP_DCACHE_INVALIDATE_MVA, pgd);
	}

	/*
	 * Clear out all the kernel space mappings, except for the first
	 * memory bank, up to the end of the vmalloc region.
	 */
	if(VMALLOC_END > HAL_VIRT_START)
		hyper_panic("Cannot clear out hypervisor page\n", 1);

	for (address = PAGE_OFFSET + guest_size;address < VMALLOC_END; address += 0x200000){
		offset= address >> 21;
		pgd[offset] = 0;
		pgd[offset+1] = 0;
		COP_WRITE(COP_SYSTEM,COP_DCACHE_INVALIDATE_MVA, pgd);
	}
}

void init_linux_sigcode()
{
	memcpy((void *)KERN_SIGRETURN_CODE, sigreturn_codes,
	       sizeof(sigreturn_codes));
	memcpy((void *)KERN_RESTART_CODE, syscall_restart_code,
	       sizeof(syscall_restart_code));
}

void init_linux_page()
{
    uint32_t *p, i;

	/*Map the first pages for the Linux kernel OS specific
	 * These pages must cover the whole boot phase before the setup arch
	 * in Linux, in case of built in Ramdisk its alot bigger*/

	uint32_t linux_phys_ram = HAL_PHYS_START + 0x1000000;
	pt_create_coarse(flpt_va,0xc0000000, linux_phys_ram, 0x100000, MLT_USER_RAM);

	for (i = 1; i < 0x50; i++){
		pt_create_section(flpt_va, 0xC0000000 + (i * (1 << 20 )), linux_phys_ram + i*(1 << 20), MLT_USER_RAM);
	}

	uint32_t phys = 0;
	p = (uint32_t *)((uint32_t)slpt_va + ((l2_index_p -1) *0x400)); /*256 pages * 4 bytes for each lvl 2 page descriptor*/
	/*Modify the master swapper page global directory to read only */

	/*Maps first eight pages 0-0x8000*/
	for(i=0x4;i < 0x8; i ++, phys+=0x1000){
		 p[i] &= PAGE_MASK;
		 /*This maps Linux kernel pages to the hypervisor and sets it read only*/
		 p[i] |= (uint32_t)(GET_PHYS(flpt_va) + phys);

		 p[i] &= ~(MMU_L2_SMALL_AP_MASK << MMU_L2_SMALL_AP_SHIFT);
		 p[i] |= (MMU_AP_USER_RO << MMU_L2_SMALL_AP_SHIFT); //READ only
	}
}

void linux_init()
{
	/*Initalise the page tables for Linux kernel*/
	init_linux_page();
    /*Copy the signal codes into the vector table*/
	init_linux_sigcode();
}



