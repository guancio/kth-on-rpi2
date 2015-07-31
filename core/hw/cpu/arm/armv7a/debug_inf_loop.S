//TODO: Move or re-move this file after debugging.

//Infinite loop, for when that comes in handy.
.global debug_inf_loop
debug_inf_loop:
	ldr r0, =0x47 @ Set r0 to 0x47.
.global loopback_point
loopback_point:
	cmp r0, #0x11 @ Compare r0 to 0x11.
	blne loopback_point @ Loop if r0 is not equal to 0x11.
	bx lr
