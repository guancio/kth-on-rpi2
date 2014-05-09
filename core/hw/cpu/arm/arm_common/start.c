#include "hyper.h"
#include "guest_blob.h"

extern virtual_machine *curr_vm;

void start(){
    uint32_t r3 = curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[3];
    uint32_t r4 = curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[4];    
    uint32_t r5 = curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[5];    
    uint32_t r6 = curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[6];    
    addr_t start = curr_vm->config->firmware->vstart + curr_vm->config->guest_entry_offset;
#ifdef LINUX
    start = curr_vm->config->firmware->pstart + curr_vm->config->guest_entry_offset;
#endif
    
    printf("Branching to address: %x\n", start);
        
#if !defined(LINUX)
    __asm__ volatile (
		"mov LR, %0\n"
		"mov r3, %1\n"
		"mov r4, %2\n"
		"mov r5, %3\n"
		"mov r6, %4\n"
		"MSR SPSR, #0xD0\n"
    	"MOVS PC, LR\n"
		:: "r"(start), "r"(r3), "r"(r4), "r"(r5), "r"(r6));
#else
    /*Prepare r0 r1 and r2 for linux boot */
    __asm__ volatile (
    	"mov lr, %0\n"
    	"mov r1, %1\n"
    	"mov r2, %2\n"
    	"mov r0, %3\n"
		"MSR SPSR, #0xD0\n"
    	"MOVS PC, LR\n"
    	:: "r"(start), "r"(LINUX_ARCH_ID), "r"(start - 0x10000 + 0x100), "r"(0));
#endif


}
