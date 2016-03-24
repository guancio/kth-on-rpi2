/*
 * ARM pagetable function including the initial setup 
 */

    .global arm_setup_initial_pt

    .code 32
    .align  0

#include "arm_common.h"

/*
 * SANITY CHECKS:
 *
 * The code in this page makes some assumptions, check they are true
 */
 
#if (__hyper_pt_start__ & (1024 * 16-1)) != 0
 #error "__hyper_pt_start__ is not 16K aligned!"
#endif

#if (__hyper_pt_end__ & (1024 * 1-1)) != 0
 #error "__hyper_pt_end__ is not 1K aligned!"
#endif

#if (HAL_PHYS_START & (1024 * 4)) != 0
 #error "HAL_PHYS_START is not 4K aligned!"
#endif

#if (HAL_VIRT_START & (1024 * 4-1)) != 0
 #error "HAL_VIRT_START is not 4K aligned!"
#endif


/*
 * set the initial page table
 */

arm_setup_initial_pt:

	/*Start with simplest possible needed to get sth running
 	  in virtual address
 	  */

	ldr r4, = (__hyper_pt_start__ + HAL_OFFSET)

	mov r0, r4
	mov r1, #0
	add r2, r0, #0x4000
	/* first, clean the memory used to make all pages invalid */
1:	str r1, [r0], #4
	str r1, [r0], #4
	str r1, [r0], #4
	str r1, [r0], #4
	teq r0, r2
	bne 1b

 	/* do a 1->1 temporary mapping for our current position */
	ldr r6, =(HAL_PHYS_START >> 20)
	ldr r7, =(HAL_PHYS_START | 0x40E)	@ CB on, Section, XN=f, AP =01 PRIV RW USER NO ACCESS
	str r7, [r4, r6, lsl #2]			@ 1:1 mapping for current position

	/* then map us from HAL_VIR_START to HAL_PHY_START in L1 */
	add r0, r4, #(HAL_VIRT_START & 0xff000000) >> 18
	str r7, [r0,#(HAL_VIRT_START & 0x00f00000) >> 18]! @ Map phys to HAL_VIRT_START

	/*Enable MMU*/
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 4 	@ drain write buffer
	mcr p15, 0, r0, c8, c7, 0  	@ flush I, D, TLB

	mcr p15, 0, r0, c1, c0, 0	@ read control register
	orr r0, r0 , #1				@ enable MMU bit
	mov r1, #-1					@ sets manager to all domains
	mcr p15, 0, r1, c3, c0, 0	@ load domain access control
	mcr p15, 0, r4, c2, c0, 0	@ load page table pointer

	mcr p15, 0, r0, c1, c0, 0	@ write control reg

    /* adjust stack and jump to our virtual address */
    ldr r1, =HAL_OFFSET
    sub sp, sp, r1
    sub pc, pc, r1

    nop     @ for PC jump above
    nop     @ for PC jump above

	/*Now running in virt space, we can remove 1:1 mapping now*/
//	mov r0, #0
//	ldr r3, =(HAL_PHYS_START >> 20)
//	str r0, [r4, r3, lsl #2]

	sub pc, lr, r1				@ correct lr to virt space
