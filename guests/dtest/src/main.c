
#include <lib.h>
#include <types.h>

#include "hypercalls.h"
#include "print_err.h"

enum mmu_ap { MMU_AP_NONE = 0, MMU_AP_SUP_RW, MMU_AP_USER_RO, MMU_AP_USER_RW };
#define MMU_SECTION_AP_SHIFT 10
#define MMU_PT_AP_SHIFT 4
#define HC_DOM_DEFAULT 	0
#define HC_DOM_KERNEL 	1
#define HC_DOM_TASK 	2
#define HC_DOM_TRUSTED 	3
#define MMU_L1_DOMAIN_SHIFT 5


uint32_t l1[4096] __attribute__ ((aligned (16 * 1024)));
uint32_t l2[1024] __attribute__ ((aligned (4 * 1024)));

// TEMP STUFF
enum dmmu_command
{
  CMD_MAP_L1_SECTION, CMD_UNMAP_L1_PT_ENTRY, CMD_CREATE_L2_PT, CMD_MAP_L1_PT, CMD_MAP_L2_ENTRY, CMD_UNMAP_L2_ENTRY, CMD_FREE_L2, CMD_CREATE_L1_PT, CMD_SWITCH_ACTIVE_L1, CMD_FREE_L1
};


extern uint32_t syscall_dmmu(uint32_t r0, uint32_t r1, uint32_t r2);
#define ISSUE_DMMU_HYPERCALL(type, p0, p1, p2) \
		syscall_dmmu(type | (p2 << 4), p0, p1);

#define ISSUE_DMMU_HYPERCALL_(type, p0, p1, p2, p3) \
		syscall_dmmu((type | (p2 & 0xFFFFFFF0)), p0, ((p1 << 20) | p3));


addr_t pstart;
addr_t vstart;
size_t psize;
size_t fwsize;


uint32_t va_base;


uint32_t va2pa(uint32_t va) {
	return va - vstart + pstart;
}

void test_map_l1_section()
{
	char * test_name= "MAP L1 SECTION";
	printf("Main Test: %s\n", test_name);

	uint32_t va, pa, attrs, res;
	int t_id = 0;
	// #0: I can not map 0, since it is reserved by the hypervisor to access the guest page tables
	va = 0x0;
	pa = 0x0;
 	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(t_id, "Map of reserved virtual address", ERR_MMU_RESERVED_VA, res);

	// #1: mapping 0xc0200000 is ok, since it is the page containing the active page table
	// This test should fail, because we are not allowed to map in a physical address outside the guest allowed range
	va = (va_base + 0x200000);
	pa = 0x0;
 	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Map a physical address outside the guest allowed range", ERR_MMU_OUT_OF_RANGE_PA, res);

	// #2: mapping 0xc020000 is ok, since it is the page containing the active page table
	// This test should fail, because this is already mapped
	va = (va_base + 0x000000);
	pa = va2pa(va);
 	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Map an already mapped entry", ERR_MMU_SECTION_NOT_UNMAPPED, res);


	// #3: mapping 0xc020000 is ok, since it is the page containing the active page table
	// This test should fail, because the access permission is not supported
	va = (va_base + 0x200000);
	pa = va2pa(va);
 	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Unsupported access permission", ERR_MMU_AP_UNSUPPORTED, res);

	// #4: mapping 0xc020000 is ok, since it is the page containing the active page table
	// This test should succeed
	va = (va_base + 0x200000);
	pa = va2pa(va_base + 0x000000);
	//attrs = 0x12; // 0b1--10
	//attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	//attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
 	attrs = 0xc2e;
 	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping a valid writable page", SUCCESS, res);

	// #5: unmap 0xc030000
	// This test should succeed
	va = (va_base + 0x300000);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id, "Unmapping an existing mapped page", SUCCESS, res);

	// #6: mapping 0xc030000 with read-only access permission is ok, since it is the page containing the active page table
	// This test should succeed
	va = (va_base + 0x300000);
	pa = va2pa(va_base + 0x000000);
 	attrs = 0xb2e;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping a valid read/only page", SUCCESS, res);
}


void test_unmap_l1_entry()
{
	char * test_name= "UNMAP L1 SECTION";
	printf("Main Test: %s\n", test_name);

	uint32_t va,pa, res, attrs;
	int t_id = 0;
	// #0: I can not unmap 0, since it is reserved by the hypervisor to access the guest page tables
	va = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id, "Unamap of a reserved va", ERR_MMU_RESERVED_VA, res);


	// #1: I can not unmap 0xf0000000, since it is reserved by the hypervisor code
	va = 0xf0000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id, "Unamap of a reserved va", ERR_MMU_RESERVED_VA, res);



	// #3: Unmapping 0xc0200000 is ok if this test is executed after the l1_map_section test, otherwise it has no effect
	va = (va_base + 0x200000);
	pa = va2pa(va_base);
	attrs = 0x12; // 0b1--10
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping a valid writable page", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id, "Unmapping a valid writable page", SUCCESS, res);

	// #2: Unmapping 0xc0200000 has no effect, since this page is unmapped
	va = (va_base + 0x200000);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id, "Unamaping an not mapped entry", ERR_MMU_ENTRY_UNMAPPED, res);

}

void  test_l2_create()
{
	  uint32_t pa, va, attrs, res, desc;
	  int j, t_id = 0;


	  attrs = 0;
	  attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT;

	  pa = va2pa(va);

/*	  // #0 : Guest can not write its l2 page table in an unmapped area
  	  // This test will break the system (Dabort)
	  va = (va_base | (uint32_t)0x300000) ;
	  res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	  expect(t_id, "Unamap of a reserved va", ERR_MMU_RESERVED_VA, res);
	  memset((void *)va, 0x32, 4096*4);
*/

	 // #1 : Guest can not write its own l2 page table in a physical address outside the allowed range
	 // This test should fail because physical address 0x0 is not accessible by the guest
	 pa = 0x0; // setting pa to an address which will cause a failure
	 res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	 expect(++t_id, "Using a physical address outside the guest allowed range", ERR_MMU_OUT_OF_RANGE_PA, res);

	 // #2: Guest can not use an address which is not 4KB aligned to create an L2
	 // this test should fail because we are only allowed to create l2 in a 4KB aligned address
	 pa = va2pa(va_base + 0x100100); //0x81100100, this address is not 4KB aligned
	 res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	 expect(++t_id, "Using a physical address which is not 4KB aligned", ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED, res);

	 // #3: Guest can not create a new L2 in a region which already contains an L2
	 va = (va_base | (uint32_t)0x1b0000) ;
	 pa = va2pa(va);

     for(j = 0; j < 1024; j++)
	  	l2[j] = ((uint32_t)0x0);
	 memcpy((void*)va, l2, sizeof l2);
	 // unmap the section
 	 res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
 	 expect(t_id,"Successful unmap the L1 entry", SUCCESS, res);

     res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
 	 expect(t_id,"Success in creating a L2 ", SUCCESS, res);
 	 res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0); // this should fail
	 expect(t_id,"Failing to create a L2 where already there exist another L2", ERR_MMU_ALREADY_L2_PT, res);
	 res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	 expect(t_id,"Successful freeing the given L2", SUCCESS, res);

	// #4: Guest can not create a new L2 in a region which already contains a page table or a referenced data page
	//*******************************************************//
	// Creating an L2 to map
	va = (va_base | (uint32_t)0x320000) ;
	pa = va2pa(va);

	for(j = 0; j < 1024; j++)
	  	l2[j] = ((uint32_t)0x0);
	memcpy((void*)va, l2, sizeof l2);
	// unmap the section
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	//*******************************************************//
	uint32_t pga = va2pa(va_base + 0x110000); // data page address
	uint32_t idx = 0xc2;
	ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, 0x32);
	// here pga is pointing to a referenced data page
    // Commenting the next line will cause this test to fail because the reference counter of pointed data page is not zero
    //ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pga, 0, 0); //referenced data page
	expect(++t_id,"Failing to create a L2 where guest tries to use a referenced data page", ERR_MMU_REFERENCED, res);

	// #5: Guest can not create a new L2 with an unsupported descriptor type (0b11)
	attrs = 0x12; // 0b1--10 // Section: non useful since already setted by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	va = (va_base + 0x400000) ;
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(t_id, "Creating a section to write data", SUCCESS, res);

	desc = (pstart | 0x31);
	for(j = 0; j < 1024; j++)
		l2[j] = ((uint32_t)desc);
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"Failing to create a L2 with an unsupported descriptor type (0b11)", ERR_MMU_SANITY_CHECK_FAILED, res);

	 // #6: Guest can not create a new L2 with an entry which point to L2 page table itself with a writable  access permission
	va = (va_base | (uint32_t)0x160000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(t_id, "Creating a section to write data", SUCCESS, res);
	desc = (pa & 0xffff0000) | 0x32;
	for(j = 0; j < 1024; j++)
		 l2[j] = ((uint32_t)desc); //self reference with ap = 3, it successfully failed
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"Failing to create a L2, where L2 base is in writable section", ERR_MMU_SANITY_CHECK_FAILED, res);
	ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);

	  // #7: Guest can create a new L2 with an entry which point to another page table with a read-only  access permission
	va = (va_base | (uint32_t)0x160000);
	pa = va2pa(va);
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	desc = (pa & 0xffff0000) | 0x22;
	for(j = 0; j < 1024; j++)
		  l2[j] = ((uint32_t)desc); //self reference with ap = 2, it succeed
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"create a new L2 with an entry which point to another page table with a read-only  access permission", SUCCESS, res);
	ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);

	// #8: Guest can not create a new L2 with an entry which point to another page table with a writable access permission
	va = (va_base | (uint32_t)0x160000);
	pa = va2pa(va);
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	desc = ((pstart & 0xff000000)| (1 << 23) | 0x32);
	for(j = 0; j < 1024; j++)
		  l2[j] = ((uint32_t)desc); //referencing to the current active L1 with ap = 3, it succeed
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"create a new L2 with an entry which point to another page table with a writable  access permission", ERR_MMU_SANITY_CHECK_FAILED, res);
	ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);


}

void test_l1_pt_map()
{
	uint32_t pa, va, attrs, res, desc;
	int j, t_id = 0;

	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT;
	// Creating an L2 to map
	va = (va_base | (uint32_t)0x170000);
	pa = va2pa(va);
	desc = (pa & 0xffff0000) | 0x22; // self referencing with read-only access permission
	for(j = 0; j < 1024; j++)
		l2[j] = ((uint32_t)desc);
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res= ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id,"Successful creation of a new L2", SUCCESS, res);
    // end of L2 page table creation

	// #1: I can not map 0, since it is reserved by the hypervisor to access the guest page tables
	va = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(++t_id,"Map l1 entry in a virtual address which is reserved for hypervisor use", ERR_MMU_RESERVED_VA, res);

	// #2: mapping 0xc0200000 is ok, since it is the page containing the active page table
	// This test should fail, because we are not allowed to map in a physical address outside the guest allowed range
	va = (va_base | (uint32_t)0x200000);
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(++t_id,"Mapping a physical address is outside the guest allowed range", ERR_MMU_OUT_OF_RANGE_PA, res);

	// #3: mapping 0xc0200000 is ok, since it is the page containing the active page table
	// This test should fail, because the given address does not point to a valid L2
	attrs = 0x0;
	va = (va_base | (uint32_t)0x210000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(++t_id,"Mapping a non-page table page in one of the the active L1 as L2 page table", ERR_MMU_IS_NOT_L2_PT, res);

	// #4: mapping 0xc0170000 is ok, since it is the page containing a valid L2
	// This test should fail, because PXN is enabled
	attrs = 0x24;
	va = (va_base | (uint32_t)0x170000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(++t_id,"Mapping an entry using a descriptor for which PXN has been enabled", ERR_MMU_XN_BIT_IS_ON, res);

	// #5: mapping 0xc0170000 is ok, since it is the page containing a valid L2
	// This test should fail, because guest can not map an L2 in a given entry two times in row
	attrs = 0x20;
	va = (va_base | (uint32_t)0x170000);
	pa = va2pa(va);
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs); // this call should fail because the entry has already been mapped
	expect(++t_id,"Mapping an entry of the L1 two times", ERR_MMU_PT_NOT_UNMAPPED, res);

}

void test_l2_map_entry()
{
	// idx is the index of a entry we want to map pga into
	uint32_t pa, va, idx, pga, attrs, res, desc;
	int j, t_id = 0;

	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT;

	// Creating an L2 to map
	va = (va_base | (uint32_t)0x100000);
	pa = va2pa(va);
	desc = (pa & 0xffff0000) | 0x22; // self referencing with read-only access permission
	for(j = 0; j < 1024; j++)
		l2[j] = ((uint32_t)desc);
	memcpy((void*)va, l2, sizeof l2);

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id++,"Successful unmap of the section", SUCCESS, res);

	res= ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id,"Successful creation of the new page", SUCCESS, res);
    // end of L2 page table creation

	// #0: L2 base can not be 0, since it is reserved by the hypervisor to access the guest page tables
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping an entry of an invalid L2, in physical address outside the allowed range", ERR_MMU_OUT_OF_RANGE_PA, res);


	// #1: L2 base address is ok, but guest can not map a page outside the allowed range into its L2 page table entries
	pga = 0x0;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping a physical address outside the guest allowed range", ERR_MMU_OUT_OF_RANGE_PA, res);

	// #2: All the parameters are correct and this test should succeed

	va = (va_base | (uint32_t)0x100000);
	pa = va2pa(va);
	pga = va2pa((va_base | (uint32_t)0x110000));
	idx = 0xc2;

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping the given entry of the valid L2", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping a valid physical address into one of a valid L2", SUCCESS, res);

	// #3: this test should fail, because the entry has already been mapped.
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping an entry two times", ERR_MMU_PT_NOT_UNMAPPED, res);

	// #4: this test should fail, because guest can not map a page table to an entry of the given L2 with writable access permission

	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pa, attrs);
	expect(++t_id,"Mapping a page table into one of the L2 entries with writable access permission", ERR_MMU_INCOMPATIBLE_AP, res);

	// #5: this test should fail, because guest can not map any thing to an entry of a data page
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pga, idx, pga, attrs);
	expect(++t_id,"Mapping an entry of a data page!!!!", ERR_MMU_IS_NOT_L2_PT, res);

	// #6: this test should fail, because guest is passing an unsupported  access permission
	attrs = 0x02;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping the given entry of the valid L2", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping an entry of L2 using an unsupported access permission!!!!", ERR_MMU_AP_UNSUPPORTED, res);

	// #7: All the parameters are correct and this test should succeed
	// in this test reference counter of the mapped page should not be increased
	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT;

	pga = va2pa((va_base | (uint32_t)0x310000));
	idx = 0xab;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping the given entry of the valid L2", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping an entry of L2 with valid parameters", SUCCESS, res);

	// #8: All the parameters are correct and this test should succeed
	// in this test guest is mapping the L2 base address into one of L2 entry with read-only access permission
	idx = 0xac;
	attrs = 0;
	attrs |= MMU_AP_USER_RO << MMU_PT_AP_SHIFT;

	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pa, attrs);
	expect(++t_id,"Mapping base address of the given L2 into an entry of L2 with read-only access permission", SUCCESS, res);

}

void test_l2_unmap_entry()
{
	// idx is the index of a entry we want to map pga into
	uint32_t pa, va, idx, pga, attrs, res, desc;
	int j, t_id = 0;

	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT;

	// Creating an L2 to map
	va = (va_base | (uint32_t)0x100000);
	pa = va2pa(va);
	desc = (pa & 0xffff0000) | 0x22; // self referencing with read-only access permission
	// The first entry is not mapped
	for(j = 1; j < 1024; j++)
		l2[j] = ((uint32_t)desc);
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res= ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id,"Successful creation of a new L2", SUCCESS, res);
    // end of L2 page table creation


	// #0: L2 base address can not be 0x0, since it is reserved by the hypervisor to access the guest page tables
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(t_id,"Unmapping an entry of an invalid L2 for which the base address is outside the allowed range", ERR_MMU_OUT_OF_RANGE_PA, res);


	// #1: The guest is trying to unmap entry of a data page
	// This test should fail because the l2 base address is not pointing to a valid page table(L2)
	pa = va2pa(va_base | (uint32_t)0x110000);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an entry of an invalid L2, page type associated with the physical address is not L2-page table", ERR_MMU_IS_NOT_L2_PT, res);

	// #2: The entry guest is trying to unmap an entry that is not mapped
	// This test should succeed
	idx = 0x0;
	pa = va2pa((va_base | (uint32_t)0x100000));
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an unmapped entry of an L2", SUCCESS, res);

	// #3: all the parameters are well defined
	// This test should succeed but the reference counter should remain untouched
	idx = 0x0;
	attrs = 0;
	attrs |= MMU_AP_USER_RO << MMU_PT_AP_SHIFT;
	ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pa, attrs);

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an entry of L2 which points to the L2 itself", SUCCESS, res);

	// #4: this test is a successful attempt
	idx = 0xc2;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an entry of L2 which points to a data page", SUCCESS, res);
}

void test_l2_unmap_pt()
{
	// 1) create an L2 at the address va2pa(va_base | (uint32_t)0x100000)
	// 2) map all the entries of the L2 to points to va2pa(va_base | (uint32_t)0x300000) and writable
	// 3) map an l1 entry with the new pt:
	//       all va (in blocks of 4KB) in (va_base | (uint32_t)0x100000) points to va2pa(va_base | (uint32_t)0x300000)
	// 4) write into (va_base | (uint32_t)0x100000) and read from (va_base | (uint32_t)0x100000+4096). This shoud point to the same address
	// 5) build a section that alias the same phisical region
	//       all va (block of 1MB) in va_base + 2 * 0x100000 point to va2pa(va_base | (uint32_t)0x300000)

	// idx is the index of a entry we want to map pga into
	uint32_t pa, va, idx, pga, attrs, res, desc;
	int j, t_id = 0;

	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT;


	// Creating an L2 to map
	va = (va_base | (uint32_t)0x100000);
	pa = va2pa(va);
	pga = va2pa(va_base |(uint32_t)0x300000);

	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res= ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id,"Successful creation of a new L2", SUCCESS, res);
	for(idx = 0; idx < 1024; idx++) {
		res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
		expect(t_id++,"Update the L2 entry", SUCCESS, res);
	}

	attrs = 0x0;
	attrs |= (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(t_id++,"Successful map the new L2", SUCCESS, res);

	// now the va is mapped and is translated to
	// va and va + 4kb*i are pointing to the same address
	(*((uint32_t *)va)) = 666;
	uint32_t val = (*((uint32_t *)(va + 4096)));
	expect(t_id++,"The small pages mapped by the new L2 points all to the same physical address", 666, val);


	// is true that va is pointing to va2pa(va_base | (uint32_t)0x300000)?
	// We build an additional section mapping to the address va2pa(va_base | (uint32_t)0x300000)
	// then we read/write
	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs |= (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	uint32_t va_section =  va_base + 2 * 0x100000;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va_section, pga, attrs);
	expect(t_id++,"Successfoul alias of the va address using a section", SUCCESS, res);

	val = (*((uint32_t *)(va_section)));
	expect(t_id++,"The small pages mapped by the new L2 points to the right physical address", 666, val);

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va_section, 0, 0);
	expect(t_id++,"Successfoul unmap of the section aliasing the va", SUCCESS, res);
	// finished


	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(t_id,"Unsuccessful freeing a referenced L2", ERR_MMU_REFERENCE_L2, res);

    // #1: this test should succeed
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id,"Successful unmap the L1 entry pointing to the new L2", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(t_id,"Successful free the given L2", SUCCESS, res);

	// #2: this test should fail because L2 base address can not be 0x0, since it is reserved by the hypervisor to access the guest page tables
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(t_id,"Freeing L2 should faile because the base address is out of the allowed range", ERR_MMU_OUT_OF_RANGE_PA, res);

	// #3: this test should fail because L2 base address does not point to a valid L2
	pa = va2pa(va_base | (uint32_t)0x110000);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(t_id,"Freeing L2 should faile because the base address is not pointing to an L2", ERR_MMU_IS_NOT_L2_PT, res);
}

void test_l1_create()
{

	char * test_name= "CREATE L1";
	printf("Main Test: %s\n", test_name);

	uint32_t pa, va, attrs, res;
	int j, t_id = 0;

	// #0: this test should fail because guest is trying to create a new page table in a part of the memory that is reserved for hypervisor use
	pa = 0x80000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id, "Failing to create a L1 in the hypervisor memory", ERR_MMU_OUT_OF_RANGE_PA, res);

	// #1: this test should fail because L1 base is not aligned
	pa = va2pa(va_base + 0x101000);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id, "Failing to create a L1 no 16KB aligned", ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED, res);

	// #2: this test should fail because guest is trying to create a new page table in a part of the memory where another L1 resides in
	pa = va2pa(va_base + 0x200000);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Failing to create a L1 on an already L1 area", ERR_MMU_ALREADY_L1_PT, res);


	// #3: Test that we can not create an L1 in a referenced page: e.g. the same page where we are working
	memset(l1, 0, 4096*4);

	pa = va2pa(l1);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Failing to create a L1 on referenced part of the memory", ERR_MMU_REFERENCED, res);

	// From here we always map a section, write the page table and then unmap the section.
	// Finally we can call the API

	// #4: this test should fail because guest is trying to map a non-L2 page table page into L1 as a L2 page table

	attrs = 0x12; // 0b1--10 // Section: non useful since already setted by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	va = (va_base + 0x400000);
	pa = va2pa(va);

	// Writing content of the new L1 page table

	uint32_t desc = (pstart | 0x1);
	memset(l1, 0, 4096*4);
	l1[512] = ((uint32_t)desc); // L2 descriptor
	memcpy((void*)va, l1, sizeof l1);

	// unmap the section
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Successful unmap the L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Failing to create a L1 using an L2 that has not been validated", ERR_MMU_SANITY_CHECK_FAILED, res);

	// #5: this test should fail because guest is trying to use super section descriptor instead of a section descriptor
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Successful map of the new page", SUCCESS, res);

	desc = (pstart | (1 << 18) | 0x2);
	memset(l1, 0, 4096*4);
	l1[512] = ((uint32_t)desc); // Supersection
	memcpy((void*)va, l1, sizeof l1);

	// unmap the section
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Successful unmap the L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Failing to create a L1 containing a supersection", ERR_MMU_SANITY_CHECK_FAILED, res);

	// #6: this test should fail because guest is trying to use an unsupported access permission in a section descriptor
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Successful map of the new page", SUCCESS, res);

	desc = (pstart | 0x2);
	memset(l1, 0, 4096*4);
	l1[512] = ((uint32_t)desc); // L2 descriptor (ap = 0 it is unsupported)
	memcpy((void*)va, l1, sizeof l1);

	// unmap the section
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Successful unmap the L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Failing to create a L1 containing an unsupported AP", ERR_MMU_SANITY_CHECK_FAILED, res);

	// #7: this test should fail because guest is trying to map part of the memory as a writable section where I initial page table of guest resides
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Successful map of the new page", SUCCESS, res);

	desc = ((pstart & 0xff000000)| (1 << 23) | 0xC2E);
	memset(l1, 0, 4096*4);
	l1[512] = ((uint32_t)desc); // It points to an L1 (the current one)
	memcpy((void*)va, l1, sizeof l1);

	// unmap the section
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Successful unmap the L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Failing to create a L1 where a writable section points to an L1", ERR_MMU_SANITY_CHECK_FAILED, res);

	// #8: this test should fail because guest is trying to map part of the memory as a writable section where it is trying to create the new L1
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Successful map of the new page", SUCCESS, res);

	memset(l1, 0, 4096*4);
	l1[512] = (((uint32_t)pa) & 0xFFF00000) | (uint32_t)0xC02;
	memcpy((void*)va, l1, sizeof l1);

	// unmap the section
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Successful unmap the L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Failing to create a L1 where a writable section points to the L1 being allocated", ERR_MMU_SANITY_CHECK_FAILED, res);

	//TODO: check that the L1 contains a writable section that points outside the guest memory
}

void test_l1_create_empty_l1() {
	char * test_name= "CREATE L1 TMP";
	printf("Main Test: %s\n", test_name);

	uint32_t pa, va, attrs, res;
	int j, t_id = 0;
	// creating a writable section to map
	// for this test minimal_config.c has been modified and now ".pa_for_pt_access_end = HAL_PHYS_START + 0x014fffff"
	attrs = 0x12; // 0b1--10 // Section: non useful since already setted by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	va = (va_base + 0x400000);
	pa = va2pa(va);

	// Map the pa thus we are able to store the pagetable
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(t_id++,"Successful map of the new page", SUCCESS, res);

	memset(l1, 0, 4096*4);
	memcpy((void*)va, l1, sizeof l1);

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id++,"Successful unmap the L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(t_id++,"Successful creation of an L1 with only one section, writable that point to a data page", SUCCESS, res);
}

void test_l1_create_and_switch_l1() {
	// Test summary:
	// - we map a new section
	// - we write some random data into this section using some va
	// - we map a new section for the new L1
	// - we create a second L1 as follows:
	//    - the first 0xc000 is mapped as usual since it contains the test code
	//    - the second a "random" address points to the page where we written the data
	// - we unmap the section of the new L1
	// - we switch to the create L1
	// - we read the written data

	char * test_name= "CREATE L1 TMP";
	printf("Main Test: %s\n", test_name);

	uint32_t pa, va, attrs, res;
	int j, t_id = 0;

	// 1) creating a writable section to map (0xc0300000). Here we write the data
	attrs = 0x12; // 0b1--10 // Section: non useful since already setted by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	va = (va_base + 0x300000);
	pa = va2pa(va);

	// 2) write some data
	*((uint32_t*)(va+1024)) = (uint32_t)666;
	expect(++t_id,"Successful section modification", 666, *((uint32_t*)(va+1024)));

	// 3) creating a writable section to map (0xc0400000). Here we write the L1
	attrs = 0x12; // 0b1--10
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	va = (va_base + 0x400000);
	pa = va2pa(va);

	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Successful map of the new page", SUCCESS, res);

	// 4) create the new L1
	// initial empty
	memset(l1, 0, 4096*4);

	// Granting to the guest a writable part of the memory
	l1[3072] = (((uint32_t)va2pa(0xc0000000)) & 0xFFF00000) | (uint32_t)0xC2E;
	l1[513] = (((uint32_t)va2pa(0xc0100000)) & 0xFFF00000) | (uint32_t)0x82C;
	memcpy((void*)va, l1, sizeof l1);

	// 5) Un-mapping the new L1 section
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Successful unmapping an entry of l1 which points to a section where the new L1 resides in", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(t_id++,"Successful creation of an L1 with only one section, writable that point to a data page", SUCCESS, res);

	// 6) switching to the new L1
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	expect(++t_id,"Successful switching to the new L1", SUCCESS, res);

	// trying to read the content of the modified page under the new page table
	va = (va_base + 0x300000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Successful map of the new page", SUCCESS, res);
    printf("reading the content of the modified page under new L1, content %x \n",*((uint32_t*)(va+1024)) );

    // trying to read the content of the suspended page table under the new page table
    va = (va_base + 0x200000);
    pa = va2pa(va);
    attrs = 0x12; // 0b1--10
	attrs |= MMU_AP_USER_RO << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Successful map of the new page", SUCCESS, res);

	uint32_t index = 0;
	for(index = 0; index < 4096; index++)
	{
		if(*(((uint32_t *)0xc0200000) + index) != 0x0)
			printf("Suspended l1 content index %x desc %x \n", index, *(((uint32_t *)0xc0200000) + index));
	}
}


void test_switch_mm()
{
	uint32_t pa, va, attrs, res;
	int j, t_id = 0;

	// #1: this test should fail because guest is trying to create a new page table in a part of the memory that is reserved for hypervisor use
	pa = 0x80000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	expect(++t_id,"Failing to switch to the given physical address which is out of guest allowed  memory range", ERR_MMU_OUT_OF_RANGE_PA, res);

	// #2: this test should fail because L1 base is not aligned
	pa = va2pa(va_base + 0x101000);
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	expect(++t_id,"Failing to switch to the given L1 physical address which not 16KB aligned", ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED, res);

	// #3: this test should fail because guest is trying to switch into a non-page table page
	pa = va2pa(va_base + 0x100000);
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	expect(++t_id,"Failing to switch to the given physical address because guest is trying to switch into a non-page table page", ERR_MMU_IS_NOT_L1_PT, res);

	test_l1_create_and_switch_l1();
}

void test_unmap_L1_pt()
{
	uint32_t pa, va, attrs, res;
	int j, t_id = 0;

	// #0: this test should fail because guest is trying to create a new page table in a part of the memory that is reserved for hypervisor use
	pa = 0x80000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unsuccessful unmapping a physical address outside the guest allowed range", ERR_MMU_OUT_OF_RANGE_PA, res);

	// #1: this test should fail because base address is not aligned
	pa = va2pa(va_base + 0x101000);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unsuccessful unmapping a physical address which is not 16KB aligned", ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED, res);

	// #2: this test should fail because base address is pointing to the active L1 page table
	pa = va2pa(va_base + 0x200000);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unsuccessful unmapping the active L1 page table", ERR_MMU_FREE_ACTIVE_L1, res);

	// #3: this test should fail because base address is not pointing to a L1 page table
	pa = va2pa(va_base + 0x1C0000);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unsuccessful unmapping a physical address which is not pointing to a L1 page table", ERR_MMU_IS_NOT_L1_PT, res);

	/*// Scenario for the following test case
	   - mapping a section, call it SEC, with writable access permission(this should increase the reference counter of the involved page by 1)
	   - Creating a new L1
	   - Switching to the new L1
	   - Free the suspended L1
	   - creating another mapping for SEC in the new L1
	   - preparing an L2
	   - Unmapping the SEC section
	   - calling Create_L2 API which should succeed
	*/
	// 1) mapping a section (0x81300000 to 0xc0300000), call it SEC, with writable access permission(this should increase the reference counter of the involved page by 1)
	// 2) Creating a new L1
	// 3) Switching to the new L1
	test_l1_create_and_switch_l1();

	// 4) Freeing the initial L1 page table
	pa = va2pa(va_base + 0x200000);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unmapping the guest initial master page table", SUCCESS, res);

	// Unmapping the read-only mapping of the section
	va = (va_base + 0x200000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id++,"Successful unmap the L1 entry", SUCCESS, res);

	// 5) creating another mapping for SEC in the new L1
	attrs = 0x12; // 0b1--10 // Section: non useful since already set by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	printf("Va %x Pa %x \n", va , pa);
	expect(++t_id,"Successful map of the new page", SUCCESS, res);

	// 6) preparing an L2
	for(j = 0; j < 1024; j++)
		l2[j] = ((uint32_t)0x0);
	memcpy((void*)va, l2, sizeof l2);

	// 7) Unmapping the SEC section
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);

	// 8) calling Create_L2 API which should succeed
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"SUCCESSFUL creation of a new L2", SUCCESS, res);
}
void unit_test()
{
 //############
}
void _main()
{
	uint32_t p0, p1, p2 , p3;
    __asm__ volatile (
         "mov %0, r3\n"
         "mov %1, r4\n"
         "mov %2, r5\n"
         "mov %3, r6\n"
         : "=r"(p0), "=r"(p1), "=r"(p2), "=r"(p3) : );

	pstart = p0;
	vstart = p1;
	psize = 0xFA11;
	fwsize= 0xFA11;

	va_base = vstart;

	printf("START TEST\n");
	printf("Received parameters: pstart= %x vstart=%x psize=%x fwsize=%x\n", pstart, vstart, psize, fwsize);



  int j;
  for(j = 0; j < 500000; j++) asm("nop");
  test_l1_create();
#ifdef TEST_DMMU_MAP_L1_SECTION
  test_map_l1_section();
#endif
#ifdef TEST_DMMU_UNMAP_L1_ENTRY
  test_unmap_l1_entry();
#endif
#ifdef TEST_DMMU_CREATE_L1
  test_l1_create();
#endif
#ifdef TEST_DMMU_CREATE_L1_EMPTY_L1
  test_l1_create_empty_l1();
#endif
#ifdef TEST_DMMU_CREATE_AND_SWITCH_L1
  test_l1_create_and_switch_l1();
#endif
#ifdef TEST_DMMU_CREATE_L2
  test_l2_create();
#endif
#ifdef TEST_DMMU_MAP_L1_PT
  test_l1_pt_map();
#endif
#ifdef TEST_DMMU_L2_MAP_ENTRY
  test_l2_map_entry();
#endif
#ifdef TEST_DMMU_L2_UNMAP_ENTRY
  test_l2_unmap_entry();
#endif
#ifdef TEST_DMMU_L2_UNMAP_PT
  test_l2_unmap_pt();
#endif
#ifdef TEST_DMMU_SWITCH_L1
  test_switch_mm();
#endif
#ifdef TEST_DMMU_UNMAP_L1_PT
  test_unmap_L1_pt();
#endif
  printf("TEST COMPLETED\n");
}

/*Each guest must provide a handler rpc*/
void handler_rpc(unsigned callNum, void *params)
{
    
}
