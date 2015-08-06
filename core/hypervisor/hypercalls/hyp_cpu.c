#include "hw.h"
#include "hyper.h"

extern virtual_machine* get_curr_vm();

/*CPU and CO-PROCESSOR operations*/

#if 0
void hypercall_set_cpu_cr(uint32_t cpu_cr){
	virtual_machine* _curr_vm = get_curr_vm();
	if(_curr_vm->current_guest_mode != HC_GM_KERNEL)
				hyper_panic("User mode not allowed to access system control register\n");

	COP_WRITE(COP_SYSTEM,COP_SYSTEM_CONTROL,cpu_cr);
}


void hypercall_get_cpu_cr(){
	virtual_machine* _curr_vm = get_curr_vm();
	if(_curr_vm->current_guest_mode != HC_GM_KERNEL)
					hyper_panic("User mode not allowed to set system control register\n");

	uint32_t cpu_cr;
	COP_READ(COP_SYSTEM,COP_SYSTEM_CONTROL,cpu_cr);

	/*Return result in r0*/
	_curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[0] = cpu_cr;

}
#endif

void hypercall_set_tls(uint32_t thread_id)
{
	/*ARM 926 does not have this function*/
	#if ARM_ARCH > 5
	COP_WRITE(COP_SYSTEM, COP_SOFTWARE_THREAD_ID_USER_R, thread_id);
	#endif


}
