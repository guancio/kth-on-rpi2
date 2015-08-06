#include "hw.h"
#include "hyper.h"

/*Interrupt operations*/

/*interrupt specifies the irq&fiq mask and op specifies the operation (disable, enable, restore)
 *restore operation will restore the flags to the interrupt mask */
void hypercall_interrupt_set(uint32_t interrupt, uint32_t op)
{

	virtual_machine* _curr_vm = get_curr_vm();
	interrupt &= (ARM_IRQ_MASK | ARM_FIQ_MASK);
	if(op==1) /*Enable*/
		_curr_vm->current_mode_state->ctx.psr &= ~(interrupt);
	else if(op==0) /*Disable*/
		_curr_vm->current_mode_state->ctx.psr |= interrupt;

	else if(op ==2){ /*Restore ,restores the flag according to param0*/
		_curr_vm->current_mode_state->ctx.psr &= ~(ARM_IRQ_MASK | ARM_FIQ_MASK) ;
		_curr_vm->current_mode_state->ctx.psr |= interrupt;
	}
	else
		hyper_panic("Unknown interrupt operation", 1);

}
#if 0
void hypercall_irq_save(uint32_t *param)
{
	uint32_t cpsr;
	virtual_machine* _curr_vm = get_curr_vm();
	/*Read CPSR from guest context*/
	cpsr = _curr_vm->current_mode_state->ctx.psr;

	/*Return value in reg0*/
	_curr_vm->current_mode_state->ctx.reg[0] = cpsr;
	/*Disable IRQ*/
	cpsr |= ARM_IRQ_MASK;
	_curr_vm->current_mode_state->ctx.psr = cpsr;
}
void hypercall_irq_restore(uint32_t flag)
{
	virtual_machine* _curr_vm = get_curr_vm();
	/*Only let guest restore IRQ, FIQ flags not mode*/
	flag &= (ARM_IRQ_MASK | ARM_FIQ_MASK);

	_curr_vm->current_mode_state->ctx.psr &= ~(ARM_IRQ_MASK | ARM_FIQ_MASK) ;
	_curr_vm->current_mode_state->ctx.psr |= flag;
}
#endif

void hypercall_end_interrupt(){
	virtual_machine* _curr_vm = get_curr_vm();
	if (_curr_vm->current_guest_mode != HC_GM_INTERRUPT) {
		hyper_panic("Guest tried to end interrupt but not in interrupt mode.", 1);
	}
	if (_curr_vm->interrupted_mode >= HC_NGUESTMODES) {
		hyper_panic("Invalid interrupted mode value.", 2);
	}
	if (_curr_vm->interrupted_mode == _curr_vm->current_guest_mode) {
		//hyper_panic("Interrupt mode interrupted itself??", 3);
		printf("An interuppt inside the interrupt happened!\n");

	}

	change_guest_mode(_curr_vm->interrupted_mode);
	_curr_vm->interrupted_mode = MODE_NONE;

}
