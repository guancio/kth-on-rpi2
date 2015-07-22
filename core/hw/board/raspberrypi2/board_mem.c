#include <types.h>
#include <mmu.h>

//The base RAM address - definitely board-dependent.
//This is the physical address at which the bootloader boots the kernel, I think.
//TODO: IF you set this to 0x0, it will overwrite parts of the bootloader???
#define BASE_RAM_ADDRESS 0x1000000

//The amount of RAM that the hypervisor gets. This should really be centrally
//defined and not board-dependent.
//Current amount:	4 MiB
#define AMOUNT_OF_HYPERVISOR_RAM 0x400000

//The amount of trusted RAM.
//Current amount:	1 MiB
#define AMOUNT_OF_TRUSTED_RAM 0x100000

//The amount of user RAM. Supposing several users, this should not be defined at
//compile time. It is hard for me to see where you ideally would put this.
//Possibly, with only one user, user could always use "the rest" of available
//RAM since we know the total amount of RAM on the board.
//TODO: instead calculate from the previous offset and total RAM.
//Current amount:	5 MiB
#define AMOUNT_OF_USER_RAM 0x500000

//Here, we reserve RAM space for stuff.
//Physical addresses of RPi2 RAM range from 0x00000000 to 0x3e000000, counting
//with the 64 MiB reserved for the GPU.

//These are the addresses of peripherals, and the address spaces of the
//different RAM areas.
memory_layout_entry memory_padr_layout[] =
{
	////////////////////////////////////////////////////////////////////////
	//Readable and writeable address ranges
	//UART
	//	The UART addresses range between 0x7E20100 to 0x7E2018F (Rpi1).
	//	For Raspberry Pi 2, we just change "7E" to "3F".
	{ADDR_TO_PAGE(0x3F20100), ADDR_TO_PAGE(0x3F2018F),
	MLT_IO_RW_REG, MLF_READABLE | MLF_WRITEABLE},

	//Address ranges under hypervisor control
	//Interrupt controller
	//	Bus addresses of the ARM interrupt register are 0x7E00B000 to
	//	0x7E00B227 (RPi1), I think (offset to first is 0x200)
	//	For Raspberry Pi 2, we just change "7E" to "3F".
	{ADDR_TO_PAGE(0x3F00B200), ADDR_TO_PAGE(0x3F00B227),
	MLT_IO_HYP_REG, MLF_READABLE | MLF_WRITEABLE},

	//GPIO
	//	Bus addresses of the GPIO registers are 0x7E200000 to
	//	0x7E2000B3 (RPi1).
	//	For Raspberry Pi 2, we just change "7E" to "3F".
	{ADDR_TO_PAGE(0x3F200000), ADDR_TO_PAGE(0x3F2000B3),
	MLT_IO_HYP_REG, MLF_READABLE | MLF_WRITEABLE},

	//TODO: Read-only address ranges - changed temp. to RW ranges.
	//L4-Wakeup (as in Beagleboard file)???
	//System control module (as in Beagleboard file)???
	//Clocks
	//	Addresses of system timers are between 0x7E003000 and 0x7E003021
	//	For Raspberry Pi 2, we just change "7E" to "3F".
	{ADDR_TO_PAGE(0x3F003000), ADDR_TO_PAGE(0x3F003021),
	MLT_IO_RW_REG,  MLF_READABLE | MLF_WRITEABLE},
	
	//	Addresses of the ARM timer are between 0x7E00B000 and 0x7E00B423
	//	(offset to first is 0x400)
	//	For Raspberry Pi 2, we just change "7E" to "3F".
	{ADDR_TO_PAGE(0x3F00B400), ADDR_TO_PAGE(0x3F00B423),
	MLT_IO_RW_REG,  MLF_READABLE | MLF_WRITEABLE},
	////////////////////////////////////////////////////////////////////////

	//These are the RAM address spaces...
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
	MLF_READABLE | MLF_WRITEABLE | MLF_LAST},//TODO: Is it really entirely cricket with a comma after the last entry?
};

