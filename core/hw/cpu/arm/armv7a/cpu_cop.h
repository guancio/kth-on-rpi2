
#ifndef _CPU_COP_H_
#define _CPU_COP_H_

/*CP10*/
#define COP_VFP_READ "cr0, cr0, 0"

/*CP15*/
#define COP_ID_CPU "c0, c0, 0"
#define COP_ID_CACHE_CONTROL_ARMv7_CLIDR "c0, c0, 1"
#define COP_ID_MEMORY_MODEL_FEAT "c0,c1,4"

#define COP_SYSTEM_CONTROL "c1, c0, 0"
#define COP_CPACR  "c1, c0, 2"

#define COP_SYSTEM_CONTROL_MMU_ENABLE (1 << 0)
#define COP_SYSTEM_CONTROL_DCACHE_ENABLE (1 << 2)
#define COP_SYSTEM_CONTROL_ICACHE_ENABLE 0x00001000

#define COP_SYSTEM_TRANSLATION_TABLE0 "c2, c0, 0"
#define COP_SYSTEM_TRANSLATION_TABLE1 "c2, c0, 1"
#define COP_SYSTEM_DOMAIN "c3, c0, 0"


#define COP_ICACHE_INVALIDATE_ALL "c7, c5, 0"

#define COP_BRANCH_PRED_INVAL_ALL "c7, c5, 6"

#define COP_DCACHE_INVALIDATE_ALL "c7, c6, 0"

#define COP_DCACHE_INVALIDATE_MVA "c7, c10, 1"
#define COP_DCACHE_INVALIDATE_SW "c7, c10, 2"
#define COP_DCACHE_CLEAN_INVALIDATE_MVA "c7, c14, 1"
#define COP_DCACHE_CLEAN_INVALIDATE_SW "c7, c14, 2"

#define COP_V2P_PAR				"c7, c4, 0"
#define COP_V2P_CWPR			"c7, c8, 0"
#define COP_V2P_CWPW			"c7, c8, 1"
#define COP_V2P_CWUR			"c7, c8, 2"
#define COP_V2P_CWUW			"c7, c8, 3"


#define COP_TLB_INVALIDATE_ALL "c8, c7, 0"
#define COP_TLB_INVALIDATE_ALL_INST "c8, c5, 0"
#define COP_TLB_INVALIDATE_ALL_DATA "c8, c6, 0"

#define COP_TLB_INVALIDATE_MVA "c8, c7, 1"
#define COP_TLB_INVALIDATE_MVA_INST "c8, c5, 1"
#define COP_TLB_INVALIDATE_MVA_DATA "c8, c6, 1"

#define COP_TLB_INVALIDATE_ASID "c8, c7, 2"

#define COP_MEMORY_REMAP_PRRR		"c10, c2, 0"
#define COP_MEMORY_REMAP_NMRR		"c10, c2, 1"

/*Only for security extension (not used)*/
#define COP_SYSTEM_RELOCATE_EXCEPTION_VECTOR "c12, c0, 0"


#define COP_CONTEXT_ID_REGISTER			"c13, c0, 1"
#define COP_SOFTWARE_THREAD_ID_USER_R	"c13, c0, 3"

#endif
