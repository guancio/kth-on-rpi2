
#include <hw.h>

#include "board.h"

extern uint32_t *flpt_va;

void board_init()
{
	/*Temporary IO Mappings */
	/*
  pt_create_section(flpt_va, 0xf8000000, 0x68000000, 0, 3, MLT_IO_REG); //Test virtual mappings
  pt_create_section(flpt_va, 0xfc000000, 0x6c000000, 0, 3, MLT_IO_REG); //Test virtual mappings
  pt_create_section(flpt_va, 0xfd000000, 0x6d000000, 0, 3, MLT_IO_REG); //Test virtual mappings
  pt_create_section(flpt_va, 0xfe000000, 0x6e000000, 0, 3, MLT_IO_REG); //Test virtual mappings
*/

}
