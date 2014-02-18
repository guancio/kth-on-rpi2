
#include "hyper.h"
#include "dmmu.h"

#if defined(TARGET_CPU_ARMv5)
#error "Not supproted for this CPU!"
#endif

int mmu_lookup_guest(addr_t vadr, addr_t *padr, int user_write)
{
    addr_t ret;
    if(user_write) {        
        __asm__ volatile("MCR p15, 0, %1, c7, c8, 3\n"
                         "ISB\n"
                         "MRC p15, 0, %0, c7, c4, 0" : "=r"(ret) : "r"(vadr));
    } else {
        __asm__ volatile("MCR p15, 0, %1, c7, c8, 2\n"
                         "ISB\n"
                         "MRC p15, 0, %0, c7, c4, 0" : "=r"(ret) : "r"(vadr));        
    }
    
    *padr = (vadr & 4095) | (ret & (~4095));    
    return (ret ^ 1) & 1;
}

int mmu_lookup_hv(addr_t vadr, addr_t *padr, int hv_write)
{
    addr_t ret;
    if(hv_write) {        
        __asm__ volatile("MCR p15, 0, %1, c7, c8, 1\n"
                         "ISB\n"
                         "MRC p15, 0, %0, c7, c4, 0" : "=r"(ret) : "r"(vadr));
    } else {
        __asm__ volatile("MCR p15, 0, %1, c7, c8, 0\n"
                         "ISB\n"
                         "MRC p15, 0, %0, c7, c4, 0" : "=r"(ret) : "r"(vadr));        
    }
    
    *padr = (vadr & 4095) | (ret & (~4095));    
    return (ret ^ 1) & 1;
}

addr_t mmu_guest_pa_to_va(addr_t padr, hc_config * config) {
  return (padr - config->pa_for_pt_access_start + config->reserved_va_for_pt_access_start);
}

