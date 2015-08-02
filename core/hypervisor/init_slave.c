#include <hw.h>
#include "hyper.h"
#include "guest_blob.h"
#include "mmu.h"
#include "hw_core_mem.h"
#include "dmmu.h"

//Add what you need here in terms of external functions and imports to be able
//to do what you want.

void slave_start_(){
	/* Flush and enable the cache, among other things. Defined in
	 * core/hw/cpu/family/model/cpu_init.c. */
    cpu_init(); //TODO: I think this needs to be done a second time.

    /* Set up pagetable rules - defined in init.c. */
    //memory_init(); //TODO: This sets up the heap and the memory layout...

    /* Initialize peripherals - defined in core/hw/soc/platform and
	 * core/hw/board/platform, respectively. */
    //soc_init(); //TODO: I'm 99% this does not need to be done a second time.
    //board_init(); //TODO: This does nothing, but what could be found in here
					//		should probably only be called once.

    /* Set up exception handlers and starting timer - defined in init.c. */
    //setup_handlers(); //TODO: This does not do anything core-specific, as far
						//		as I'm aware...

    /* DMMU initialization - defined in dmmu.c. */
    //dmmu_init(); //TODO: This needs to be done once per core if every core
				   //	   requires a separate BFT - but that will require some
				   //	   modification...

    /* Initialize hypervisor guest modes and data structures according to config
	 * file in guest - defined in init.c. */
    //guests_init(); //TODO: This appears hard-coded to load only one guest and/
					 //		 or the trusted guest, and takes no arguments.
					 //      Currently, everything related to choosing guests
					 //      is done by pre-processor macros.
    
	//start_guest(); //TODO: Simply swaps to kernel mode and starts guest.
}
