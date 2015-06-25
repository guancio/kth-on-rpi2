#include "hw.h"
#include "arm_common.h"
#include "cpu_cop.h"

cpu_callback family_callback_inst_abort = 0;
cpu_callback family_callback_data_abort = 0;

cpu_callback family_callback_swi = 0;
cpu_callback family_callback_undef = 0;

void cpu_get_type(cpu_type *type, cpu_model *model)
{
    *type = CT_ARM;
    *model = CM_ARMv5;
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
    // TODO: find a way to break into debuuger from ARM9
}

void cpu_init()
{
    /* Invalidate and enable cache*/
    mem_cache_invalidate(TRUE,TRUE,TRUE); //instr, data, writeback
    mem_cache_set_enable(TRUE);    
    
    
    /*
    env_memsapce_created_physical(PROC_TYPE_HYPERVISOR, "__hyper_excp",
                                  0xFFFF0000, HAL_VIRT_START, PAGE_SIZE);
    */
}


