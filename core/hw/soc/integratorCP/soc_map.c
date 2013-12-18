#include "hw.h"

void soc_map(uint32_t l1_address)
{
	//Map Integrator
	map_section(l1_address, 0xf1000000, 0x10000000, FALSE);
	// map Timer
	map_section(l1_address, 0xf1300000, 0x13000000, FALSE);
	//map Pic
	map_section(l1_address, 0xf1400000, 0x14000000, FALSE);
	//Map UART
	map_section(l1_address, 0xf1600000, 0x16000000, FALSE);

}
