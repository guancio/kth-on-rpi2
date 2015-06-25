

#include <hw.h>

#include "soc_defs.h"

static uint32_t clk_rst_base [] = {
    0, /* peripheras starts at 1 */
    0x8012F000, /* PER1 */
    0x8011F000,
    0x8000F000,
    0, /* PER4 is always on, base = 0x80150000 */
    0xA03FF000, /* PER5 */
    0xA03CF000, /* PER6 */
};

void soc_clock_enable(int per, int enable, int start)
{
    BASE_REG r = (BASE_REG) clk_rst_base[per];
    if(!r) return;
    if(enable >= 0) r[PRCC_PCKEN] |= 1UL << enable;
    if(start >= 0) r[PRCC_KCKEN] |= 1UL << start;
}

void soc_ctrl_init()
{
    BASE_REG prcmu = (BASE_REG ) PRCMU_BASE;
    BASE_REG gpio0 = (BASE_REG ) GPIO0_BASE;
    
    /* PRCMU clock stuff: turn on timer and peripherals busses */
    prcmu[PRCMU_TCR] = 1UL << 17;  /* ??? */
    prcmu[PRCMU_PER1_MGT] |= PRCM_CLKEN;
    prcmu[PRCMU_PER2_MGT] |= PRCM_CLKEN;
    prcmu[PRCMU_PER3_MGT] |= PRCM_CLKEN;
    prcmu[PRCMU_PER5_MGT] |= PRCM_CLKEN;
    prcmu[PRCMU_PER6_MGT] |= PRCM_CLKEN;
    prcmu[PRCMU_PER7_MGT] |= PRCM_CLKEN;
    
    /* UART2 clock 1... */
    prcmu[PRCMU_UARTCLK_MGT] = PRCM_CLK38 | PRCM_CLKEN;
    
    /* enable and start the clocks */
    soc_clock_enable(3, 6, 6); /* CLK_P3_UART2 */    
    soc_clock_enable(6, 7, -1); /* MTU 0 */    
    soc_clock_enable(1, 9, -1);  /* GPIO for UART2 */

    gpio0[GPIO_AFSLA] |= 0x60000000; /* GPIO alternate function (UART) */
    gpio0[GPIO_AFSLB] |= 0x60000000; /* GPIO alternate function (UART) */
          
}

