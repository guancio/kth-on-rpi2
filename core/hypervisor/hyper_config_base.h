#ifndef _HYPER_CONFIG_BASE_H_
#define _HYPER_CONFIG_BASE_H_

#include "types.h"
#include "guest_blob.h"

/*
 * We need this to know how many modes,
 * memory regions, etc. exist.
 */
#include "hyper_config.h"


/*
 * Describes a guest execution mode, including the domain access control
 * bits that are set when entering the mode.
 * Guest modes are "virtual" -- they are logical submodes of the
 * non-privileged CPU mode.
 *
 * There should be 32 or fewer guest modes.
 */
typedef struct hc_guest_mode_ {
	const char* name;
	uint32_t domain_ac;     /* Domain AC bitmap for this mode. */
} hc_guest_mode;

/*
 * Describes an RPC handler -- a way by which one guest mode
 * can send messages to (aka "call") another guest mode.
 *
 */
typedef struct hc_rpc_handler_ {
	const char* name;
	uint32_t mode; /* The guest mode this handler executes in. */
	addr_t entry_point;
	uint32_t sp; /* Set the user-mode SP to this when entering. */
} hc_rpc_handler;


/*
 * Mostly for debugging -- a description of a
 * memory domain.
 */
typedef struct hc_mem_domain_ {
	const char* name;
} hc_mem_domain;


/*
 * Config structure that the hypervisor will use to
 * set up everything.
 */
typedef struct hc_config_ {
    struct guest_binary *firmware;
    addr_t guest_entry_offset;
  const hc_guest_mode *guest_modes[HC_NGUESTMODES];
  const hc_rpc_handler *rpc_handlers;
  const addr_t reserved_va_for_pt_access_start;
  const addr_t pa_initial_l1_offset;
  const addr_t pa_initial_l2_offset;
} hc_config;

#endif
