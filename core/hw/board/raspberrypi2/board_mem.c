#include <types.h>
#include <mmu.h>
#include <soc_defs.h> //This is where we get the peripherals' addresses from.

//The base RAM address.
//This is only used in this file, and decides where hypervisor RAM 
//starts in physical RAM space.
//TODO: Note: On the beagleboard configuration, this starts with an offset of 0x100000 from the physical starting address of RAM.
//Since we load the hypervisor at 0x1000000, maybe this should be
//0x1100000?
#define BASE_RAM_ADDRESS HAL_PHYS_START+0x100000

//The amount of RAM that the hypervisor gets. This should really be centrally
//defined and not board-dependent.
//Current amount:	4 MiB
#define AMOUNT_OF_HYPERVISOR_RAM 0x400000

//The amount of trusted RAM.
//Current amount:	1 MiB
#define AMOUNT_OF_TRUSTED_RAM 0x100000

//The amount of user RAM.
//Possibly, with only one user, user could always use "the rest" of available
//RAM since we know the total amount of RAM on the board.
//TODO: Instead calculate from the previous offset and total RAM, giving guest(s)
//the remaining RAM?
//Current amount:	5 MiB
#define AMOUNT_OF_USER_RAM 0x500000

//Here, we reserve RAM space for stuff.
//Physical addresses of RPi2 RAM range from 0x00000000 to 0x3e000000, counting
//with the 64 MiB by default reserved for the GPU.

//These are the addresses of peripherals, and the address spaces of the
//different RAM areas.
memory_layout_entry memory_padr_layout[] = {
	//Explanation of the various flags:
	//MLT_IO_HYP_REG
	//	This is a memory range only the hypervisor should access. Example:
	//	interrupt controller.
	//MLT_IO_RW_REG:
	//	This is a memory range a guest should be able to both read from and write
	//	to. Example: UART. No problems will arise from this.
	//MLT_IO_RO_REG:
	//	This is a memory range a guest should only be able to read from.
	//	Examples could be various clock registers.

	//Note: There is a convention to only assign 4 KiB-sized ranges (multiples
	//of 0x1000). If you smaller ranges, the program will break.
	//TODO: Make default behaviour in memory_init to either warn if range is too
	//small, or assign a 4 KiB-sized range with the same starting address.
	//Note: These are all physical addresses, which are then converted to pages.
	
	//Clocks (0x3F003000 to 0x3F003021)
	{ADDR_TO_PAGE(CLOCK_BASE), ADDR_TO_PAGE(CLOCK_BASE + 0x1000),
	MLT_IO_RO_REG,  MLF_READABLE | MLF_WRITEABLE},

	//Interrupt controller (0x3F00B200 to 0x3F00B227)
	{ADDR_TO_PAGE(0x3F00B000), ADDR_TO_PAGE(0x3F00C000), //TODO: Collides with ARM timer
	MLT_IO_HYP_REG, MLF_READABLE | MLF_WRITEABLE},

	//ARM timer (0x3F00B400 to 0x3F00B423)
	/*
	{ADDR_TO_PAGE(0x3F00B000), ADDR_TO_PAGE(0x3F00C000), //TODO: Collides with IC
	MLT_IO_RW_REG,  MLF_READABLE | MLF_WRITEABLE},
	*/
	//GPIO (general) (0x3F200000 to 0x3F2000B3)
	{ADDR_TO_PAGE(GPIO_BASE), ADDR_TO_PAGE(GPIO_BASE + 0x1000), //TODO: Ends at UART
	MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE},

	//UART (0x3F201000 to 0x3F20108F)
	{ADDR_TO_PAGE(UART_BASE), ADDR_TO_PAGE(UART_BASE + 0x1000),
	MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE},

	//Quad-Core processor control (0x40000000 to 0x400000FF)
	{ADDR_TO_PAGE(0x40000000), ADDR_TO_PAGE(0x40000000 + 0x1000),
	MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE},
	
	////////////////////////////////////////////////////////////////////////////
	//These are the RAM address spaces.

	//Hypervisor RAM
	{ADDR_TO_PAGE(BASE_RAM_ADDRESS),
	ADDR_TO_PAGE(BASE_RAM_ADDRESS + AMOUNT_OF_HYPERVISOR_RAM),
	MLT_HYPER_RAM,
	MLF_READABLE | MLF_WRITEABLE},

	//Trusted RAM
	{ADDR_TO_PAGE(BASE_RAM_ADDRESS + AMOUNT_OF_HYPERVISOR_RAM),
	ADDR_TO_PAGE(BASE_RAM_ADDRESS + AMOUNT_OF_HYPERVISOR_RAM + AMOUNT_OF_TRUSTED_RAM),
	MLT_TRUSTED_RAM,
	MLF_READABLE | MLF_WRITEABLE},

	//User RAM
	{ADDR_TO_PAGE(BASE_RAM_ADDRESS + AMOUNT_OF_HYPERVISOR_RAM + AMOUNT_OF_TRUSTED_RAM),
	ADDR_TO_PAGE(BASE_RAM_ADDRESS + AMOUNT_OF_HYPERVISOR_RAM + AMOUNT_OF_TRUSTED_RAM + AMOUNT_OF_USER_RAM),
	MLT_USER_RAM,
	MLF_READABLE | MLF_WRITEABLE | MLF_LAST},
};

