#include "hw.h"
#include "hyper_config.h"
#include "hyper.h"

extern virtual_machine* get_curr_vm();

void hypercall_rpc(uint32_t rpc_op, void *arg){
	const hc_rpc_handler *handler;
	virtual_machine* _curr_vm = get_curr_vm();
	handler = _curr_vm->config->rpc_handlers;

	uint32_t handling_mode = handler->mode;

	if(_curr_vm->current_mode_state->rpc_to  != MODE_NONE )
		hyper_panic("Guest trying to start RPC while being in one", 1);

	if(_curr_vm->current_guest_mode == handling_mode)
		hyper_panic("Guest trying to send RPC to itself", 1);

	if(_curr_vm->mode_states[handling_mode].rpc_for != MODE_NONE)
		hyper_panic("Guest trying to send RPC to a mode that is already handling a RPC", 1);

	if(rpc_op <=4){

		_curr_vm->current_mode_state->rpc_to = handling_mode;
		_curr_vm->mode_states[handling_mode].rpc_for = _curr_vm->current_guest_mode;

		change_guest_mode(HC_GM_TRUSTED);
		_curr_vm->current_mode_state->ctx.reg[0] = rpc_op;
		_curr_vm->current_mode_state->ctx.reg[1] = (uint32_t)arg;
		_curr_vm->current_mode_state->ctx.pc = handler->entry_point;
		_curr_vm->current_mode_state->ctx.psr = 0xD0; /*USR mode, IRQ off*/
	}
	else{
		hyper_panic("Unallowed rpc operation\n",1);

	}

}

void hypercall_end_rpc(){
	virtual_machine* _curr_vm = get_curr_vm();
	uint32_t calling_mode = _curr_vm->current_mode_state->rpc_for;

	if(calling_mode == MODE_NONE )
		hyper_panic("Guest ended rpc without being in one", 1);

	if(_curr_vm->mode_states[calling_mode].rpc_to !=  _curr_vm->current_guest_mode )
		hyper_panic("Guest trying to end rpc but caller did not start one", 1);

	calling_mode = _curr_vm->current_mode_state->rpc_for;

	_curr_vm->current_mode_state->rpc_for = MODE_NONE;
	_curr_vm->mode_states[calling_mode].rpc_to = MODE_NONE;

	change_guest_mode(calling_mode);

}
