
#ifndef _CPU_COP_H_
#define _CPU_COP_H_


/*CP15*/
#define COP_ID_CPU "c0, c0, 0"
#define COP_ID_MEMORY_MODEL_FEAT "c0,c1,4" /*DUMMY 926 does not have this*/


#define COP_SYSTEM_CONTROL "c1, c0, 0"
#define COP_SYSTEM_CONTROL_MMU_ENABLE (1 << 0)
#define COP_SYSTEM_CONTROL_CACHE_ENABLE (1 << 2)

#define COP_SYSTEM_TRANSLATION_TABLE0 "c2, c0, 0"

#define COP_SYSTEM_DOMAIN "c3, c0, 0"
#define COP_SYSTEM_TRANSLATION_TABLE "c2, c0, 0"

#define COP_BRANCH_PRED_INVAL_ALL "c7, c5, 6" /*DUMMY 926 does not have this*/

#define COP_TLB_INVALIDATE_ALL "c8, c7, 0"
#define COP_TLB_INVALIDATE_ALL_INST "c8, c5, 0"
#define COP_TLB_INVALIDATE_ALL_DATA "c8, c6, 0"

#define COP_TLB_INVALIDATE_ONE "c8, c7, 1"
#define COP_TLB_INVALIDATE_ONE_INST "c8, c5, 1"
#define COP_TLB_INVALIDATE_ONE_DATA "c8, c6, 1"


#define COP_ICACHE_INVALIDATE_ALL "c7, c5, 0"
#define COP_DCACHE_INVALIDATE_ALL "c7, c6, 0"
#define COP_DCACHE_ICACHE_INVALIDATE_ALL "c7, c7, 0"
#define COP_DCACHE_INVALIDATE_MVA "c7, c10, 1"
#define COP_DCACHE_CLEAN_INVALIDATE_LINE "c7, c14, 2"

#define COP_TLB_INVALIDATE_MVA "c8, c7, 1"

#define COP_CACHE_TYPE "c0, c0, 1"

#define COP_CONTEXT_ID_REGISTER			"c13, c0, 1"
#endif
