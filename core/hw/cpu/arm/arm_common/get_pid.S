.code 32
.align  0

.macro GET_PID
    mrc p15, 0, r0, c0, c0, 5
    and r0, r0, #0x3   
.endm

/*int get_pid() */
.global get_pid 
get_pid:
    GET_PID 
    bx lr

    
.global get_vm
get_vm:
    GET_PID
    bx lr

