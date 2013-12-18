#include "hyper.h"

extern virtual_machine *curr_vm;

void start(){
    asm ("mov LR, %0      \n\t" :: "r"(curr_vm->config->guest_entry_point));
#ifdef LINUX
    /*Prepare r0 r1 and r2 for linux boot */
    asm ("mov r1, %0      \n\t" :: "r"(LINUX_ARCH_ID));
    asm ("mov r2, %0      \n\t" :: "r"(curr_vm->config->guest_entry_point - 0x10000 + 0x100));
    asm ("mov r0, #0 \n\t");
#endif
    asm ("MSR SPSR, #0xD0 \n\t");
    asm ("MOVS PC, LR \n\t");

}
