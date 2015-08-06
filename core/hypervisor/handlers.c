#include <hw.h>
#include "hypercalls.h"
//TODO: Note: Added the below to avoid warnings.
extern void hypercall_dyn_switch_mm(addr_t table_base, uint32_t context_id);
extern void hypercall_dyn_free_pgd(addr_t *pgd_va);
extern void hypercall_dyn_new_pgd(addr_t *pgd_va);
extern void hypercall_dyn_set_pmd(addr_t *pmd, uint32_t desc);
extern void hypercall_dyn_set_pte(addr_t *l2pt_linux_entry_va, uint32_t linux_pte, uint32_t phys_pte);
extern int dmmu_handler(uint32_t p03, uint32_t p1, uint32_t p2);

//TODO: Fix this...
extern virtual_machine *curr_vm;
extern int __hyper_stack_bottom__;

extern virtual_machine vms[4];

//This function returns a pointer to the VM currently running on this core,
//getting the ID of the processor from the stack pointer (good because it is
//platform-independent). 
virtual_machine* get_curr_vm(){
	//TODO: See how this turns out in assembly.
	//TODO: This will be used a lot, so it should be maximally efficient.
	//1. Get value of stack pointer.
	register uint32_t stack_pointer __asm("sp"); //The actual register?
	uint32_t temp_stack_pointer = stack_pointer; //TODO: Do this on one line?

	//2. Subtract __hyper_stack_bottom__ from the stack pointer (all stacks are
	//adjacent, so we get the placement of the address relative to the start of
	//all stacks).
	temp_stack_pointer = temp_stack_pointer - __hyper_stack_bottom__;
	
	//3. XOR with masks of the different stacks until you get 0.
	//The size of one stack is 8*1024 bytes -> 10 0000 0000 0000 in binary.
	//So, we want to clear the first 13 bits.
	//TODO: Note that 13 is hard-coded from the stack size. In the slightly
	//more general case of the stack size being a power of two, 
	temp_stack_pointer = temp_stack_pointer >> 13;
	//TODO: Where do we decide how many VMs we have? Until then:

	//4. Return curr_vm, which is a pointer to the current machine on this
	//particular core.
	return &vms[temp_stack_pointer]; //TODO: Entirely cricket?
}

#define USE_DMMU

void swi_handler(uint32_t param0, uint32_t param1, uint32_t param2, uint32_t hypercall_number){
	virtual_machine* _curr_vm = get_curr_vm();
	/*TODO Added check that controls if it comes from user space, makes it pretty inefficient, remake later*/
	/*Testing RPC from user space, remove later*/
	if(_curr_vm->current_guest_mode == HC_GM_TASK){
		if(hypercall_number == 1020){
			//ALLOWED RPC OPERATION
			hypercall_rpc(param0, (uint32_t *)param1);
			return;
		}
		else if(hypercall_number == 1021){
			hypercall_end_rpc();
			return;
		}
	}
    if(_curr_vm->current_guest_mode == HC_GM_TASK){
        
	//	debug("\tUser process made system call:\t\t\t %x\n",  _curr_vm->mode_states[HC_GM_TASK].ctx.reg[7] );
		change_guest_mode(HC_GM_KERNEL);
		/* TODO: The current way of saving context by the hypervisor is very inefficient,
		 * can be improved alot with some hacking and shortcuts (for the FUTURE)*/
		_curr_vm->current_mode_state->ctx.sp -= (72 + 8) ; //FRAME_SIZE (18 registers to be saved) + 2 swi args
		uint32_t *context, *sp, i;

		context = &_curr_vm->mode_states[HC_GM_TASK].ctx.reg[0];
		sp 		=(uint32_t *) _curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;

		*sp++ = _curr_vm->mode_states[HC_GM_TASK].ctx.reg[4];
		*sp++ = _curr_vm->mode_states[HC_GM_TASK].ctx.reg[5];

		/* Saves 16 ARM registers (all ARM registers in context except program
		 * status register). */
		i = 17; //r0-lr
		while ( i > 0){
			*sp++ = *context++;
			i--;
		}

		*sp = _curr_vm->mode_states[HC_GM_TASK].ctx.reg[0]; //OLD_R0

		//update CR for alignment fault
		//Enable IRQ
		_curr_vm->current_mode_state->ctx.psr &= ~(IRQ_MASK);


		_curr_vm->current_mode_state->ctx.lr = _curr_vm->exception_vector[V_RET_FAST_SYSCALL];//_curr_vm->handlers.syscall.ret_fast_syscall;
		//copy task context to kernel context. syscall supports 6 arguments

		/*system call nr in r7*/
		_curr_vm->current_mode_state->ctx.reg[7] = _curr_vm->mode_states[HC_GM_TASK].ctx.reg[7];


		if(_curr_vm->mode_states[HC_GM_TASK].ctx.reg[7] < _curr_vm->guest_info.nr_syscalls ){
			/*Regular system call, restore params*/
			for(i =0;i <= 5; i++)
				_curr_vm->current_mode_state->ctx.reg[i] = _curr_vm->mode_states[HC_GM_TASK].ctx.reg[i];
			/*Set PC to systemcall function*/
			_curr_vm->current_mode_state->ctx.pc = *( (uint32_t *) (_curr_vm->exception_vector[V_SWI] + (_curr_vm->current_mode_state->ctx.reg[7] << 2)));
		}
		else{
			//TODO Have not added check that its a valid private arm syscall, done anyways inside arm_syscall
			//if(_curr_vm->current_mode_state->ctx.reg[7] >= 0xF0000){ //NR_SYSCALL_BASE
			/*Arm private system call*/
			_curr_vm->current_mode_state->ctx.reg[0] = _curr_vm->mode_states[HC_GM_TASK].ctx.reg[7];
			_curr_vm->current_mode_state->ctx.reg[1] = _curr_vm->mode_states[HC_GM_KERNEL].ctx.sp + 8; //Adjust sp with S_OFF, contains regs
			_curr_vm->current_mode_state->ctx.pc = _curr_vm->exception_vector[V_ARM_SYSCALL];
		}
	}
	else if(_curr_vm->current_guest_mode != HC_GM_TASK){
	  //printf("\tHypercall number: %d (%x, %x) called\n", hypercall_number, param0, param1);
	  uint32_t res;
	  switch(hypercall_number){		 
	    /* TEMP: DMMU TEST */
  	        case 666:
		        //res = dmmu_handler(param0, param1, param2, _curr_vm->current_mode_state->ctx.reg[3]);
				
  	        	res = dmmu_handler(param0, param1, param2);
		        _curr_vm->current_mode_state->ctx.reg[0] = res;

				//Note: The below four rows must be here in order that the 
				//hypervisor has correct cache behaviour. This was revealed
				//when testing on the RPi2 hardware (this error did not show up
				//when testing on the Beagleboard simulator).
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
				return;
			case HYPERCALL_GUEST_INIT:
				hypercall_guest_init(param0);
				return;
			case HYPERCALL_INTERRUPT_SET:
				hypercall_interrupt_set(param0, param1);
				return;
			case HYPERCALL_END_INTERRUPT:
				hypercall_end_interrupt();
				return;
			case HYPERCALL_CACHE_OP:
				hypercall_cache_op(param0, param1, param2);
				return;
			case HYPERCALL_SET_TLS_ID:
				hypercall_set_tls(param0);
				return;
			case HYPERCALL_SET_CTX_ID:
				COP_WRITE(COP_SYSTEM,COP_CONTEXT_ID_REGISTER, param0);
				isb();
				return;
			/*Context*/
			case HYPERCALL_RESTORE_LINUX_REGS:
				hypercall_restore_linux_regs(param0, param1);
				return;
			case HYPERCALL_RESTORE_REGS:
				hypercall_restore_regs((uint32_t *)param0);
				return;

			/*Page table operations*/
			case HYPERCALL_SWITCH_MM:
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
				hypercall_dyn_switch_mm(param0, param1);
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
				//hypercall_switch_mm(param0, param1);
				return;

			case HYPERCALL_NEW_PGD:
				//hypercall_new_pgd((uint32_t*)param0);
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
				hypercall_dyn_new_pgd((uint32_t *)param0);
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
				return;
			case HYPERCALL_FREE_PGD:
				//hypercall_free_pgd((uint32_t*)param0);
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
				hypercall_dyn_free_pgd((uint32_t*)param0);
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
				return;
			case HYPERCALL_CREATE_SECTION:
				/*Not used anymore, DMMU init sets up everything in advance
				 *TODO remove call from linux kernel */
				//hypercall_create_section(param0,param1, param2);
				return;
			case HYPERCALL_SET_PMD:
#ifdef USE_DMMU
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
				hypercall_dyn_set_pmd(param0, param1);
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
#else
				hypercall_set_pmd((uint32_t*)param0, param1);
#endif
				return;
			case HYPERCALL_SET_PTE:
				//hypercall_set_pte((uint32_t*)param0, param1, param2);
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
				//CacheDataCleanInvalidateBuff((uint32_t)((addr_t *)((addr_t ) ((uint32_t*)param0) - 0x800)),4);
				hypercall_dyn_set_pte((uint32_t*)param0, param1,param2);
#ifdef AGGRESSIVE_FLUSHING_HANDLERS
				isb();
				mem_mmu_tlb_invalidate_all(TRUE, TRUE);
				CacheDataCleanInvalidateAll();
				dsb();
#endif
				return;
			/****************************/
			/*RPC*/
			case HYPERCALL_RPC:
				hypercall_rpc(param0, (uint32_t *)param1);
				return;
			case HYPERCALL_END_RPC:
				hypercall_end_rpc();
				return;
			default:
				hypercall_num_error(hypercall_number);
		}
	}
}

return_value prefetch_abort_handler(uint32_t addr, uint32_t status, uint32_t unused){
	virtual_machine* _curr_vm = get_curr_vm();
	if(addr >= 0xc0000000){
		printf("Prefetch abort: %x Status: %x, u= %x \n", addr, status, unused);
	}
	uint32_t interrupted_mode = _curr_vm->current_guest_mode;

	/*Need to be in virtual kernel mode to access data abort handler*/
	change_guest_mode(HC_GM_KERNEL);

    /*Set uregs, Linux kernel ususally sets these up in exception vector
     * which we have to handle now*/

    _curr_vm->mode_states[HC_GM_KERNEL].ctx.sp -= (72) ; //FRAME_SIZE (18 registers to be saved)

    uint32_t *sp = (uint32_t *)_curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;
    uint32_t *context = _curr_vm->mode_states[interrupted_mode].ctx.reg;
    uint32_t i;

    for(i = 0; i < 17; i++){
    	*sp++ = *context++;
    }
    *sp = 0xFFFFFFFF; //ORIG_R0

    /*Prepare args for prefetchabort handler*/
    _curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[0] = addr;
    _curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[1] = status;
    /*Linux saves the user registers in the stack*/
    _curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[2] = (uint32_t)_curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;


    if(!(_curr_vm->mode_states[HC_GM_KERNEL].ctx.psr & 0xF)){ //coming from svc
    	_curr_vm->mode_states[HC_GM_KERNEL].ctx.psr |= IRQ_MASK; //TODO DISABLE IRQnot neccessarily, check this
    }
   	else{
    	_curr_vm->mode_states[HC_GM_KERNEL].ctx.psr &= ~(IRQ_MASK); //ENABLE IRQ coming from usr
   	}

    /*Prepare pc for handler and lr to return from handler*/
	_curr_vm->mode_states[HC_GM_KERNEL].ctx.pc = _curr_vm->exception_vector[V_PREFETCH_ABORT];//(uint32_t)_curr_vm->handlers.pabort;
	_curr_vm->mode_states[HC_GM_KERNEL].ctx.lr = _curr_vm->exception_vector[V_RET_FROM_EXCEPTION];//(uint32_t)_curr_vm->handlers.ret_from_exception;
	//printf("Kernel PC:%x LR:%x \n",_curr_vm->mode_states[HC_GM_KERNEL].ctx.pc, _curr_vm->mode_states[HC_GM_KERNEL].ctx.lr);
    return RV_OK;
}

return_value data_abort_handler(uint32_t addr, uint32_t status, uint32_t unused){
	virtual_machine* _curr_vm = get_curr_vm();
	if(addr >= 0xc0000000){
		printf("Data abort: %x Status: %x, u= %x \n", addr, status, unused);
	}
    uint32_t interrupted_mode = _curr_vm->current_guest_mode;
    /*Must be in virtual kernel mode to access kernel handlers*/
    change_guest_mode(HC_GM_KERNEL);

    _curr_vm->mode_states[HC_GM_KERNEL].ctx.sp -= (72) ; //FRAME_SIZE (18 registers to be saved)
    /*Set uregs, Linux kernel ususally sets these up in exception vector
     * which we have to handle now*/

    uint32_t *sp = (uint32_t *)_curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;
    uint32_t *context = _curr_vm->mode_states[interrupted_mode].ctx.reg;
    uint32_t i;

    for(i = 0; i < 17; i++){
    	if(i == 13 && interrupted_mode == 1)
    		*sp++ = (*context++) + 72;
    	else
    		*sp++ = *context++;

    }
    *sp = 0xFFFFFFFF; //ORIG_R0
    //Context saved in sp

    /*Prepare args for dataabort handler*/
    _curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[0] = addr;
    _curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[1] = status;
    /*Linux saves the user registers in the stack*/
    _curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[2] = (uint32_t)_curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;

    if(!(_curr_vm->mode_states[HC_GM_KERNEL].ctx.psr & 0xF)){ //coming from svc
    	_curr_vm->mode_states[HC_GM_KERNEL].ctx.psr |= IRQ_MASK; //TODO DISABLE IRQnot neccessarily, check this
    	//
    }
   	else{
    	_curr_vm->mode_states[HC_GM_KERNEL].ctx.psr &= ~(IRQ_MASK); //ENABLE IRQ coming from usr
   	}

    /*Prepare pc for handler and lr to return from handler*/
    _curr_vm->mode_states[HC_GM_KERNEL].ctx.pc = _curr_vm->exception_vector[V_DATA_ABORT];//(uint32_t)_curr_vm->handlers.dabort;
   	_curr_vm->mode_states[HC_GM_KERNEL].ctx.lr = _curr_vm->exception_vector[V_RET_FROM_EXCEPTION];//(uint32_t)_curr_vm->handlers.ret_from_exception;

   	//printf("Kernel PC:%x LR:%x \n",_curr_vm->mode_states[HC_GM_KERNEL].ctx.pc, _curr_vm->mode_states[HC_GM_KERNEL].ctx.lr);

    return RV_OK;
}

return_value irq_handler(uint32_t irq, uint32_t r1, uint32_t r2 ){
	virtual_machine* _curr_vm = get_curr_vm();
	//printf("IRQ handler called %x:%x:%x\n", irq, r1, r2);
	/*Interrupt inside interrupt mode (i.e soft interrupt) */
	if(_curr_vm->current_guest_mode == HC_GM_INTERRUPT){
		_curr_vm->current_mode_state->ctx.psr |= IRQ_MASK;
		/*We dont handle reentrant IRQ... yet, let the current interrupt finish*/
		//Bypass it for now
		//Should redirect to usr_exit -> (uint32_t)_curr_vm->handlers.ret_from_exception;
//		_curr_vm->current_mode_state->ctx.pc = (uint32_t)_curr_vm->handlers.tick;
		return RV_OK;
	}

    _curr_vm->interrupted_mode = _curr_vm->current_guest_mode;
    change_guest_mode(HC_GM_INTERRUPT);
    _curr_vm->current_mode_state->ctx.reg[0] = irq;
    _curr_vm->current_mode_state->ctx.pc = _curr_vm->exception_vector[V_IRQ];//(uint32_t)_curr_vm->handlers.irq;
   	_curr_vm->current_mode_state->ctx.psr |= IRQ_MASK;
    //_curr_vm->current_mode_state->ctx.sp = _curr_vm->config->interrupt_config.sp;
   	_curr_vm->current_mode_state->ctx.sp = _curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;
    return RV_OK;
}

/*These are not handled yet*/
return_value undef_handler(uint32_t instr, uint32_t unused, uint32_t addr)
{
    printf("Undefined abort. Address: %x Instruction: %x \n", addr, instr);
    while(1);
    return RV_OK;
}
