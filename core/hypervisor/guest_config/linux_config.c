#include "hyper_config_base.h"
#include "hyper_config.h"


// #define GUEST_LOCATION 0xF1000000
#define TRUSTED_LOCATION 0xF0500000
#define TRUSTED_ENTRY (TRUSTED_LOCATION)
#define TRUSTED_RPC   ((TRUSTED_LOCATION) + 4)
#define TRUSTED_SP    ((TRUSTED_LOCATION) + 0x00100000 - 4) /* ??? */
 


/*
 * Bitmask constants for specifying guest mode
 * contexts that can be get/set.
 */
#define HC_GM_TRUSTED_MASK   (1 << HC_GM_TRUSTED)
#define HC_GM_KERNEL_MASK    (1 << HC_GM_KERNEL)
#define HC_GM_INTERRUPT_MASK (1 << HC_GM_INTERRUPT)
#define HC_GM_TASK_MASK      (1 << HC_GM_TASK)

/*
 * Guest mode access to certain domains
 * ********************************************************/

#define HC_DOMAC_ALL \
	((1 << (2 * HC_DOM_DEFAULT)) | \
	(1 << (2 * HC_DOM_TASK)) | \
	(1 << (2 * HC_DOM_KERNEL)) | \
	(1 << (2 * HC_DOM_TRUSTED)))

#define HC_DOMAC_KERNEL  \
	((1 << (2 * HC_DOM_DEFAULT)) | \
	(1 << (2 * HC_DOM_KERNEL)) | \
	(1 << (2 * HC_DOM_TASK)))

#define HC_DOMAC_TRUSTED \
	((1 << (2 * HC_DOM_DEFAULT)) | \
	(1 << (2 * HC_DOM_TRUSTED)))

#define HC_DOMAC_INTERRUPT \
	((1 << (2 * HC_DOM_DEFAULT)) | \
	(1 << (2 * HC_DOM_KERNEL)) | \
	(1 << (2 * HC_DOM_TASK)))

#define HC_DOMAC_TASK \
	((1 << (2 * HC_DOM_DEFAULT)) | \
	(1 << (2 * HC_DOM_TASK)))

/************************************************************/

/*
 * Configuration for guest modes
 */

static const hc_guest_mode
	gm_trusted = {
			.name = "trusted",
			.domain_ac = HC_DOMAC_TRUSTED,
	},
	gm_kernel = {
			.name = "kernel",
			.domain_ac = HC_DOMAC_KERNEL,

	},
	gm_task = {
			.name = "application",
			.domain_ac = HC_DOMAC_TASK,
	},
	gm_interrupt = {
			.name = "interrupt",
			.domain_ac = HC_DOMAC_INTERRUPT,
	};


/*RPC handler*/
static const hc_rpc_handler	rpc_handler_trusted = {
		.name = "trusted_rpc_handler",
		.mode = HC_GM_TRUSTED,
		.entry_point = TRUSTED_RPC,
		.sp = TRUSTED_SP
		};


/*
 * Guest configuration structure
 */

hc_config linux_config = {
		.guest_entry_offset = 0x10000,
		.guest_modes = {&gm_trusted, &gm_kernel, &gm_task, &gm_interrupt},
		.rpc_handlers = &rpc_handler_trusted,
		.reserved_va_for_pt_access_start = 0xE8000000,
		// Offset respect the initial pa of the guest
		.pa_initial_l1_offset = 0x00004000, // Initial address + 2MB
		.pa_initial_l2_offset = 0x00001000
};
