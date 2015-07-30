
#ifndef PRINT_ERR_H_
#define PRINT_ERR_H_


#define SUCCESS                             (0)
/* Error messages */
#define ERR_MMU_RESERVED_VA                 (1)
#define ERR_MMU_ENTRY_UNMAPPED              (2)
#define ERR_MMU_OUT_OF_RANGE_PA             (3)
#define ERR_MMU_SECTION_NOT_UNMAPPED        (4)
#define ERR_MMU_PH_BLOCK_NOT_WRITABLE       (5)
#define ERR_MMU_AP_UNSUPPORTED              (6)
#define ERR_MMU_BASE_ADDRESS_IS_NOT_ALIGNED (7)
#define ERR_MMU_ALREADY_L1_PT               (8)
#define ERR_MMU_ALREADY_L2_PT               (8)
#define ERR_MMU_PT_REGION		            (10)
#define ERR_MMU_NO_UPDATE                   (11)
#define ERR_MMU_IS_NOT_L2_PT                (12)
#define ERR_MMU_XN_BIT_IS_ON                (13)
#define ERR_MMU_PT_NOT_UNMAPPED             (14)
#define ERR_MMU_REF_OVERFLOW                (15)
#define ERR_MMU_INCOMPATIBLE_AP             (16)
#define ERR_MMU_L2_UNSUPPORTED_DESC_TYPE    (17)
#define ERR_MMU_REFERENCE_L2                (18)
#define ERR_MMU_L1_BASE_IS_NOT_16KB_ALIGNED (19)
#define ERR_MMU_IS_NOT_L1_PT                (20)
#define ERR_MMU_REFERENCED			        (21)
#define ERR_MMU_FREE_ACTIVE_L1				(22)
#define ERR_MMU_SUPERSECTION				(23)
#define ERR_MMU_NEW_L1_NOW_WRITABLE			(24)
#define ERR_MMU_L2_BASE_OUT_OF_RANGE        (25)
#define ERR_MMU_NOT_CACHEABLE               (26)
#define ERR_MMU_OUT_OF_CACHEABLE_RANGE      (27)
#define ERR_MMU_NEW_L2_NOW_WRITABLE	        (28)
#define ERR_MMU_UNIMPLEMENTED               (-1)

void expect(uint32_t test_id, char * msg, uint32_t value, uint32_t res) {
	if(value == res) {
		printf("Test ID: %d,  %s [SUCCESS] value: %d \n",test_id , msg, value);
		return;
	}
	printf("Test ID: %d,  %s [FAIL] expected: %d result: %d\n",test_id , msg, value, res);
}


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
