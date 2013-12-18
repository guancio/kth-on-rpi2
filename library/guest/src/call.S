    .global do_svc0
    .code 32

do_svc0:    
    push {r4-r7}
    ldmia r0, {r0-r7}
    svc #0
    pop {r4, r7}
    mov pc, lr
