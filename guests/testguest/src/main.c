#include <lib.h>
#include <types.h>

#include "dtest.h"

void _main()
{
	uint32_t p0, p1, p2 , p3;
    __asm__ volatile (
         "mov %0, r3\n"
         "mov %1, r4\n"
         "mov %2, r5\n"
         "mov %3, r6\n"
         : "=r"(p0), "=r"(p1), "=r"(p2), "=r"(p3) : );

	pstart = p0;
	vstart = p1;
	psize = 0xFA11;
	fwsize= 0xFA11;

	va_base = vstart;
	printf("You are now inside the 'dtest' guest, which can be configured at compile time to run several tests of the hypervisor.\n");
	printf("Received parameters: pstart=%x vstart=%x psize=%x fwsize=%x\n", pstart, vstart, psize, fwsize);

	int j;
	for(;;){
		asm("nop");
                printf("you what bro?\n");
	}
}

/*Each guest must provide a handler rpc*/
void handler_rpc(unsigned callNum, void *params)
{

}
