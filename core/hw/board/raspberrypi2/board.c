#include <hw.h>
#include "board.h"

//Probably a dummy function which must be here, in case the hypervisor needs to
//do something upon initialization of the board.

extern uint32_t *flpt_va;

void board_init(){
	//Beagleboard, ovp_arm9, u8500_ref and Beaglebone has nothing in here.
	//IntegratorCP has preparation of ATAG... Whatever that is.
}
