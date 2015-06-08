        .global CP15ICacheDisable
        .global CP15UnifiedCacheDisable
        .global CP15DCacheDisable
        .global CP15ICacheEnable
        .global CP15DCacheEnable
        .global CP15ICacheFlush
        .global CP15DCacheCleanFlush
        .global CP15DCacheClean
        .global CP15DCacheFlush
        .global CP15DCacheCleanBuff
        .global CP15DCacheCleanFlushBuff
        .global CP15DCacheFlushBuff
        .global CP15ICacheFlushBuff
        .global CP15Ttb0Set
        .global CP15TlbInvalidate
        .global CP15MMUDisable
        .global CP15MMUEnable
        .global CP15VectorBaseaddrSet
        .global CP15BranchPredictorInvalidate
        .global CP15BranchPredictionEnable
        .global CP15BranchPredictionDisable
        .global CP15DomainAccessClientSet
        .global CP15ControlFeatureDisable
        .global CP15ControlFeatureEnable
        .global CP15TtbCtlTtb0Config
        .global CP15AuxControlFeatureEnable
        .global CP15AuxControlFeatureDisable
        .global CP15MainIdPrimPartNumGet


CP15ICacheDisable:
    mrc     p15, #0, r0, c1, c0, #0
    bic     r0,  r0, #0x00001000
    mcr     p15, #0, r0, c1, c0, #0
    bx      lr

CP15UnifiedCacheDisable:
	mrc p15, #0, r0, c1, c0, #1
	bic r0, r0, #0x00000002
	mcr p15, #0, r0, c1, c0, #1

CP15DCacheDisable:
    push    {r4-r11, lr}
    mrc     p15, #0, r0, c1, c0, #0
    bic     r0,  r0, #0x00000004
    mcr     p15, #0, r0, c1, c0, #0
    BL      CP15DCacheCleanFlush
    pop     {r4-r11, lr}
    bx      lr

CP15ICacheEnable:
    mrc     p15, #0, r0, c1, c0, #0
    orr     r0,  r0, #0x00001000
    mcr     p15, #0, r0, c1, c0, #0
    bx      lr

CP15DCacheEnable:
    mrc     p15, #0, r0, c1, c0, #0
    orr     r0,  r0, #0x00000004
    mcr     p15, #0, r0, c1, c0, #0
    bx      lr

CP15DCacheFlush:
    push    {r4-r11}
    dmb
    mrc     p15, #1, r0, c0, c0, #1  @@ Read CLID register
    and    r3, r0, #0x7000000       @@ Get Level of Coherency
    mov     r3, r3, lsr #23
    beq     ffinished
    mov     r10, #0
floop1:
    add     r2, r10, r10, lsr #1
    mov     r1, r0, lsr r2
    and     r1, r1, #7
    cmp     r1, #2
    blt     fskip
    mcr     p15, #2, r10, c0, c0, #0
    isb
    mrc     p15, #1, r1, c0, c0, #0
    and     r2, r1, #7
    add     r2, r2, #4
    ldr     r4, _FLD_MAX_WAY
    and    r4, r4, r1, lsr #3
    clz     r5, r4
    ldr     r7, _FLD_MAX_IDX
    and    r7, r7, r1, lsr #13
floop2:
    mov     r9, r4
floop3:
    orr     r11, r10, r9, lsl r5
    orr     r11, r11, r7, lsl r2
    mcr     p15, #0, r11, c7, c6, #2
    subs    r9, r9, #1
    bge     floop3
    subs    r7, r7, #1
    bge     floop2
fskip:
    add     r10, r10, #2
    cmp     r3, r10
    bgt     floop1

ffinished:
    dsb
    isb
    pop     {r4-r11}
    bx      lr

CP15DCacheClean:
    push    {r4-r11}
    dmb
    mrc     p15, #1, r0, c0, c0, #1  @@ Read CLID register
    and    r3, r0, #0x7000000       @@ Get Level of Coherency
    mov     r3, r3, lsr #23
    beq     cfinished
    mov     r10, #0
cloop1:
    add     r2, r10, r10, lsr #1
    mov     r1, r0, lsr r2
    and     r1, r1, #7
    cmp     r1, #2
    blt     cskip
    mcr     p15, #2, r10, c0, c0, #0
    isb
    mrc     p15, #1, r1, c0, c0, #0
    and     r2, r1, #7
    add     r2, r2, #4
    ldr     r4, _FLD_MAX_WAY
    and    r4, r4, r1, lsr #3
    clz     r5, r4
    ldr     r7, _FLD_MAX_IDX
    and    r7, r7, r1, lsr #13
cloop2:
    mov     r9, r4
cloop3:
    orr     r11, r10, r9, lsl r5
    orr     r11, r11, r7, lsl r2
    mcr     p15, #0, r11, c7, c10, #2
    subs    r9, r9, #1
    bge     cloop3
    subs    r7, r7, #1
    bge     cloop2
cskip:
    add     r10, r10, #2
    cmp     r3, r10
    bgt     cloop1

cfinished:
    dsb
    isb
    pop     {r4-r11}
    bx      lr

CP15DCacheCleanFlush:
    push    {r4-r11}
    dmb
    mrc     p15, #1, r0, c0, c0, #1  @@ Read CLID register
    and    r3, r0, #0x7000000       @@ Get Level of Coherency
    mov     r3, r3, lsr #23
    beq     finished
    mov     r10, #0
loop1:
    add     r2, r10, r10, lsr #1
    mov     r1, r0, lsr r2
    and     r1, r1, #7
    cmp     r1, #2
    blt     skip
    mcr     p15, #2, r10, c0, c0, #0
    isb
    mrc     p15, #1, r1, c0, c0, #0
    and     r2, r1, #7
    add     r2, r2, #4
    ldr     r4, _FLD_MAX_WAY
    and    r4, r4, r1, lsr #3
    clz     r5, r4
    ldr     r7, _FLD_MAX_IDX
    and    r7, r7, r1, lsr #13
loop2:
    mov     r9, r4
loop3:
    orr     r11, r10, r9, lsl r5
    orr     r11, r11, r7, lsl r2
    mcr     p15, #0, r11, c7, c14, #2
    subS    r9, r9, #1
    bge     loop3
    subS    r7, r7, #1
    bge     loop2
skip:
    add     r10, r10, #2
    cmp     r3, r10
    bgt     loop1

finished:
    dsb
    isb
    pop     {r4-r11}
    bx      lr

CP15ICacheFlush:
    mov     r0, #0
    mcr     p15, #0, r0, c7, c5, #0
    dsb
    bx      lr

CP15DCacheCleanBuff:
    push    {r14}
    add     r14, r0, r1               @@ Calculate the end address
    dmb
    mrc     p15, #0, r2, c0, c0, #1   @@ Read Cache Type Register
    ubfx    r2, r2, #16, #4           @@ Extract the DMinLine
    mov     r3, #2
    add     r3, r3, r2
    mov     r2, #1
    lsl     r2, r2, r3                @@ Calculate the line size

    sub     r3, r2, #1                @@ Calculate the mask
    bic     r0, r0, r3                @@ Align to cache line boundary
    TST     r3, r14
    bic     r14, r14, r3
    mcrNE   p15, #0, r14, c7, c10, #1 @@ Clean D/Unified to PoC by MVA

cleanloop:
    mcr     p15, #0, r0 , c7, c10, #1 @@ Clean D/Unified to PoC by MVA
    addS    r0, r0, r2                @@ Go to next line
    cmp     r0, r14
    blt     cleanloop

    pop     {r14}
    dsb
    bx      lr

CP15DCacheCleanFlushBuff:
    push    {r14}
    add     r14, r0, r1               @@ Calculate the end address
    dmb
    mrc     p15, #0, r2, c0, c0, #1   @@ Read Cache Type Register
    ubfx    r2, r2, #16, #4           @@ Extract the DMinLine
    mov     r3, #2
    add     r3, r3, r2
    mov     r2, #1
    lsl     r2, r2, r3                @@ Calculate the line size

    sub     r3, r2, #1                @@ Calculate the mask
    bic     r0, r0, r3                @@ Align to cache line boundary
    TST     r3, r14
    bic     r14, r14, r3
    mcrNE   p15, #0, r14, c7, c14, #1 @@ Clean and Flush D/U line to PoC

cleanflushloop:
    mcr     p15, #0, r0 , c7, c14, #1 @@ Clean and Flush D/U line to PoC
    addS    r0, r0, r2                @@ Go to next line
    cmp     r0, r14
    blt     cleanflushloop

    pop     {r14}
    dsb
    bx      lr


CP15DCacheFlushBuff:
    push    {r14}
    add     r14, r0, r1               @@ Calculate the end address
    dmb
    mrc     p15, #0, r2, c0, c0, #1   @@ Read Cache Type Register
    ubfx    r2, r2, #16, #4           @@ Extract the DMinLine
    mov     r3, #2
    add     r3, r3, r2
    mov     r2, #1
    lsl     r2, r2, r3                @@ Calculate the line size

    sub     r3, r2, #1                @@ Calculate the mask
    TST     r3, r0
    bic     r0, r0, r3                @@ Align to cache line boundary
    mcrNE   p15, #0, r0, c7, c14, #1  @@ Clean and Flush D/U line to PoC
    addNE   r0, r0, r2
    TST     r3, r14
    bic     r14, r14, r3
    mcrNE   p15, #0, r14, c7, c14, #1 @@ Clean and Flush D/U line to PoC
    B       dflushcmp

dflushloop:
    mcr     p15, #0, r0 , c7, c6, #1  @@ Flush D/U line to PoC
    addS    r0, r0, r2                @@ Go to next line

dflushcmp:
    cmp     r0, r14
    blt     dflushloop
    pop     {r14}
    dsb
    bx      lr

CP15ICacheFlushBuff:
    push    {r14}
    add     r14, r0, r1               @@ Calculate the end address
    dmb
    mrc     p15, #0, r2, c0, c0, #1   @@ Read Cache Type Register
    ubfx    r2, r2, #0, #4            @@ Extract the DMinLine
    mov     r3, #2
    add     r3, r3, r2
    mov     r2, #1
    lsl     r2, r2, r3                @@ Calculate the line size

    sub     r3, r2, #1                @@ Calculate the mask
    bic     r0, r0, r3                @@ Align to cache line boundary
    TST     r3, r14
    bic     r14, r14, r3
    mcrne   p15, #0, r14, c7, c5, #1  @@ Invalidate by MVA to PoU

iflushloop:
    mcr     p15, #0, r0, c7, c5, #1   @@ Invalidate by MVA to PoU
    addS    r0, r0, r2                @@ Go to next line
    cmp     r0, r14
    blt     iflushloop

    pop     {r14}
    dsb
    bx      lr

CP15Ttb0Set:
    mcr     p15, #0, r0, c2, c0, #0
    dmb
    bx      lr

CP15TlbInvalidate:
    mcr     p15, #0, r0, c8, c7, #0    @@ r0 value will be ignored
    dsb
    bx      lr

CP15MMUDisable:
    mcr     p15, #0, r0, c8, c7, #0    @@ Invalidate TLB
    mrc     p15, #0, r0, c1, c0, #0
    bic     r0, r0, #1
    mcr     p15, #0, r0, c1, c0, #0    @@ Clear MMU bit
    dsb
    bx      lr

CP15MMUEnable:
    mrc     p15, #0, r0, c1, c0, #0
    orr     r0, r0, #0x001
    mcr     p15, #0, r0, c1, c0, #0    @@ Set MMU Enable bit
    dsb
    bx      lr

CP15VectorBaseaddrSet:
    mcr     p15, #0, r0, c12, c0, #0
    dsb
    bx      lr

CP15BranchPredictorInvalidate:
    mcr     p15, #0, r0, c7, c5, #6
    isb
    bx      lr

CP15BranchPredictionEnable:
    mrc     p15, #0, r0, c1, c0, #0
    orr     r0, r0, #0x00000800
    mcr     p15, #0, r0, c1, c0, #0
    bx      lr

CP15BranchPredictionDisable:
    mrc     p15, #0, r0, c1, c0, #0
    bic     r0, r0, #0x00000800
    mcr     p15, #0, r0, c1, c0, #0
    bx      lr

CP15DomainAccessClientSet:
    ldr     r0, _CLIENTD
    mcr     p15, #0, r0, c3, c0, #0
    dsb
    bx      lr


CP15ControlFeatureDisable:
    mrc     p15, #0, r1, c1, c0, #0
    bic     r0, r1, r0
    mcr     p15, #0, r0, c1, c0, #0
    dsb
    bx      lr

CP15ControlFeatureEnable:
    mrc     p15, #0, r1, c1, c0, #0
    orr     r0, r1, r0
    mcr     p15, #0, r0, c1, c0, #0
    dsb
    bx      lr

CP15TtbCtlTtb0Config:
    mov     r0, #0x0
    mcr     p15, #0, r0, c2, c0, #2
    dsb
    bx      lr

CP15AuxControlFeatureEnable:
    mrc     p15, #0, r1, c1, c0, #1
    orr     r0,  r0, r1
    mcr     p15, #0, r0, c1, c0, #1
    dsb
    bx      lr

CP15AuxControlFeatureDisable:
    mrc     p15, #0, r1, c1, c0, #1
    bic     r0,  r1, r0
    mcr     p15, #0, r0, c1, c0, #1
    dsb
    bx      lr

CP15MainIdPrimPartNumGet:
    mrc     p15, #0, r0, c0, c0, #0
    ubfx    r0, r0, #4, #12
    bx      lr

_CLIENTD:
   .word  0x55555555
_FLD_MAX_WAY:
   .word  0x3ff
_FLD_MAX_IDX:
   .word  0x7ff
