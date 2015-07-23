//TODO: Move or re-move this file after debugging.

//Assembly function with noops. Needed to debug when you can't remove breakpoints.
.global debug_breakpoint
debug_breakpoint:
	nop     @ Put breakpoint at this address.
    nop     @ To continue, manually move the PC to this address.
    bx lr
