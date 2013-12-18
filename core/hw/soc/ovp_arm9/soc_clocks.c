#include <hw.h>
#include <mmu.h>

void soc_clocks_init()
{        
    powersaving_registers * power;
    memspace_t *ms_power;
    
#if 1
    ms_power = env_map_from(PROC_TYPE_HYPERVISOR, PROC_TYPE_HYPERVISOR,
                          "__soc_clock", POWERSAVING_BASE, PAGE_SIZE, TRUE);
     
#else   
    ms_power = env_memspace_create_physical(PROC_TYPE_HYPERVISOR, "__soc_clock",
                                             POWERSAVING_BASE, 0xFFF00000, PAGE_SIZE, 
                                            TRUE);
#endif

    if(!ms_power) panic("soc_clocks_init");
    
    power = (powersaving_registers *) ms_power->vadr;
    
    /* AND POWER FOR ALL: clock enable for all peripherals */
    power->pcer = -1;
    
    /* no longer need that one */

#if 1
    env_unmap(PROC_TYPE_HYPERVISOR, PROC_TYPE_HYPERVISOR, ms_power, TRUE);    

#else
    env_memspace_free(PROC_TYPE_HYPERVISOR, ms_power, TRUE);    
#endif
}
