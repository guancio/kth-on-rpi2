

    .global syscall_dmmu
    .global syscall_cache_op
    .code 32
    .balign 4
    

syscall_dmmu:
    svc # 666
    bx lr

syscall_cache_op:
	svc # 1009
	bx lr
