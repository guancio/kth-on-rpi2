
#include <lib.h>
#include <types.h>

#include "hypercalls.h"

// TEMP STUFF
enum dmmu_command {
    CMD_CREATE_L1
};

extern uint32_t syscall_dmmu(uint32_t r0, uint32_t r1, uint32_t r2);

uint32_t l1[4096] __attribute__ ((aligned (16 * 1024)));

void _main()
{
  int j;
  printf("starting\n");
    /* syscall_dmmu(CMD_CREATE_L1, &l1, 0); */
    
    /* syscall_dmmu(CMD_CREATE_L1, 0xF0000000, 0); */
    /* syscall_dmmu(CMD_CREATE_L1, 0x00000000, 0); */
    /* syscall_dmmu(CMD_CREATE_L1, 0x10000000, 0); */
    // test hypercall from guest
    // ISSUE_HYPERCALL_REG1(HYPERCALL_NEW_PGD, 0x01234567);
  for(;;) {
    for(j = 0; j < 500000; j++) asm("nop");
    printf("running\n");
  }
}

/*Each guest must provide a handler rpc*/
void handler_rpc(unsigned callNum, void *params)
{
    
}
