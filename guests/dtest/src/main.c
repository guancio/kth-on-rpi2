
#include <lib.h>
#include <types.h>

#include "dtest.h"

//#ifdef ALLTESTS
//#define TEST_DMMU_MAP_L1_SECTION
//#define TEST_DMMU_UNMAP_L1_ENTRY
//#define TEST_DMMU_CREATE_L1
//#define TEST_DMMU_CREATE_L1_EMPTY_L1
//#define TEST_DMMU_CREATE_AND_SWITCH_L1
//#define TEST_DMMU_CREATE_L2
//#define TEST_DMMU_MAP_L1_PT
//#define TEST_DMMU_L2_MAP_ENTRY
//#define TEST_DMMU_L2_UNMAP_ENTRY
//#define TEST_DMMU_L2_UNMAP_PT
//#define TEST_DMMU_SWITCH_L1
//#define TEST_DMMU_UNMAP_L1_PT
//#endif

//The initial memory layout of the dtest guest is something like 0xc0i00000
//mapped to base_pa+i, with i in [0..5] (with the exception of i=2, where the
//initial L1 is created).
//Moreover, the regions i=2 and i=3 are considered to be always cacheable.

//Each function consists of a test suite, which is executed after the guest is
//loaded by the hypervisor. At compile time, you can choose which test suite
//you want to run by making the hypervisor with "make TEST_NAME=TEST_UNDEFINED",
//where you switch "TEST_UNDEFINED" for the name of the test you want to execute.
//Do not forget to execute the command "make clean" between changing tests.

void test_map_l1_section()
{
	char * test_name= "MAP L1 SECTION";
	printf("Test suite: %s\n", test_name);

	//A virtual address, a physical address, attributes, and result of a test.
	uint32_t va, pa, attrs, res;
	int t_id = 0;

	//#0: We can not map 0, since it is reserved by the hypervisor to access the
	//guest page tables.
	va = 0x0;
	pa = 0x0;
 	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(t_id, "Mapping a reserved virtual address", ERR_MMU_RESERVED_VA, res);

	//#1: Mapping 0xc0200000 is OK, since it is the page containing the active
	//page table. However, this test should fail, because we are not allowed to
	//map to a physical address outside the allowed range of the guest.
	va = (va_base + 0x200000);
	pa = 0x0;
 	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping to a physical address outside the allowed range of the guest", ERR_MMU_OUT_OF_RANGE_PA, res);

	//#2: Mapping 0xc0000000 should fail, because this is already mapped.
	va = (va_base + 0x000000);
	pa = va2pa(va);
 	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping an already mapped L1 entry", ERR_MMU_SECTION_NOT_UNMAPPED, res);

	//#3: Mapping 0xc020000 is OK, since it is the page containing the active
	//page table. However, this test should fail, because the attribute must be
	//set to "cacheable".
	va = (va_base + 0x200000);
	pa = va2pa(va);
 	attrs = 0b0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping to a cacheable address without setting cacheability attribute", ERR_MMU_NOT_CACHEABLE, res);

	//#4: Mapping 0xc020000 is OK, since it is the page containing the active
	//page table. However, this test should fail, because it can not be set to
	//be writable.
	va = (va_base + 0x200000);
	pa = va2pa(va);
 	attrs = 0b1000;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping to address using unsupported access permissions", ERR_MMU_AP_UNSUPPORTED, res);

	//#5: Mapping 0xc020000 is OK, since it is the page containing the active
	//page table - this test should succeed.
	va = (va_base + 0x200000);
	pa = va2pa(va_base + 0x000000);
	//attrs = 0x12; // 0b1--10
	//attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	//attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
 	attrs = 0xc2e; //TODO: Fix hard-coded attributes.
 	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping a valid writable page", SUCCESS, res);

	//#6: Unmapping 0xc0300000 is allowed - this test should succeed.
	va = (va_base + 0x300000);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id, "Unmapping an existing mapped page", SUCCESS, res);

	//#7: Mapping 0xc0300000 with read-only access permission is OK - this test
	//should succeed.
	va = (va_base + 0x300000);
	pa = va2pa(va_base + 0x000000);
 	attrs = 0xb2e;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping a valid read-only page", SUCCESS, res);
}

void test_unmap_l1_entry()
{
	char * test_name= "UNMAP L1 ENTRY";
	printf("Test suite: %s\n", test_name);

	//A virtual address, a physical address, attributes, and result of test.
	uint32_t va,pa, res, attrs;
	int t_id = 0;

	//#0: We can not unmap 0x0, since it is reserved by the hypervisor to access
	//the guest page tables.
	va = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id, "Unmapping a reserved virtual address", ERR_MMU_RESERVED_VA, res);

	//#1: We can not unmap 0xf0000000, since it is reserved by the hypervisor
	//code.
	va = 0xf0000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id, "Unmapping a reserved virtual address", ERR_MMU_RESERVED_VA, res);

	//#2: We create an alias from (va_base + 0x200000) to the same address
	//pointed to by va_base (that is, va2pa(va_base)).
	va = (va_base + 0x200000);
	pa = va2pa(va_base);
	attrs = 0x12; // 0b1--10
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Mapping a valid writable page", SUCCESS, res);

	//#3: Unmapping 0xc0200000 is allowed - this test should succeed.
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id, "Unmapping a valid writable page", SUCCESS, res);

	//#4: Unmapping 0xc0200000 has no effect, since this page is unmapped
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id, "Unmapping an unmapped entry", ERR_MMU_ENTRY_UNMAPPED, res);

}

void test_l2_create()
{
	char * test_name= "CREATE L2";
	printf("Test suite: %s\n", test_name);

	//A virtual address, a physical address, attributes, and result of test.
	uint32_t pa, va, attrs, res, desc;
	int j, t_id = 0;	

	/*	  // #0 : Guest can not write its l2 page table in an unmapped area
	// This test will break the system (Dabort)
	va = (va_base | (uint32_t)0x300000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id, "Unamap of a reserved va", ERR_MMU_RESERVED_VA, res);
	memset((void *)va, 0x32, 4096*4);
	*/

	//#1: We can not write our own L2 page table in a physical address outside 
	//the allowed range. This test should fail, because physical address 0x0 is
	//not accessible to the guest.
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id, "Mapping to a physical address outside the allowed range of the guest", ERR_MMU_OUT_OF_RANGE_PA, res);

	//#2: We can not use an address which is not 4 KiB aligned to create an L2
	//page table. This test should fail, because we are only allowed to create
	//a L2 page table in a 4 KiB-aligned address.
	pa = va2pa(va_base + 0x204100);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id, "Mapping to a physical address which is not 4 KiB-aligned", ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED, res);

	//#3: We can not create a new L2 page table in a region which already
	//contains an L2.
	va = (va_base | (uint32_t)0x3b0000) ;
	pa = va2pa(va);
	for(j = 0; j < 1024; j++){
		l2[j] = ((uint32_t)0x0);
	}
	memcpy((void*)va, l2, sizeof l2);
	//TODO: Allow sub-tests?
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id,"Unmapping an existing L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id,"Creating a L2 page table", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id,"Creating a L2 page table for a region of memory which already has a L2 page table", ERR_MMU_ALREADY_L2_PT, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(++t_id,"Freeing the above L2 page table", SUCCESS, res);

	//#4: We can not create a new L2 page table in a region which already
	//contains a page table or a referenced data page.
	//we first remap the unmapped page
	attrs = 0xc2e; //TODO: Hard-coded attribute.
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(t_id, "Remapping a writable L1 page", SUCCESS, res);

	va = (va_base | (uint32_t)0x320000);
	pa = va2pa(va);
	for(j = 0; j < 1024; j++){
	  	l2[j] = ((uint32_t)0x0);
	}
	memcpy((void*)va, l2, sizeof l2);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id,"Unmapping the section which points to the area where we want to create an L2", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id,"Creating a L2 page table", SUCCESS, res);

	//The data page address
	//(NOTE: must be located in the always cacheable region)
	uint32_t pga = va2pa(va_base + 0x330000);
	uint32_t idx = 0xc2;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, 0x32 | 0b1000);
	expect(t_id,"Creating a mapping from the new L2 page table to a data block", SUCCESS, res);

	//Here, pga is pointing to a referenced data page.
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pga, 0, 0);
	expect(++t_id,"Creating a L2 page table on the (writable) block which is pointed to by the previously allocated L2 page table", ERR_MMU_REFERENCED, res);

	//#5: We can not create a new L2 page table with an unsupported descriptor
	//type (0b11). First, we must free the previously created L2 and remap the
	//section.
	va = (va_base | (uint32_t)0x320000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(++t_id,"Freeing the memory hosting a previously allocated L2", SUCCESS, res);

 	attrs = 0xc2e;
 	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id, "Remapping a writable page", SUCCESS, res);

	attrs = 0x12; // 0b1--10 // Section: not useful since already set by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	va = (va_base + 0x300000) ;
	pa = va2pa(va);
	desc = (pstart | 0x31);
	for(j = 0; j < 1024; j++){
		l2[j] = ((uint32_t)desc);
	}
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"Creating a L2 page table with an unsupported descriptor type (0b11)", ERR_MMU_L2_UNSUPPORTED_DESC_TYPE, res);

	//#6: We can not create a new L2 page table with an entry which points to
	//the L2 page table itself with write access permission.
	va = (va_base + 0x300000) ;
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(t_id, "Creating a writable section", SUCCESS, res);
	desc = (pa & 0xffff0000) | 0x32 | 0b1000;
	for(j = 0; j < 1024; j++)
		 l2[j] = ((uint32_t)desc);
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"Creating a L2 page table located in a writable section", ERR_MMU_NEW_L2_NOW_WRITABLE, res);
	ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);

	//#7: We can create a new L2 with an entry which point to another page
	//table with read-only access permission.
	va = (va_base + 0x300000) ;
	pa = va2pa(va);
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	desc = (pa & 0xffff0000) | 0x22 | 0b1000;
	for(j = 0; j < 1024; j++)
		  l2[j] = ((uint32_t)desc); //self reference with ap = 2, it succeed
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"Creating a new L2 page table with an entry which points to another page table with read-only access permission", SUCCESS, res);
	ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);

	//#8: We can not create a new L2 with an entry which points to another page
	//table with write access permission.
	va = (va_base + 0x300000) ;
	pa = va2pa(va);
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	desc = ((pstart & 0xff000000)| (1 << 23) | 0x32 | 0b1000);
	for(j = 0; j < 1024; j++){
		  l2[j] = ((uint32_t)desc);
	}
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"Creating a new L2 page table with an entry which points to another page table with write access permission", ERR_MMU_PH_BLOCK_NOT_WRITABLE, res);
	ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);


}

void test_l1_pt_map()
{
	char * test_name= "MAP L1 PAGE TABLE";
	printf("Test suite: %s\n", test_name);

	//A virtual address, a physical address, attributes, and result of test.
	uint32_t pa, va, attrs, res, desc;
	int j, t_id = 0;

	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT;
	va = (va_base + (uint32_t)0x300000);
	pa = va2pa(va);
	desc = (pa & 0xffff0000) | 0x22 | 0b1000;
	for(j = 0; j < 1024; j++){
		l2[j] = ((uint32_t)desc);
	}
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res= ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id,"Creating a new L2 page table", SUCCESS, res);

	//#1: We can not map 0, since it is reserved by the hypervisor to access the
	//guest page tables.
	va = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(++t_id,"Creating a L1 entry for a virtual address which is reserved for hypervisor use", ERR_MMU_RESERVED_VA, res);

	//#2: Mapping 0xc0200000 is OK, since it is the page containing the active
	//page table. This test should fail, because we are not allowed to map to a
	//physical address outside the allowed range of the guest.
	va = (va_base | (uint32_t)0x200000);
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(++t_id,"Mapping to a physical address which is outside the allowed range of the guest", ERR_MMU_OUT_OF_RANGE_PA, res);

	//#3: Mapping 0xc0200000 is OK, since it is the page containing the active
	//page table. This test should fail, because the given address does not
	//point to a valid L2 page table.
	attrs = 0x0;
	va = (va_base | (uint32_t)0x210000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(++t_id,"Mapping a non-page table page in one of the the active L1 page tables as a L2 page table", ERR_MMU_IS_NOT_L2_PT, res);

	//#4: Mapping 0xc0300000 is OK, since it is a page containing a valid L2
	//This test should fail, because PXN is enabled
	attrs = 0x24;
	va = (va_base | (uint32_t)0x300000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(++t_id,"Mapping an entry using a descriptor for which PXN has been enabled", ERR_MMU_XN_BIT_IS_ON, res);

	//#5: Mapping 0xc0300000 is OK, since it is a page containing a valid L2.
	//However, this test should fail, because we can not map an L2 table table
	//in a given entry two times in a row.
	attrs = 0x20;
	va = (va_base | (uint32_t)0x300000);
	pa = va2pa(va);
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(++t_id,"Mapping an L1 entry twice", ERR_MMU_PT_NOT_UNMAPPED, res);

}

void test_l2_map_entry()
{
	char * test_name= "L2 MAP ENTRY";
	printf("Test suite: %s\n", test_name);

	// idx is the index of a entry we want to map pga into
	uint32_t pa, va, idx, pga, attrs, res, desc;
	int j, t_id = 0;

	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT | 0b1000;
	va = (va_base | (uint32_t)0x300000);
	pa = va2pa(va);
	desc = (pa & 0xffff0000) | 0x22 | 0b1000;
	for(j = 0; j < 1024; j++){
		l2[j] = ((uint32_t)desc);
	}
	memcpy((void*)va, l2, sizeof l2);

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id,"Unmapping a L1 page table entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"Creating a L2 page table", SUCCESS, res);

	//#0: L2 base can not be 0x0, since it is reserved by the hypervisor for
	//access to the guest page tables.
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping an entry of an invalid L2, in physical address outside the allowed range", ERR_MMU_OUT_OF_RANGE_PA, res); //TODO: What does this mean?

	//#1: L2 base address is OK, but guest can not map a page outside the
	//allowed range into its L2 page table entries.
	pga = 0x0;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping to a physical address outside the allowed range of the guest", ERR_MMU_OUT_OF_RANGE_PA, res);

	//#2: All the parameters are correct and this test should succeed
	va = (va_base | (uint32_t)0x300000);
	pa = va2pa(va);
	pga = va2pa((va_base | (uint32_t)0x310000));
	idx = 0xc2;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an entry of a valid L2 page table", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping a valid physical address into one of a valid L2 page table", SUCCESS, res);

	//#3: this test should fail, because the entry has already been mapped.
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping an L2 page table entry twice", ERR_MMU_PT_NOT_UNMAPPED, res);

	//#4: this test should fail, because guest can not map a page table to an entry of the given L2 with writable access permission
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pa, attrs);
	expect(++t_id,"Mapping a page table onto one of the L2 entries with write access permission", ERR_MMU_NEW_L2_NOW_WRITABLE, res);

	//#5: This test should fail, because we can not map anything to an entry of
	//a data page. //TODO: Fails with wrong failure...
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pga, idx, pga, attrs);
	expect(++t_id,"Mapping an entry of a data page", ERR_MMU_IS_NOT_L2_PT, res);

	//#6: this test should fail, because guest is passing an unsupported  access permission
	attrs = 0x02 | 0b1000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping the given entry of the valid L2", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping an entry of L2 using an unsupported access permission", ERR_MMU_AP_UNSUPPORTED, res);

	//#7: All the parameters are correct and this test should succeed
	//in this test reference counter of the mapped page should not be increased
	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT | 0b1000;
	pga = va2pa((va_base | (uint32_t)0x310000));
	idx = 0xab;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an entry of a valid L2", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	expect(++t_id,"Mapping an entry of L2 with valid parameters", SUCCESS, res);

	//#8: All the parameters are correct and this test should succeed
	//in this test guest is mapping the L2 base address into one of L2 entry with read-only access permission
	idx = 0xac;
	attrs = 0;
	attrs |= MMU_AP_USER_RO << MMU_PT_AP_SHIFT | 0b1000;
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pa, attrs);
	expect(++t_id,"Mapping the base address of the given L2 page table into an entry of the L2 page table with read-only access permission", SUCCESS, res);

}

void test_l2_unmap_entry()
{
	char * test_name= "L2 PAGE TABLE UNMAP ENTRY";
	printf("Test suite: %s\n", test_name);

	// idx is the index of a entry we want to map pga into
	uint32_t pa, va, idx, pga, attrs, res, desc;
	int j, t_id = 0;

	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_PT_AP_SHIFT;
	va = (va_base | (uint32_t)0x300000);
	pa = va2pa(va);
	desc = (pa & 0xffff0000) | 0x22 | 0b1000;
	for(j = 0; j < 1024; j++){
		l2[j] = ((uint32_t)desc);
	}
	memcpy((void*)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res= ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id,"Creating a new L2", SUCCESS, res);

	//#0: L2 base address can not be 0x0, since it is reserved by the hypervisor
	//to access the guest page tables.
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(t_id,"Unmapping an entry of an invalid L2 for which the base address is outside the allowed range", ERR_MMU_OUT_OF_RANGE_PA, res);


	//#1: We are trying to unmap entry of a data page.
	//This test should fail because the L2 base address is not pointing to a valid page table(L2)
	pa = va2pa(va_base | (uint32_t)0x310000);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an entry of an invalid L2, page type associated with the physical address is not L2-page table", ERR_MMU_IS_NOT_L2_PT, res);

	//#2: The entry guest is trying to unmap an entry that is not mapped
	//This test should succeed
	idx = 0x0;
	pa = va2pa((va_base | (uint32_t)0x300000));
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an unmapped entry of an L2 page table", SUCCESS, res);

	//#3: All the parameters are well defined
	//This test should succeed but the reference counter should remain untouched
	idx = 0x0;
	attrs = 0;
	attrs |= MMU_AP_USER_RO << MMU_PT_AP_SHIFT;
	ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pa, attrs);

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an entry of L2 page table which points to the L2 page table itself", SUCCESS, res);

	//#4: This test is a successful attempt
	idx = 0xc2;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	expect(++t_id,"Unmapping an entry of L2 page table which points to a data page", SUCCESS, res);
}

void test_l2_unmap_pt()
{
	char * test_name= "L2 PAGE TABLE UNMAP PAGE TABLE";
	printf("Test suite: %s\n", test_name);
	
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
	va = (va_base | (uint32_t)0x300000);
	pa = va2pa(va);
	pga = va2pa(va_base |(uint32_t)0x400000);
	for(j = 0; j < 1024; j++){
		(*(((uint32_t *)va)+j)) = ((uint32_t)0x0);
	}
	res=ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id,"Unmapping the L1 page table entry to contain the new L2 page table", SUCCESS, res);

	res= ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(t_id++,"Creating a new L2 page table", SUCCESS, res);

	for (idx = 0; idx < 1024; idx++){
		res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
		expect(t_id++,"Updating a L2 page table entry", SUCCESS, res);
	}

	attrs = 0x0;
	attrs |= (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	expect(t_id++,"Mapping the L1 page table to the new L2 page table", SUCCESS, res);

	// now the va is mapped and is translated to
	// va and va + 4kb*i are pointing to the same address
	//(*((uint32_t *)va)) = 666;
	(*((uint32_t *)(va + 2*4096))) = 666;
	uint32_t val = (*((uint32_t *)(va + 4096)));
	expect(t_id++,"Confirming that va and va + 4 KiB point to the same value, where va is mapped to by the new L2 page table", 666, val);


	// is true that va is pointing to va2pa(va_base | (uint32_t)0x400000)?
	// We build an additional section mapping to the address va2pa(va_base | (uint32_t)0x400000)
	// then we read/write
	attrs = 0;
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs |= (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	uint32_t va_section =  va_base + 0x200000;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va_section, pga, attrs);
	expect(t_id++,"Setting up aliasing of a virtual address using a section", SUCCESS, res);

	val = (*((uint32_t *)(va_section)));
	expect(t_id++,"Confirming that aliasing of a virtual address using a section works", 666, val);

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va_section, 0, 0);
	expect(t_id++,"Unmapping the section aliasing the virtual address", SUCCESS, res);
	// finished


	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(t_id,"Freeing a referenced L2 page table", ERR_MMU_REFERENCE_L2, res);

    //#1: This test should succeed
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id,"Unmapping the L1 entry pointing to the new L2", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(t_id,"Freeing the given L2", SUCCESS, res);

	//#2: This test should fail because L2 base address can not be 0x0, since it
	//is reserved by the hypervisor to access the guest page tables.
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(t_id,"Freeing a L2 page table with a base address outside the allowed range of the guest", ERR_MMU_OUT_OF_RANGE_PA, res);

	//#3: This test should fail, because L2 base address does not point to a
	//valid L2.
	pa = va2pa(va_base | (uint32_t)0x110000);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	expect(t_id,"Freeing a L2 page table at base address not pointing to an L2 page table", ERR_MMU_IS_NOT_L2_PT, res);
}

void test_l1_create()
{
	char * test_name= "CREATE L1";
	printf("Test suite: %s\n", test_name);

	uint32_t pa, va, attrs, res;
	int j, t_id = 0;

	//#0: this test should fail because we are trying to create a new page table
	//in a part of the memory which is reserved for hypervisor use.
	//pa = 0x80000000; //<-- Beagleboard value TODO: Should be equal to HAL_PHYS_START for the board in question... How fix?
	pa = 0x1000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(t_id, "Creating an L1 page table in the hypervisor memory", ERR_MMU_OUT_OF_RANGE_PA, res);

	//#1: This test should fail because L1 base is not 16 KiB-aligned
	pa = va2pa(va_base + 0x205000);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id, "Creating an L1 page table at a non-16 KiB-aligned memory address", ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED, res);

	//#2: This test should fail because guest is trying to create a new page
	//table in a part of the memory where another L1 resides.
	pa = va2pa(va_base + 0x200000);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Creating an L1 page table in a memory area where another L1 page table resides", ERR_MMU_ALREADY_L1_PT, res);

	//#3: Test that we can not create an L1 in a referenced page.
	memset(l1, 0, 4096*4);
	pa = va2pa(va_base + 0x300000);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Creating a L1 page table in a referenced part of the memory", ERR_MMU_REFERENCED, res);

	//#4: Test that we can not create an L1 in a region that is not always
	//cacheable.
	pa = va2pa(va_base);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Creating a L1 page table in memory which is not always-cacheable", ERR_MMU_OUT_OF_CACHEABLE_RANGE, res);

	// From here we always map a section, write the page table and then unmap the section.
	// Finally we can call the API

	//#4: This test should fail because we are trying to map a non-L2 page table
	//page into L1 as a L2 page table.
	attrs = 0x12; // 0b1--10 // Section: non useful since already setted by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	va = (va_base + 0x300000);
	pa = va2pa(va);
	// Writing content of the new L1 page table
	uint32_t desc = (pstart | 0x1);
	memset(l1, 0, 4096*4);
	l1[512] = ((uint32_t)desc); // L2 descriptor
	memcpy((void*)va, l1, sizeof l1);
	// unmap the section
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Unmapping an L1 page table entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Creating an L1 page table using an L2 page table which has not been previously validated", ERR_MMU_IS_NOT_L2_PT, res);

	//#5: This test should fail because we are trying to use supersection
	//descriptor instead of a section descriptor.
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Mapping a new valid L1 page", SUCCESS, res);

	desc = (pstart | (1 << 18) | 0x2);
	memset(l1, 0, 4096*4);
	l1[512] = ((uint32_t)desc); // Supersection
	memcpy((void*)va, l1, sizeof l1);
	// unmap the section
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Unmapping an L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Creating an L1 page table containing a supersection", ERR_MMU_SUPERSECTION, res);

	//#6: This test should fail because we are trying to use an unsupported
	//access permission in a section descriptor.
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Mapping a new valid L1 page", SUCCESS, res);

	desc = (pstart | 0x2);
	memset(l1, 0, 4096*4);
	l1[512] = ((uint32_t)desc); // L2 descriptor (ap = 0 it is unsupported)
	memcpy((void*)va, l1, sizeof l1);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Unmapping an L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Creating an L1 page table using an unsupported access permission", ERR_MMU_AP_UNSUPPORTED, res);

	//#7: this test should fail because guest is trying to map part of the memory as a writable section where I initial page table of guest resides
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Mapping a new valid L1 page", SUCCESS, res);

	desc = ((pstart & 0xff000000)| (1 << 23) | 0xC2E);
	memset(l1, 0, 4096*4);
	l1[512] = ((uint32_t)desc); // It points to an L1 (the current one)
	memcpy((void*)va, l1, sizeof l1);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Unmapping an L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Creating an L1 entry where a writable section points to another L1 page table", ERR_MMU_PH_BLOCK_NOT_WRITABLE, res);

	//#8: this test should fail because guest is trying to map part of the memory as a writable section where it is trying to create the new L1
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Mapping a new valid L1 page", SUCCESS, res);

	memset(l1, 0, 4096*4);
	// Notice we make this cacheable
	l1[512] = (((uint32_t)pa) & 0xFFF00000) | (uint32_t)0xC02 | (uint32_t) 0b1000;
	memcpy((void*)va, l1, sizeof l1);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Unmapping an L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Creating a L1 page table where a writable section points to the L1 being allocated", ERR_MMU_NEW_L1_NOW_WRITABLE, res);


	// Test that we do not allow noncacheable sections to point to the always cacheable region
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Mapping a new valid L1 page", SUCCESS, res);

	memset(l1, 0, 4096*4);
	// Notice we make this cacheable
	l1[512] = (((uint32_t)pa) & 0xFFF00000) | (uint32_t)0x802;
	memcpy((void*)va, l1, sizeof l1);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Unmapping an L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(++t_id,"Creating an L1 page table where a non-cacheable section points to the always cacheable region", ERR_MMU_NOT_CACHEABLE, res);
	//TODO: check that the L1 contains a writable section that points outside the guest memory

}

void test_l1_create_empty_l1() {
	char * test_name= "CREATE L1 EMPTY L1";
	printf("Test suite: %s\n", test_name);

	uint32_t pa, va, attrs, res;
	int j, t_id = 0;
	// creating a writable section to map
	// for this test minimal_config.c has been modified and now ".pa_for_pt_access_end = HAL_PHYS_START + 0x014fffff"
	attrs = 0x12; // 0b1--10 // Section: non useful since already setted by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	va = (va_base + 0x300000);
	pa = va2pa(va);

	// Map the pa thus we are able to store the pagetable
	memset(l1, 0, 4096*4);
	memcpy((void*)va, l1, sizeof l1);

	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id++,"Unmapping an L1 entry", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(t_id++,"Creating an L1 page table with only one (writable) section which points to a data page", SUCCESS, res);
}

void test_l1_create_and_switch_l1() {
	// Test summary:
	// - We map a new section
	// - We write some random data into this section using some va
	// - We map a new section for the new L1
	// - We create a second L1 as follows:
	//    - the first 0xc000 is mapped as usual since it contains the test code
	//    - the second a "random" address points to the page where we written the data
	// - We unmap the section of the new L1
	// - We switch to the create L1
	// - We read the written data

	char * test_name= "CREATE AND SWITCH L1";
	printf("Test suite: %s\n", test_name);

	uint32_t pa, va, attrs, res;
	int j, t_id = 0;

	// 1) creating a writable section to map (0xc0300000). Here we write the data
	attrs = 0x12; // 0b1--10 // Section: non useful since already setted by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	va = (va_base + 0x400000);
	pa = va2pa(va);

	// 2) write some data
	*((uint32_t*)(va+1024)) = (uint32_t)666;
	expect(++t_id,"Writing to section", 666, *((uint32_t*)(va+1024)));

	// 3) creating a writable section to map (0xc0400000). Here we write the L1
	attrs = 0x12; // 0b1--10
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);

	va = (va_base + 0x300000);
	pa = va2pa(va);


	// 4) create the new L1 (initially empty)
	memset(l1, 0, 4096*4);

	// Grant to the guest a writable part of the memory
	l1[3072] = (((uint32_t)va2pa(0xc0000000)) & 0xFFF00000) | (uint32_t)0xC2E;
	l1[513] = (((uint32_t)va2pa(0xc0100000)) & 0xFFF00000) | (uint32_t)0x82C;
	memcpy((void*)va, l1, sizeof l1);

	// 5) Un-mapping the new L1 section
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(++t_id,"Unmapping an entry of the L1 page table which points to a section where the new L1 page table resides", SUCCESS, res);

	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	expect(t_id++,"Creating an L1 page table with only one (writable) section, which points to a data page", SUCCESS, res);

	// 6) switching to the new L1
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	expect(++t_id,"Switching to a new L1 page table", SUCCESS, res);

	// trying to read the content of the modified page under the new page table
	va = (va_base + 0x400000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Mapping a new valid page", SUCCESS, res);

    // trying to read the content of the suspended page table under the new page table
    va = (va_base + 0x200000);
    pa = va2pa(va);
    attrs = 0x12; // 0b1--10
	attrs |= MMU_AP_USER_RO << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Mapping a new valid page", SUCCESS, res);
	//TODO: This bottom section causes data aborts, followed by prefetch aborts...
    //printf("Suspended l1 content  desc %x \n",  *(((uint32_t *)va)));
	/*
	uint32_t index = 0;
	for(index = 0; index < 4096; index++)
	{
		if(*(((uint32_t *)va)+ index) != 0x0)
			printf("Suspended l1 content index %x desc %x \n", index, *(((uint32_t *)va) + index));
	}*/
}


void test_switch_mm()
{
	char * test_name= "SWITCH L1";
	printf("Test suite: %s\n", test_name);

	uint32_t pa, va, attrs, res;
	int j, t_id = 0;

	//#1: This test should fail because we are trying to create a new page table
	//in a part of the memory which is reserved for hypervisor use.
	//pa = 0x80000000; <-- Beagleboard value //TODO: Should be equal to HAL_START_PHYS
	pa = 0x1000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	expect(++t_id,"Failing to switch to the given physical address which is out of guest allowed  memory range", ERR_MMU_OUT_OF_RANGE_PA, res);

	//#2: This test should fail, because L1 base address is not 16 KiB-aligned.
	pa = va2pa(va_base + 0x101000);
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	expect(++t_id,"Failing to switch to the given L1 physical address which not 16 KiB-aligned", ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED, res);

	//#3: This test should fail because we are trying to switch into a non-page
	//table page.
	pa = va2pa(va_base + 0x100000);
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	expect(++t_id,"Failing to switch to the given physical address because guest is trying to switch into a non-page table page", ERR_MMU_IS_NOT_L1_PT, res);

	test_l1_create_and_switch_l1();
}

void test_unmap_L1_pt()
{
	char * test_name= "UNMAP L1 PAGE TABLE";
	printf("Test suite: %s\n", test_name);

	uint32_t pa, va, attrs, res;
	int j, t_id = 0;

	//#0: This test should fail, because we are trying to create a new page
	//table in a part of the memory which is reserved for hypervisor use.
	//pa = 0x80000000; <-- Beagleboard value //TODO: Should be equal to HAL_START_PHYS
	pa = 0x1000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unmapping a physical address outside the allowed range of the guest", ERR_MMU_OUT_OF_RANGE_PA, res);

	//#1: This test should fail, because base address is not 16 KiB-aligned.
	pa = va2pa(va_base + 0x101000);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unmapping a physical address which is not 16 KiB-aligned", ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED, res);

	//#2: This test should fail, because base address is pointing to the active
	//L1 page table.
	pa = va2pa(va_base + 0x200000);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unmapping the active L1 page table", ERR_MMU_FREE_ACTIVE_L1, res);

	//#3: This test should fail, because base address is not pointing to a L1
	//page table.
	pa = va2pa(va_base + 0x1C0000);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unmapping a physical address which is not pointing to a L1 page table", ERR_MMU_IS_NOT_L1_PT, res);

	/*// Scenario for the following test case
	   - Map a section, call it SEC, with write access permission (this should increase the reference counter of the involved page by 1)
	   - Create a new L1
	   - Switch to the new L1
	   - Free the suspended L1
	   - Create another mapping for SEC in the new L1
	   - Prepare an L2
	   - Unmap the SEC section
	   - Call Create_L2 API which should succeed
	*/
	// 1) mapping a section (0x81300000 to 0xc0300000), call it SEC, with writable access permission(this should increase the reference counter of the involved page by 1)
	// 2) Creating a new L1
	// 3) Switching to the new L1
	test_l1_create_and_switch_l1();

	//4) Freeing the initial L1 page table
	pa = va2pa(va_base + 0x200000); //9F800000
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	expect(t_id, "Unmapping the guest initial master page table", SUCCESS, res);

	//Unmapping the read-only mapping of the section
	va = (va_base + 0x200000);
	pa = va2pa(va);
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	expect(t_id++,"Unmapping the L1 entry", SUCCESS, res);

	//5) creating another mapping for SEC in the new L1
	attrs = 0x12; // 0b1--10 // Section: not useful since already set by the API
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	expect(++t_id,"Mapping a new valid page", SUCCESS, res);

	//6) preparing an L2
	for(j = 0; j < 1024; j++){
		l2[j] = ((uint32_t)0x0);
	}
	memcpy((void*)va, l2, sizeof l2);

	//7) Unmapping the SEC section
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);

	//8) calling Create_L2 API which should succeed
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	expect(++t_id,"Creating a new L2", SUCCESS, res);
}

void unit_test()
{
	 //TODO: Empty...
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
	printf("You are now inside the 'dtest' guest, which can be configured at compile time to run several tests of the hypervisor.\n");
	printf("Received parameters: pstart=%x vstart=%x psize=%x fwsize=%x\n", pstart, vstart, psize, fwsize);
	printf("START OF TEST\n");

	int j;
	for(j = 0; j < 500000; j++){
		asm("nop");
	}
  //*****************//
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
#ifdef TEST_UNDEFINED
  //Base case: if no test has been specified
  printf("No test has been specified. Do so by executing 'make TEST_NAME=TEST_UNDEFINED' when making the hypervisor, where you exchange 'TEST_UNDEFINED' for the name of the test you want to run. Valid names of tests can be found near the bottom of guests/dtest/src/main.c. Do not forget to execute 'make clean' in-between changing of tests.\n");
#endif
  printf("TEST COMPLETED\n");
}

/*Each guest must provide a handler rpc*/
void handler_rpc(unsigned callNum, void *params)
{
    //TODO: Empty...
}
