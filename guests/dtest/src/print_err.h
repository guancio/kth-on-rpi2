
#ifndef PRINT_ERR_H_
#define PRINT_ERR_H_

const char* MSG[] =
{"SUCCEED!",
 "ERR_MMU_RESERVED_VA",
 "ERR_MMU_ENTRY_UNMAPPED",
 "ERR_MMU_OUT_OF_RANGE_PA",
 "ERR_MMU_SECTION_NOT_UNMAPPED",
 "ERR_MMU_PH_BLOCK_NOT_WRITABLE",
 "ERR_MMU_AP_UNSUPPORTED",
 "ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED",
 "ERR_MMU_ALREADY_L1/L2_PT",
 "ERR_MMU_SANITY_CHECK_FAILED",
 "ERR_MMU_REFERENCED_OR_PT_REGION",
 "ERR_MMU_NO_UPDATE",
 "ERR_MMU_IS_NOT_L2_PT",
 "ERR_MMU_XN_BIT_IS_ON",
 "ERR_MMU_PT_NOT_UNMAPPED",
 "ERR_MMU_REF_OVERFLOW",
 "ERR_MMU_INCOMPATIBLE_AP",
 "ERR_MMU_L2_UNSUPPORTED_DESC_TYPE",
 "ERR_MMU_REFERENCE_L2",
 "ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED",
 "ERR_MMU_IS_NOT_L1_PT",
 "ERR_MMU_UNIMPLEMENTED"
};

#define print_2_err(test_id,  api_name, addr, err_num)\
	if(err_num == 0)\
		printf("test ID: %d,  %s :) pt_base/va = %x, reason: %s \n",test_id , api_name, addr , MSG[err_num]);\
	else\
		printf("test ID: %d,  %s failed. pt_base/va =  %x, reason: %s \n",test_id, api_name, addr , MSG[err_num]);\


#define print_3_err(test_id, api_name, addr, addrOrIdx, err_num)\
	if(err_num == 0)\
		printf("test ID: %d,  %s :) pt_base/va = %x pg_base/index = %x , reason: %s \n",test_id , api_name, addr, addrOrIdx , MSG[err_num]);\
	else\
		printf("test ID: %d,  %s failed. pt_base/va = %x pg_base/index = %x , reason: %s \n",test_id, api_name, addr, addrOrIdx , MSG[err_num]);\


#endif /* PRINT_ERR_H_ */
