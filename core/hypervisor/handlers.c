#include <hw.h>
#include "hypercalls.h"

extern virtual_machine *curr_vm;

#define USE_DMMU

void swi_handler(uint32_t param0, uint32_t param1, uint32_t param2, uint32_t hypercall_number)
{
    
	/*TODO Added check that controls if it comes from user space, makes it pretty inefficient, remake later*/
	/*Testing RPC from user space, remove later*/
	if(curr_vm->current_guest_mode == HC_GM_TASK){
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
    if(curr_vm->current_guest_mode == HC_GM_TASK){
        
	//	debug("\tUser process made system call:\t\t\t %x\n",  curr_vm->mode_states[HC_GM_TASK].ctx.reg[7] );
		change_guest_mode(HC_GM_KERNEL);
		/*The current way of saving context by the hypervisor is very inefficient,
		 * can be improved alot with some hacking and shortcuts (for the FUTURE)*/
		curr_vm->current_mode_state->ctx.sp -= (72 + 8) ; //FRAME_SIZE (18 registers to be saved) + 2 swi args
		uint32_t *context, *sp, i;

		context = &curr_vm->mode_states[HC_GM_TASK].ctx.reg[0];
		sp 		=(uint32_t *) curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;

		*sp++ = curr_vm->mode_states[HC_GM_TASK].ctx.reg[4];
		*sp++ = curr_vm->mode_states[HC_GM_TASK].ctx.reg[5];

		i = 17; //r0-lr

		while ( i > 0){
			*sp++ = *context++;
			i--;
		}

		*sp = curr_vm->mode_states[HC_GM_TASK].ctx.reg[0]; //OLD_R0

		//update CR for alignment fault
		//Enable IRQ
		curr_vm->current_mode_state->ctx.psr &= ~(IRQ_MASK);


		curr_vm->current_mode_state->ctx.lr = curr_vm->exception_vector[V_RET_FAST_SYSCALL];//curr_vm->handlers.syscall.ret_fast_syscall;
		//copy task context to kernel context. syscall supports 6 arguments

		/*system call nr in r7*/
		curr_vm->current_mode_state->ctx.reg[7] = curr_vm->mode_states[HC_GM_TASK].ctx.reg[7];


		if(curr_vm->mode_states[HC_GM_TASK].ctx.reg[7] < curr_vm->guest_info.nr_syscalls ){
			/*Regular system call, restore params*/
			for(i =0;i <= 5; i++)
				curr_vm->current_mode_state->ctx.reg[i] = curr_vm->mode_states[HC_GM_TASK].ctx.reg[i];
			/*Set PC to systemcall function*/
			curr_vm->current_mode_state->ctx.pc = *( (uint32_t *) (curr_vm->exception_vector[V_SWI] + (curr_vm->current_mode_state->ctx.reg[7] << 2)));
		}
		else{
			//TODO Have not added check that its a valid private arm syscall, done anyways inside arm_syscall
			//if(curr_vm->current_mode_state->ctx.reg[7] >= 0xF0000){ //NR_SYSCALL_BASE
			/*Arm private system call*/
			curr_vm->current_mode_state->ctx.reg[0] = curr_vm->mode_states[HC_GM_TASK].ctx.reg[7];
			curr_vm->current_mode_state->ctx.reg[1] = curr_vm->mode_states[HC_GM_KERNEL].ctx.sp + 8; //Adjust sp with S_OFF, contains regs
			curr_vm->current_mode_state->ctx.pc = curr_vm->exception_vector[V_ARM_SYSCALL];
		}
	}
	else if(curr_vm->current_guest_mode != HC_GM_TASK){
	  //    printf("\tHypercallnumber: %d (%x, %x) called\n", hypercall_number, param0, param);
	  uint32_t res;
	  switch(hypercall_number){				 
	    /* TEMP: DMMU TEST */
  	        case 666:
		        //res = dmmu_handler(param0, param1, param2, curr_vm->current_mode_state->ctx.reg[3]);
  	        	res = dmmu_handler(param0, param1, param2);
		        curr_vm->current_mode_state->ctx.reg[0] = res;
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
				hypercall_switch_mm(param0, param1);
				return;

			case HYPERCALL_NEW_PGD:
				hypercall_new_pgd((uint32_t*)param0);
				return;
			case HYPERCALL_FREE_PGD:
				hypercall_free_pgd((uint32_t*)param0);
				return;
			case HYPERCALL_CREATE_SECTION:
				//hypercall_create_section(param0,param1, param2);
				return;
			case HYPERCALL_SET_PMD:
#ifdef USE_DMMU
				hypercall_dyn_set_pmd(param0, param1);
#else
				hypercall_set_pmd((uint32_t*)param0, param1);
#endif
				return;
			case HYPERCALL_SET_PTE:
				hypercall_set_pte((uint32_t*)param0, param1, param2);
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

return_value prefetch_abort_handler(uint32_t addr, uint32_t status, uint32_t unused)
{
	if(addr >= 0xc0000000)
	  printf("Pabort:%x Status:%x, u=%x \n", addr, status, unused);

	uint32_t interrupted_mode = curr_vm->current_guest_mode;

	/*Need to be in virtual kernel mode to access data abort handler*/
	change_guest_mode(HC_GM_KERNEL);

    /*Set uregs, Linux kernel ususally sets these up in exception vector
     * which we have to handle now*/

    curr_vm->mode_states[HC_GM_KERNEL].ctx.sp -= (72) ; //FRAME_SIZE (18 registers to be saved)

    uint32_t *sp = (uint32_t *)curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;
    uint32_t *context = curr_vm->mode_states[interrupted_mode].ctx.reg;
    uint32_t i;

    for(i = 0; i < 17; i++){
    	*sp++ = *context++;
    }
    *sp = 0xFFFFFFFF; //ORIG_R0

    /*Prepare args for prefetchabort handler*/
    curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[0] = addr;
    curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[1] = status;
    /*Linux saves the user registers in the stack*/
    curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[2] = (uint32_t)curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;


    if(!(curr_vm->mode_states[HC_GM_KERNEL].ctx.psr & 0xF)){ //coming from svc
    	curr_vm->mode_states[HC_GM_KERNEL].ctx.psr |= IRQ_MASK; //TODO DISABLE IRQnot neccessarily, check this
    }
   	else{
    	curr_vm->mode_states[HC_GM_KERNEL].ctx.psr &= ~(IRQ_MASK); //ENABLE IRQ coming from usr
   	}

    /*Prepare pc for handler and lr to return from handler*/
	curr_vm->mode_states[HC_GM_KERNEL].ctx.pc = curr_vm->exception_vector[V_PREFETCH_ABORT];//(uint32_t)curr_vm->handlers.pabort;
	curr_vm->mode_states[HC_GM_KERNEL].ctx.lr = curr_vm->exception_vector[V_RET_FROM_EXCEPTION];//(uint32_t)curr_vm->handlers.ret_from_exception;
	//printf("Kernel PC:%x LR:%x \n",curr_vm->mode_states[HC_GM_KERNEL].ctx.pc, curr_vm->mode_states[HC_GM_KERNEL].ctx.lr);
    return RV_OK;
}

return_value data_abort_handler(uint32_t addr, uint32_t status, uint32_t unused)
{
	if(addr >= 0xc0000000)
	  printf("Dabort:%x Status:%x, u=%x \n", addr, status, unused);

    uint32_t interrupted_mode = curr_vm->current_guest_mode;
    /*Must be in virtual kernel mode to access kernel handlers*/
    change_guest_mode(HC_GM_KERNEL);

    curr_vm->mode_states[HC_GM_KERNEL].ctx.sp -= (72) ; //FRAME_SIZE (18 registers to be saved)
    /*Set uregs, Linux kernel ususally sets these up in exception vector
     * which we have to handle now*/

    uint32_t *sp = (uint32_t *)curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;
    uint32_t *context = curr_vm->mode_states[interrupted_mode].ctx.reg;
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
    curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[0] = addr;
    curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[1] = status;
    /*Linux saves the user registers in the stack*/
    curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[2] = (uint32_t)curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;

    if(!(curr_vm->mode_states[HC_GM_KERNEL].ctx.psr & 0xF)){ //coming from svc
    	curr_vm->mode_states[HC_GM_KERNEL].ctx.psr |= IRQ_MASK; //TODO DISABLE IRQnot neccessarily, check this
    	//
    }
   	else{
    	curr_vm->mode_states[HC_GM_KERNEL].ctx.psr &= ~(IRQ_MASK); //ENABLE IRQ coming from usr
   	}

    /*Prepare pc for handler and lr to return from handler*/
    curr_vm->mode_states[HC_GM_KERNEL].ctx.pc = curr_vm->exception_vector[V_DATA_ABORT];//(uint32_t)curr_vm->handlers.dabort;
   	curr_vm->mode_states[HC_GM_KERNEL].ctx.lr = curr_vm->exception_vector[V_RET_FROM_EXCEPTION];//(uint32_t)curr_vm->handlers.ret_from_exception;

   	//printf("Kernel PC:%x LR:%x \n",curr_vm->mode_states[HC_GM_KERNEL].ctx.pc, curr_vm->mode_states[HC_GM_KERNEL].ctx.lr);

    return RV_OK;
}

return_value irq_handler(uint32_t irq, uint32_t r1, uint32_t r2 )
{
//	printf("IRQ handler called %x:%x:%x\n", irq, r1, r2);
	/*Interrupt inside interrupt mode (i.e soft interrupt) */
	if(curr_vm->current_guest_mode == HC_GM_INTERRUPT){
		curr_vm->current_mode_state->ctx.psr |= IRQ_MASK;
		/*We dont handle reentrant IRQ... yet, let the current interrupt finish*/
		//Bypass it for now
		//Should redirect to usr_exit -> (uint32_t)curr_vm->handlers.ret_from_exception;
//		curr_vm->current_mode_state->ctx.pc = (uint32_t)curr_vm->handlers.tick;
		return RV_OK;
	}

    curr_vm->interrupted_mode = curr_vm->current_guest_mode;
    change_guest_mode(HC_GM_INTERRUPT);
    curr_vm->current_mode_state->ctx.reg[0] = irq;
    curr_vm->current_mode_state->ctx.pc = curr_vm->exception_vector[V_IRQ];//(uint32_t)curr_vm->handlers.irq;
   	curr_vm->current_mode_state->ctx.psr |= IRQ_MASK;
    //curr_vm->current_mode_state->ctx.sp = curr_vm->config->interrupt_config.sp;
   	curr_vm->current_mode_state->ctx.sp = curr_vm->mode_states[HC_GM_KERNEL].ctx.sp;
    return RV_OK;
}

/*These are not handled yet*/
return_value undef_handler(uint32_t instr, uint32_t unused, uint32_t addr)
{
    printf("Undefined abort\n Address:%x Instruction:%x \n", addr, instr);
    while(1);
    return RV_OK;
}
