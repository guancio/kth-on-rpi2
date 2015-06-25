#include "hw.h"
#include "hyper.h"
extern virtual_machine *curr_vm;

/*Interrupt operations*/

/*interrupt specifies the irq&fiq mask and op specifies the operation (disable, enable, restore)
 *restore operation will restore the flags to the interrupt mask */
void hypercall_interrupt_set(uint32_t interrupt, uint32_t op)
{
	interrupt &= (ARM_IRQ_MASK | ARM_FIQ_MASK);
	if(op==1) /*Enable*/
		curr_vm->current_mode_state->ctx.psr &= ~(interrupt);
	else if(op==0) /*Disable*/
		curr_vm->current_mode_state->ctx.psr |= interrupt;

	else if(op ==2){ /*Restore ,restores the flag according to param0*/
		curr_vm->current_mode_state->ctx.psr &= ~(ARM_IRQ_MASK | ARM_FIQ_MASK) ;
		curr_vm->current_mode_state->ctx.psr |= interrupt;
	}
	else
		hyper_panic("Unknown interrupt operation", 1);

}
#if 0
void hypercall_irq_save(uint32_t *param)
{
	uint32_t cpsr;

	/*Read CPSR from guest context*/
	cpsr = curr_vm->current_mode_state->ctx.psr;

	/*Return value in reg0*/
	curr_vm->current_mode_state->ctx.reg[0] = cpsr;
	/*Disable IRQ*/
	cpsr |= ARM_IRQ_MASK;
	curr_vm->current_mode_state->ctx.psr = cpsr;
}
void hypercall_irq_restore(uint32_t flag)
{
	/*Only let guest restore IRQ, FIQ flags not mode*/
	flag &= (ARM_IRQ_MASK | ARM_FIQ_MASK);

	curr_vm->current_mode_state->ctx.psr &= ~(ARM_IRQ_MASK | ARM_FIQ_MASK) ;
	curr_vm->current_mode_state->ctx.psr |= flag;
}
#endif

void hypercall_end_interrupt () {
	if (curr_vm->current_guest_mode != HC_GM_INTERRUPT) {
		hyper_panic("Guest tried to end interrupt but not in interrupt mode.", 1);
	}
	if (curr_vm->interrupted_mode >= HC_NGUESTMODES) {
		hyper_panic("Invalid interrupted mode value.", 2);
	}
	if (curr_vm->interrupted_mode == curr_vm->current_guest_mode) {
		//hyper_panic("Interrupt mode interrupted itself??", 3);
		printf("An interuppt inside the interrupt happened!\n");

	}

	change_guest_mode(curr_vm->interrupted_mode);
	curr_vm->interrupted_mode = MODE_NONE;

}
