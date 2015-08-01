#include <hw.h>
#include "hyper.h"
#include "guest_blob.h"
#include "mmu.h"
#include "hw_core_mem.h"
#include "dmmu.h"
//TODO: Note: Added these to avoid warnings.
extern void dmmu_init();
extern uint32_t dmmu_map_L1_section(addr_t va, addr_t sec_base_add, uint32_t attrs);
extern void debug_inf(void); //TODO. Remove after debugging

//Add what you need here in terms of external functions and imports to be able
//to do what you want.

void slave_start_(){
	//TODO: Which of these needs to be done?
    //cpu_init();

    /* Set up pagetable rules. */
    //memory_init();

    /* Initialize hardware. */
    //soc_init();
    //board_init();

    /* Set up exception handlers and starting timer. */
    //setup_handlers();

    /* DMMU initialization. */
    //dmmu_init();

    /* Initialize hypervisor guest modes and data structures
     * according to config file in guest*/
    //guests_init();

	//Move along, main core...
    //start_guest();
}
