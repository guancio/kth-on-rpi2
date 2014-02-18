
#include <hw.h>
#include <mmu.h>

memory_layout_entry memory_padr_layout[] = 
{
    {ADDR_TO_PAGE(0x00000000), ADDR_TO_PAGE(0x000fffff), MLT_HYPER_RAM, MLF_READABLE | MLF_WRITEABLE }, // hypervisor ram
    {ADDR_TO_PAGE(0x00100000), ADDR_TO_PAGE(0x001fffff), MLT_USER_RAM, MLF_READABLE | MLF_WRITEABLE }, // user ram
    {ADDR_TO_PAGE(0xfff00000), ADDR_TO_PAGE(0xfffe3fff), MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE}, // special function, USART0, TC
    {ADDR_TO_PAGE(0xffff0000), ADDR_TO_PAGE(0xffffbfff), MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE}, // WD, power save, PIO
    {ADDR_TO_PAGE(0xffffF000), ADDR_TO_PAGE(0xfffffFFF), MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE | MLF_LAST}, // AIC
};
