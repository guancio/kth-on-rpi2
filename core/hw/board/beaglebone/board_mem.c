#include <types.h>
#include <mmu.h>

memory_layout_entry memory_padr_layout[] = 
{
    // these two are not really used at the moment
    {ADDR_TO_PAGE(0x40000000), ADDR_TO_PAGE(0x4002BFFF), MLT_HYPER_RAM, MLF_READABLE }, // internal ROM
    {ADDR_TO_PAGE(0x402F0000), ADDR_TO_PAGE(0x402FFFFF), MLT_HYPER_RAM, MLF_READABLE | MLF_WRITEABLE }, // internal SRAM


    {ADDR_TO_PAGE(0x44000000), ADDR_TO_PAGE(0x5FFFFFFF), MLT_IO_HYP_REG, MLF_READABLE | MLF_WRITEABLE }, // IO registers

    {ADDR_TO_PAGE(0x80000000), ADDR_TO_PAGE(0x800FFFFF), MLT_HYPER_RAM, MLF_READABLE | MLF_WRITEABLE }, // hypervisor ram
    {ADDR_TO_PAGE(0x80100000), ADDR_TO_PAGE(0x87EFFFFF), MLT_USER_RAM, MLF_READABLE | MLF_WRITEABLE}, // user ram

    // use the last 1meg at the end of memory for hypervisor stack
    {ADDR_TO_PAGE(0x87F00000), ADDR_TO_PAGE(0x87FFFFFF), MLT_HYPER_RAM, MLF_READABLE | MLF_WRITEABLE | MLF_LAST }, // hypervisor stack
};
