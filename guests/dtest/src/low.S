

    .global syscall_dmmu   
    .code 32
    .balign 4
    

syscall_dmmu:
    svc #666
    bx lr
