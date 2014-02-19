
#include <lib.h>
#include <types.h>

#include "hypercalls.h"

// TEMP STUFF
enum dmmu_command {
    CMD_MAP_L1_SECTION, CMD_UNMAP_L1_PT_ENTRY
};

extern uint32_t syscall_dmmu(uint32_t r0, uint32_t r1, uint32_t r2);
#define ISSUE_DMMU_HYPERCALL(type, p0, p1, p2) \
		syscall_dmmu(type | (p2 << 4), p0, p1);



uint32_t l1[4096] __attribute__ ((aligned (16 * 1024)));


void dmmu_map_L1_section_()
{
	uint32_t va, pa, attrs, res;
	// #1: I can not map 0, since it is reserved by the hypervisor to access the guest page tables
	va = 0x0;
	pa = 0x0;
 	attrs = 0x0;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	asm("mov  %[result],r0 \n\t"
	      :[result] "=r" (res)
	      : /*input*/
	      : /* No clobbers */);
	if (res == 1)
		printf("map_L1_section 1: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("map_L1_section 1: FAIL, add %x, res %d\n", va, res);

	// #2: mapping 0xc0200000 is ok, since it is the page containing the active page table
	// This test should fail, because we are not allowed to map in a physical address outside the guest allowed range
	va = 0xc0200000;
	pa = 0x0;
 	attrs = 0x0;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	asm("mov  %[result],r0 \n\t"
	      :[result] "=r" (res)
	      : /*input*/
	      : /* No clobbers */);
	if (res == 1)
		printf("map_L1_section 2: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("map_L1_section 2: FAIL, add %x, res %d\n", va, res);

	// #4: mapping 0xc020000 is ok, since it is the page containing the active page table
	// This test should fail, because this is already mapped
	va = 0xc0000000;
	pa = 0x81000000;
 	attrs = 0x0;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	asm("mov  %[result],r0 \n\t"
	      :[result] "=r" (res)
	      : /*input*/
	      : /* No clobbers */);
	if (res != 4)
		printf("map_L1_section 3: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("map_L1_section 3: FAIL, add %x, res %d\n", va, res);
	// #4: mapping 0xc020000 is ok, since it is the page containing the active page table
	// This test should fail, because the access permission is not supported
	va = 0xc0200000;
	pa = 0x81000000;
 	attrs = 0x0;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	asm("mov  %[result],r0 \n\t"
	      :[result] "=r" (res)
	      : /*input*/
	      : /* No clobbers */);
	if (res != 6)
		printf("map_L1_section 4: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("map_L1_section 4: FAIL, add %x, res %d\n", va, res);

	// #5: mapping 0xc020000 is ok, since it is the page containing the active page table
	// This test should succeed
	va = 0xc0200000;
	pa = 0x81000000;
	//attrs = 0x12; // 0b1--10
	//attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	//attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
 	attrs = 0xc2e;
 	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	asm("mov  %[result],r0 \n\t"
	      :[result] "=r" (res)
	      : /*input*/
	      : /* No clobbers */);
	if (res == 0)
		printf("map_L1_section 5: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("map_L1_section 5: FAIL, add %x, res %d\n", va, res);

	// #6: mapping 0xc030000 with readonly access permission is ok, since it is the page containing the active page table
	// This test should succeed
	va = 0xc0300000;
	pa = 0x81000000;
 	attrs = 0xb2e;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	asm("mov  %[result],r0 \n\t"
	      :[result] "=r" (res)
	      : /*input*/
	      : /* No clobbers */);
	if (res == 0)
		printf("map_L1_section 6: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("map_L1_section 6: FAIL, add %x, res %d\n", va, res);
}

void dmmu_unmap_L1_pageTable_entry_()
{
	uint32_t va, res;
	// #1: I can not unmap 0, since it is reserved by the hypervisor to access the guest page tables
	va = 0x0;
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	asm("mov  %[result],r0 \n\t"
			 :[result] "=r" (res)
			 : /*input*/
			 : /* No clobbers */);
	if (res == 1)
		printf("unmap_L1_pageTable_entry 1: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("unmap_L1_pageTable_entry 1: FAIL, add %x, res %d\n", va, res);

	// #2: I can not unmap 0xf0000000, since it is reserved by the hypervisor code
	va = 0xf0000000;
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	asm("mov  %[result],r0 \n\t"
			:[result] "=r" (res)
			: /*input*/
			: /* No clobbers */);
	if (res == 1)
		printf("unmap_L1_pageTable_entry 2: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("unmap_L1_pageTable_entry 2: FAIL, add %x, res %d\n", va, res);

	// #3: Unmapping 0xc0300000 has no effect, since this page is unmapped
	va = 0xc0300000;
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	asm("mov  %[result],r0 \n\t"
			:[result] "=r" (res)
			: /*input*/
			: /* No clobbers */);
	if (res == 2)
		printf("unmap_L1_pageTable_entry 3: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("unmap_L1_pageTable_entry 3: FAIL, add %x, res %d\n", va, res);

	// #4: Unmapping 0xc0200000 is ok if this test is executed after the l1_map_section test, otherwise it has no effect
	va = 0xc0200000;
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	asm("mov  %[result],r0 \n\t"
			:[result] "=r" (res)
			: /*input*/
			: /* No clobbers */);
	if (res == 0)
		printf("unmap_L1_pageTable_entry 4: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("unmap_L1_pageTable_entry 4: FAIL, add %x, res %d\n", va, res);

	// Unmapping 0xc0000000 is ok, but this is the page where the guest code resides
	//printf("test 5: THIS WILL BRAKE THE GUEST\n");
/*
	va = 0xc0000000;
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	asm("mov  %[result],r0 \n\t"
			:[result] "=r" (res)
			: input
			:  No clobbers );
	if (res == 0)
		printf("unmap_L1_pageTable_entry 5: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("unmap_L1_pageTable_entry 5: FAIL, add %x, res %d\n", va, res);
*/

}
void _main()
{
  int j;
  printf("starting\n");
    /* syscall_dmmu(CMD_CREATE_L1, &l1, 0); */
    
    /* syscall_dmmu(CMD_CREATE_L1, 0xF0000000, 0); */
    /* syscall_dmmu(CMD_CREATE_L1, 0x00000000, 0); */
    /* syscall_dmmu(CMD_CREATE_L1, 0x10000000, 0); */
    // test hypercall from guest
    // ISSUE_HYPERCALL_REG1(HYPERCALL_NEW_PGD, 0x01234567);
  for(;;) {
    for(j = 0; j < 500000; j++) asm("nop");
    dmmu_map_L1_section_();
    dmmu_unmap_L1_pageTable_entry_();
    printf("running\n");
  }
}

/*Each guest must provide a handler rpc*/
void handler_rpc(unsigned callNum, void *params)
{
    
}
