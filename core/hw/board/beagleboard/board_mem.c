#include <types.h>
#include <mmu.h>
#include <soc_defs.h> //This is where we get the peripherals' addresses from.

//The base RAM address.
//This is only used in this file, and decides where hypervisor RAM 
//starts in physical RAM space.
#define BASE_RAM_ADDRESS HAL_PHYS_START+0x100000

//The amount of RAM that the hypervisor gets.
//Current amount:	4 MiB
#define AMOUNT_OF_HYPERVISOR_RAM 0x400000

//The amount of trusted RAM.
//Current amount:	1 MiB
#define AMOUNT_OF_TRUSTED_RAM 0x100000

//The amount of user RAM.
//TODO: Instead calculate from the previous offset and total RAM, giving guest(s)
//the remaining RAM?
//Current amount:	5 MiB
#define AMOUNT_OF_USER_RAM 0x500000

//These are the addresses of peripherals, and the address spaces of the
//different RAM areas.
memory_layout_entry memory_padr_layout[] =
{

	//Note: There is a convention to only assign 4 KiB-sized ranges (multiples
	//of 0x1000). If you smaller ranges, the program will break.
	//Note: These are all physical addresses, which are then converted to pages.

	/* SYSTEM CONTROL MODULE 4 KiB - preferably RO */
	{ADDR_TO_PAGE(SMC_CONTROL), ADDR_TO_PAGE(SMC_CONTROL + 0x1000),
	MLT_IO_RO_REG, MLF_READABLE},

	/* CLOCKS 16 KiB (only 8 KiB needed in Linux port)*/
	{ADDR_TO_PAGE(0x48004000), ADDR_TO_PAGE(0x48006000),
	MLT_IO_RO_REG, MLF_READABLE},

	/* UART1 4 KiB */
	{ADDR_TO_PAGE(UART1_BASE), ADDR_TO_PAGE(UART1_BASE + 0x1000),
	MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE},

	/* UART2 4 KiB */
	{ADDR_TO_PAGE(UART2_BASE), ADDR_TO_PAGE(UART2_BASE + 0x1000),
	MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE},

	/* INTERRUPT CONTROLLER BASE 16 KiB (only 4 KiB needed in Linux port) */
    {ADDR_TO_PAGE(INTC_BASE), ADDR_TO_PAGE(INTC_BASE + 0x1000),
	MLT_IO_HYP_REG, MLF_READABLE | MLF_WRITEABLE},

	/* L4-Wakeup (gp-timer in reserved) 4 KiB */
    {ADDR_TO_PAGE(0x48304000), ADDR_TO_PAGE(0x48305000),
	MLT_IO_RO_REG, MLF_READABLE},

	/* L4-Wakeup (power-reset manager) module A 8 KiB - can be RO
	OMAP READS THE HW REGISTER TO SET UP CLOCKS */
    {ADDR_TO_PAGE(0x48306000), ADDR_TO_PAGE(0x48308000),
	MLT_IO_RO_REG, MLF_READABLE},

	/* L4-Wakeup (32 KiB TIMER module) 4 KiB RO */
    {ADDR_TO_PAGE(0x48320000), ADDR_TO_PAGE(0x48321000),
	MLT_IO_RO_REG, MLF_READABLE},

	/* CONTROL MODULE ID CODE 4 KiB RO */
    {ADDR_TO_PAGE(0x4830A000), ADDR_TO_PAGE(0x4830B000),
	MLT_IO_RO_REG, MLF_READABLE},

	/* UART 3 */
    {ADDR_TO_PAGE(UART3_BASE), ADDR_TO_PAGE(UART3_BASE + 0x1000),
	MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE},

	////////////////////////////////////////////////////////////////////////////
	//These are the RAM address spaces.

	//Hypervisor RAM
    {ADDR_TO_PAGE(BASE_RAM_ADDRESS),
	ADDR_TO_PAGE(BASE_RAM_ADDRESS + AMOUNT_OF_HYPERVISOR_RAM),
	MLT_HYPER_RAM , MLF_READABLE | MLF_WRITEABLE },

	//Trusted RAM
    {ADDR_TO_PAGE(BASE_RAM_ADDRESS + AMOUNT_OF_HYPERVISOR_RAM),
	ADDR_TO_PAGE(BASE_RAM_ADDRESS + AMOUNT_OF_HYPERVISOR_RAM + AMOUNT_OF_TRUSTED_RAM),
	MLT_TRUSTED_RAM , MLF_READABLE | MLF_WRITEABLE },

	//User RAM
	//TODO: Trusted RAM ends at 0x80600000. Why does this start at 0x81000000?
    {ADDR_TO_PAGE(0x81000000),
	ADDR_TO_PAGE(0x81000000 + AMOUNT_OF_USER_RAM),
	MLT_USER_RAM , MLF_READABLE | MLF_WRITEABLE | MLF_LAST},
};

