
#include <hw.h>

#include "board.h"

void board_init()
{
#ifdef LINUX
    uint32_t *p;
    p = (uint32_t *)(HAL_PHYS_START + 0x1000100);

	/*OVP Simulation
	 *We need to prepare ATAG manually! (Without OVP smartloader)*/
    printf("Integrator CP ATAG setup\n");
    /* ATAG_CORE */
    *p++= 5;
    *p++= 0x54410001;
    *p++= 1;
    *p++= 0x1000;
    *p++= 0;                                                    //0x10

    /* ATAG_MEM */
    /* TODO: handle multiple chips on one ATAG list */

    *p++= 4;
    *p++= 0x54410002;
    *p++= (128 * 1024 * 1024);//info->ram_size;
    *p++= HAL_PHYS_START + 0x1000000;//info->loader_start;
    *p++= 0xb;

    /*ATAG COMMAND LINE*/
    *p++=0x54410009;
    *p++=0x6c726165;
    *p++=0x69727079;
    *p++=0x3d6b746e;
    *p++=0x69726573;
    *p++=0x202c6c61;

    *p++=0x736e6f63;
    *p++=0x3d656c6f;

    *p++=0x41797474;
    *p++=0x30414d;
#endif
}
