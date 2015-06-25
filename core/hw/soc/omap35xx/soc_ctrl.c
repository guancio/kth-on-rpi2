

#include <hw.h>

#include "soc_defs.h"

void soc_ctrl_init()
{
    
    /* peripherals clocks: enable everything! */
    hwreg_write(IO_VA_ADDRESS(CLOCK_PER) + CLOCK_PER_FCLKEN * 4, 0x0003FFFF);
    hwreg_write(IO_VA_ADDRESS(CLOCK_PER) + CLOCK_PER_ICLKEN * 4, 0x0003FFFF);
    
    /* OCP interface clock active */
    hwreg_write(IO_VA_ADDRESS(CLOCK_PER) + CLOCK_PER_CLKSTST *4, 0x00000001);
    // TODO: setup PLL etc.
    
}

