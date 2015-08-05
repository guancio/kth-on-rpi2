#include "hw.h"
#include "arm_common.h"
#include "cpu_cop.h"
#include "cache.h"

extern uint32_t __hyper_pt_start__[]; // Symbols address is the value (linker script)

/* Forward reference */
static return_value default_catcher(uint32_t r0, uint32_t r1, uint32_t r2);

/* Callback functions */
cpu_callback family_callback_inst_abort = default_catcher;
cpu_callback family_callback_data_abort = default_catcher;

cpu_callback family_callback_swi = default_catcher;
cpu_callback family_callback_undef = default_catcher;


void cpu_get_type(cpu_type *type, cpu_model *model)
{
    *type = CT_ARM;
    *model = CM_ARMv7a;
}

void cpu_set_abort_handler(cpu_callback inst, cpu_callback data)
{
    family_callback_inst_abort = inst;
    family_callback_data_abort = data;
}

void cpu_set_swi_handler(cpu_callback handler)
{
    family_callback_swi = handler;
}
void cpu_set_undef_handler(cpu_callback handler)
{
    family_callback_undef = handler;
}

void cpu_break_to_debugger()
{
    /* TODO */
}


/* 
 * Default exception catcher so we dont crash into NULL due to very early
 * exceptions.
 */
static return_value default_catcher(uint32_t r0, uint32_t r1, uint32_t r2)
{
    uint32_t i;
    context *t = cpu_context_current_get();
    
    printf("\nDC %x_%x_%x\n", r0, r1, r2);
    if(t) {
		//Print the processor core registers.

		//13 general-purpose registers...
        for(i = 0; i < 13; i++){
            printf("R%d=%x\t", i, t->reg[i]);
		}

		//... the stack pointer, ...
        printf("SP=%x\n\n", t->sp);
		//... the link register, ...
        printf("LR=%x\n\n", t->lr);
		//... the program counter, ...
        printf("PC=%x\n\n", t->pc);

		//... and the program status register.
        printf("PSR=%x\n\n", t->psr);
    }
    
    /* no point continuing ? */
    for(;;) 
        ;
    return RV_OK;
}

void cpu_init()
{
    /* Invalidate and enable cache */
    //mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    //mem_cache_set_enable(TRUE);
	CacheDataCleanInvalidateAll();
	CacheInstInvalidateAll();
    CacheEnable(CACHE_ALL);
    //Setup page table pointer 1
    /* PTWs cacheable, inner WB not shareable, outer WB not shareable. It is in
	 * fact here we switch to the master page table.  */
    uint32_t pt		   = (uint32_t)GET_PHYS(__hyper_pt_start__);
    uint32_t ttb_flags = (pt |  TTB_IRGN_WB | TTB_RGN_OC_WB);
    COP_WRITE(COP_SYSTEM,COP_SYSTEM_TRANSLATION_TABLE1,ttb_flags);
    /* The following is Linux specific configuration on armV7,
     * These configuration are used to identify what kind of memory
     * the address space is by looking at the page attributes */

	/*
	 * Memory region attributes with SCTLR.TRE=1
	 *
	 *   n = TEX[0],C,B
	 *   TR = PRRR[2n+1:2n]		- memory type
	 *   IR = NMRR[2n+1:2n]		- inner cacheable property
	 *   OR = NMRR[2n+17:2n+16]	- outer cacheable property
	 *
	 *			n	TR	IR	OR
	 *   UNCACHED		000	00
	 *   BUFFERABLE		001	10	00	00
	 *   WRITETHROUGH	010	10	10	10
	 *   WRITEBACK		011	10	11	11
	 *   reserved		110
	 *   WRITEALLOC		111	10	01	01
	 *   DEV_SHARED		100	01
	 *   DEV_NONSHARED	100	01
	 *   DEV_WC		001	10
	 *   DEV_CACHED		011	10
	 *
	 * Other attributes:
	 *
	 *   DS0 = PRRR[16] = 0		- device shareable property
	 *   DS1 = PRRR[17] = 1		- device shareable property
	 *   NS0 = PRRR[18] = 0		- normal shareable property
	 *   NS1 = PRRR[19] = 1		- normal shareable property
	 *   NOS = PRRR[24+n] = 1	- not outer shareable */

    uint32_t prrr = 0xFF0a81A8;	//Primary region remap regiser
    uint32_t nmrr = 0x40E040e0; //Normal memory remap register
    COP_WRITE(COP_SYSTEM,COP_MEMORY_REMAP_PRRR,prrr);
    COP_WRITE(COP_SYSTEM,COP_MEMORY_REMAP_NMRR,nmrr);

	/*   AT
	 *  TFR   EV X F   I D LR    S
	 * .EEE ..EE PUI. .T.T 4RVI ZWRS BLDP WCAM
	 * rxxx rrxx xxx0 0101 xxxx xxxx x111 xxxx < forced
	 *    1    0 110       0011 1100 .111 1101 < we want
	 */
    uint32_t clear 	= 0x0120c302;
    uint32_t set	= 0x10c03c7d;
    uint32_t mmu_config;
    COP_READ(COP_SYSTEM, COP_SYSTEM_CONTROL, mmu_config);
    mmu_config &= (~clear);
    mmu_config |= set;
    /* Setting alignment fault with Beagleboard crashes it */
    //mmu_config |= CR_A; // Set Alignment fault checking
    COP_WRITE(COP_SYSTEM, COP_SYSTEM_CONTROL, mmu_config);
    //mem_cache_set_enable(TRUE);
    CacheEnable(CACHE_ALL);
}


