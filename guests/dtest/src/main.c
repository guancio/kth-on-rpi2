
#include <lib.h>
#include <types.h>

#include "hypercalls.h"

// TEMP STUFF
enum dmmu_command {
    CMD_MAP_L1_SECTION, CMD_UNMAP_L1_PT_ENTRY, CMD_CREATE_L2_PT, CMD_MAP_L1_PT
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

uint32_t l2[1024] __attribute__ ((aligned (4 * 1024)));
void dmmu_create_L2_pt_()
{
	  uint32_t pa, va, attrs, res;
	  int j;
/*	  attrs = 0x12; // 0b1--10
	  attrs |= 3 << 10;
	  attrs = (attrs & (~0x10)) | 0xC | (1 << 5);
	  va = 0xc0300000;
	  pa = 0x81100000;
	  ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);

	  printf("pa is %x\n", pa);
	  for(j = 0; j < 1024; j++)
	    //l2[j] = ((uint32_t)0x31);
		//l2[j] = ((uint32_t)0x32);
		//l2[j] = ((uint32_t)0x81100032); //self reference with ap = 3, it successfully failed
		//l2[j] = ((uint32_t)0x81100022); //self reference with ap = 2, it succeed
		//l2[j] = ((uint32_t)0x81200032); // pointing to guest initial l1 with ap = 3, it successfully failed
	  memcpy((void*)va, l2, sizeof l2);
	  ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);*/


	  // #0 : Guest can not write its own l2 page table in an unmapped area
	  // This test will break the system (Dabort)
	  /*
	  attrs = 0;
	  va = 0xc0300000;
	  pa = 0x81100000;
	  //ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	  for(j = 0; j < 1024; j++)
		  l2[j] = ((uint32_t)0x32);
	  memcpy((void*)va, l2, sizeof l2);
	   */


	  // #1 : Guest can not write its own l2 page table in a physical address outside the allowed range
	  // This test should fail because physical address 0x0 is not accessible by guest

	  attrs = 0xc2e;
	  va = 0xc0300000;
	  pa = 0x81100000;
	  ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	  for(j = 0; j < 1024; j++)
		  l2[j] = ((uint32_t)0x32);
	  memcpy((void*)va, l2, sizeof l2);
	  ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	  pa = 0x0; // setting pa to an address which will case a failure
	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
		asm("mov  %[result],r0 \n\t"
				:[result] "=r" (res)
				: /*input*/
				: /* No clobbers */);

	  if (res != 3)
		  printf("create_L2_pt 1: SUCCESS, add %x, res %d\n", pa, res);
	  else
		  printf("create_L2_pt 1: FAIL, add %x, res %d\n", pa, res);


	  // #2: Guest can not use an address which is not 4KB aligned to create an L2
	  // this test should fail because we are only allowed to create l2 in 4KB aligned address
	  attrs = 0xc2e;
	  va = 0xc0300000;
	  pa = 0x81100100; // this address is not 4KB aligned
	  ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	  for(j = 0; j < 1024; j++)
		  l2[j] = ((uint32_t)0x32);
	  memcpy((void*)va, l2, sizeof l2);
	  ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
		asm("mov  %[result],r0 \n\t"
				:[result] "=r" (res)
				: /*input*/
				: /* No clobbers */);

	  if (res != 7)
		  printf("create_L2_pt 2: SUCCESS, add %x, res %d\n", pa, res);
	  else
		  printf("create_L2_pt 2: FAIL, add %x, res %d\n", pa, res);

//	  // #3: Guest can not create a new L2 in a region which already contains an L2
//	  attrs = 0xc2e;
//	  va = 0xc0310000;
//	  pa = 0x81100000;
//	  ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
//	  for(j = 0; j < 1024; j++)
//	  	l2[j] = ((uint32_t)0x32);
//	  memcpy((void*)va, l2, sizeof l2);
//	  ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
//	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
//	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0); // this should fail
//		asm("mov  %[result],r0 \n\t"
//				:[result] "=r" (res)
//				: /*input*/
//				: /* No clobbers */);
//
//	  if (res != 9)
//		  printf("create_L2_pt 3: SUCCESS, add %x, res %d\n", pa, res);
//	  else
//		  printf("create_L2_pt 3: FAIL, add %x, res %d\n", pa, res);

	  // #4: Guest can not create a new L2 in a region which already contains an L1 or a referenced data page
	  attrs = 0xc2e;
	  va = 0xc0200000;
	  pa = 0x81100000;
	  ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	  for(j = 0; j < 1024; j++)
	  	l2[j] = ((uint32_t)0x32);
	  memcpy((void*)va, l2, sizeof l2);
	  // Commenting the next line will cause this test to fail because the reference counter of pointed data page is not zero
	  //ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	  asm("mov  %[result],r0 \n\t"
	  			:[result] "=r" (res)
	  			: /*input*/
	  			: /* No clobbers */);

	  if (res != 10)
		  printf("create_L2_pt 4: SUCCESS, add %x, res %d\n", pa, res);
	  else
		  printf("create_L2_pt 4: FAIL, add %x, res %d\n", pa, res);

	  // #5: Guest can not create a new L2 with an unsupported descriptor type (0b11)
	  attrs = 0xc2e;
	  va = 0xc0200000;
	  pa = 0x81100000;
	  ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	  for(j = 0; j < 1024; j++)
	  	l2[j] = ((uint32_t)0x31);
	  memcpy((void*)va, l2, sizeof l2);
	  // Commenting the next line will cause this test to fail because the reference counter of pointed data page is not zero
	  ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	  asm("mov  %[result],r0 \n\t"
	  			:[result] "=r" (res)
	  			: /*input*/
	  			: /* No clobbers */);

	  if (res != 11)
		  printf("create_L2_pt 5: SUCCESS, add %x, res %d\n", pa, res);
	  else
		  printf("create_L2_pt 5: FAIL, add %x, res %d\n", pa, res);

	  // #6: Guest can not create a new L2 with an entry which point to L2 page table itself with a writable  access permission
	  attrs = 0xc2e;
	  va = 0xc0200000;
	  pa = 0x81100000;
	  ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	  for(j = 0; j < 1024; j++)
		  l2[j] = ((uint32_t)0x81100032); //self reference with ap = 3, it successfully failed
	  memcpy((void*)va, l2, sizeof l2);
	  // Commenting the next line will cause this test to fail because the reference counter of pointed data page is not zero
	  ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	  asm("mov  %[result],r0 \n\t"
	  			:[result] "=r" (res)
	  			: /*input*/
	  			: /* No clobbers */);

	  if (res != 11)
		  printf("create_L2_pt 6: SUCCESS, add %x, res %d\n", pa, res);
	  else
		  printf("create_L2_pt 6: FAIL, add %x, res %d\n", pa, res);

//	  // Repeatiting the following test will break system
//	  //TODO: this can be fixed using l2 free  API
//	  // #7: Guest can create a new L2 with an entry which point to another page table with a readonly  access permission
//	  attrs = 0xc2e;
//	  va = 0xc0300000;
//	  pa = 0x81100000;
//	  ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
//	  for(j = 0; j < 1024; j++)
//		  l2[j] = ((uint32_t)0x81100022); //self reference with ap = 2, it succeed
//	  memcpy((void*)va, l2, sizeof l2);
//	  ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
//	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
//	  asm("mov  %[result],r0 \n\t"
//	  			:[result] "=r" (res)
//	  			: /*input*/
//	  			: /* No clobbers */);
//
//	  if (res == 0)
//		  printf("create_L2_pt 7: SUCCESS, add %x, res %d\n", pa, res);
//	  else
//		  printf("create_L2_pt 7: FAIL, add %x, res %d\n", pa, res);

	  // #8: Guest can not create a new L2 with an entry which point to another page table with a writable access permission
	  attrs = 0xc2e;
	  va = 0xc0200000;
	  pa = 0x81100000;
	  ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	  for(j = 0; j < 1024; j++)
		  l2[j] = ((uint32_t)0x81200032); // pointing to guest initial l1 with ap = 3, it successfully failed
	  memcpy((void*)va, l2, sizeof l2);
	  // Commenting the next line will cause this test to fail because the reference counter of pointed data page is not zero
	  ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	  ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	  asm("mov  %[result],r0 \n\t"
	  			:[result] "=r" (res)
	  			: /*input*/
	  			: /* No clobbers */);

	  if (res != 11)
		  printf("create_L2_pt 8: SUCCESS, add %x, res %d\n", pa, res);
	  else
		  printf("create_L2_pt 8: FAIL, add %x, res %d\n", pa, res);
}

void dmmu_l1_pt_map_()
{
	uint32_t pa, va, attrs, res;
	int j;

	// Creating an L1 to map
	attrs = 0xc2e;
	va = 0xc0300000;
	pa = 0x81100000;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	for(j = 0; j < 1024; j++)
		l2[j] = ((uint32_t)0x32);
	memcpy((void*)va, l2, sizeof l2);
    ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
    // end of L2 page table creation

	attrs = 0xc21;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs); // this call should fail because the entry has already been mapped


	// #1: I can not map 0, since it is reserved by the hypervisor to access the guest page tables
 	attrs = 0x0;
	va = 0x0;
	pa = 0x0;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	asm("mov  %[result],r0 \n\t"
	      :[result] "=r" (res)
	      : /*input*/
	      : /* No clobbers */);
	if (res != 1)
		printf("l1_pt_map 1: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("l1_pt_map 1: FAIL, add %x, res %d\n", va, res);

	// #2: mapping 0xc0200000 is ok, since it is the page containing the active page table
	// This test should fail, because we are not allowed to map in a physical address outside the guest allowed range
	attrs = 0x0;
	va = 0xc0200000;
	pa = 0x0;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	asm("mov  %[result],r0 \n\t"
	      :[result] "=r" (res)
	      : /*input*/
	      : /* No clobbers */);
	if (res != 3)
		printf("l1_pt_map 2: SUCCESS, add %x, res %d\n", va, res);
	else
		printf("l1_pt_map 2: FAIL, add %x, res %d\n", va, res);

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
    //dmmu_map_L1_section_();
    //dmmu_unmap_L1_pageTable_entry_();
    //dmmu_create_L2_pt_();
    dmmu_l1_pt_map_();
    printf("running\n");
  }
}

/*Each guest must provide a handler rpc*/
void handler_rpc(unsigned callNum, void *params)
{
    
}
