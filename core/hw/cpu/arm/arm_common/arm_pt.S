/*
 * ARM pagetable functions including the initial setup 
 */
    .global arm_setup_initial_pt
	.global arm_setup_initial_pt_slave
	.global arm_reset_initial_pt

    .code 32
    .align  0

#include "arm_common.h"

/*
 * SANITY CHECKS:
 *
 * The code in this page makes some assumptions, and checks if they are true.
 */
 
#if (__hyper_pt_start__ & (1024 * 16-1)) != 0
 #error "__hyper_pt_start__ is not 16K aligned!"
#endif

#if (__hyper_pt_end__ & (1024 * 1-1)) != 0
 #error "__hyper_pt_end__ is not 1K aligned!"
#endif

#if (__hyper_pt_start_slave__ & (1024 * 16-1)) != 0
 #error "__hyper_pt_start_slave__ is not 16K aligned!"
#endif

#if (__hyper_pt_end_slave__ & (1024 * 1-1)) != 0
 #error "__hyper_pt_end_slave__ is not 1K aligned!"
#endif

#if (HAL_PHYS_START & (1024 * 4)) != 0
 #error "HAL_PHYS_START is not 4K aligned!"
#endif

#if (HAL_VIRT_START & (1024 * 4-1)) != 0
 #error "HAL_VIRT_START is not 4K aligned!"
#endif


/*
 * Set up the initial page table.
 */

arm_setup_initial_pt_slave:
	/* Start with the simplest possible PT needed to get the second guest
	 * running in virtual address space (we will switch to the master page table
	 * later).
 	 */

	/*mov r0, = #(__hyper_pt_start_slave__ + HAL_OFFSET)*/
	/* Place r0 at the physical starting address of the secondary (slave) page
	 * table. */
	ldr r0, =__hyper_pt_start_slave__
	ldr r1, =HAL_OFFSET
	add r0, r0, r1
	ldr r4, r0

	/* Place zero in r1 for later usage, and store the physical starting address
	 * of the secondary page table with an offset of 0x4000 in r2. */
	//mov r1, #0
	//add r2, r0, #0x4000

	/* First, clean the memory used to make all pages invalid. */
	/* TODO: No need to do this again? mov r1, #0 is be removable in that case
1:	str r1, [r0], #4
	str r1, [r0], #4
	str r1, [r0], #4
	str r1, [r0], #4
	teq r0, r2
	bne 1b */

 	/* Do a 1->1 temporary mapping for our current position.
	   TODO: Should already be done.
	ldr r6, =(HAL_PHYS_START >> 20)
	ldr r7, =(HAL_PHYS_START | 0x40E)	@ CB on, Section, XN=f, AP =01 PRIV RW USER NO ACCESS
	str r7, [r4, r6, lsl #2]			@ 1:1 mapping for current position */

	/* Then, map us from HAL_VIR_START to HAL_PHY_START in L1. 
	 * TODO: Changes needed here? */
	//add r0, r4, #(HAL_VIRT_START & 0xff000000) >> 18
	//str r7, [r0, #(HAL_VIRT_START & 0x00f00000) >> 18]! @ Map phys to HAL_VIRT_START

	/* Enable MMU
	 * TODO: Do everything again? */
	mov r0, #0
	//mcr p15, 0, r0, c7, c10, 4 	@ drain write buffer
	mcr p15, 0, r0, c8, c7, 0  	@ flush I, D, TLB

	mcr p15, 0, r0, c1, c0, 0	@ read control register
	orr r0, r0 , #1				@ enable MMU bit
	mov r1, #-1					@ sets manager to all domains
	mcr p15, 0, r1, c3, c0, 0	@ load domain access control
	mcr p15, 0, r4, c2, c0, 0	@ load page table pointer

	mcr p15, 0, r0, c1, c0, 0	@ write control reg

    /* Adjust stack and jump to our virtual address.
	 * TODO: This part should probably be identical... */
    ldr r1, =HAL_OFFSET
    sub sp, sp, r1
    sub pc, pc, r1

    nop     @ For PC jump above
    nop     @ For PC jump above

	//ldr r4, =__hyper_pt_start__
	//ldr r1, =HAL_OFFSET
	//add r4, r4, r1
	//mcr p15, 0, r4, c2, c0, 0	@ load page table pointer

	sub pc, lr, r1				@ correct lr to virt space

arm_reset_initial_pt:
	ldr r4, =__hyper_pt_start__
	ldr r1, =HAL_OFFSET
	add r4, r4, r1
	
	mcr p15, 0, r4, c2, c0, 0	@ load page table pointer
	bx lr

arm_setup_initial_pt:
	/* Start with the simplest possible PT needed to get STH running
 	 * in virtual address space.
 	 */
	
	/* mov r0, = #(__hyper_pt_start__ + HAL_OFFSET) */
	ldr r0, =__hyper_pt_start__
	ldr r1, =HAL_OFFSET
	add r0, r0, r1
	ldr r4, r0

	mov r1, #0
	add r2, r0, #0x4000

	/* First, clean the memory used to make all pages invalid. */
1:	str r1, [r0], #4
	str r1, [r0], #4
	str r1, [r0], #4
	str r1, [r0], #4
	teq r0, r2
	bne 1b

 	/* Do a 1->1 temporary mapping for our current position. */
	ldr r6, =(HAL_PHYS_START >> 20)
	ldr r7, =(HAL_PHYS_START | 0x40E)	@ CB on, Section, XN=f, AP =01 PRIV RW USER NO ACCESS
	str r7, [r4, r6, lsl #2]			@ 1:1 mapping for current position

	/* Then, map us from HAL_VIR_START to HAL_PHY_START in L1. */
	add r0, r4, #(HAL_VIRT_START & 0xff000000) >> 18
	str r7, [r0,#(HAL_VIRT_START & 0x00f00000) >> 18]! @ Map phys to HAL_VIRT_START

	/* Enable MMU */
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 4 	@ drain write buffer
	mcr p15, 0, r0, c8, c7, 0  	@ flush I, D, TLB

	mcr p15, 0, r0, c1, c0, 0	@ read control register
	orr r0, r0 , #1				@ enable MMU bit
	mov r1, #-1					@ sets manager to all domains
	mcr p15, 0, r1, c3, c0, 0	@ load domain access control
	mcr p15, 0, r4, c2, c0, 0	@ load page table pointer

	mcr p15, 0, r0, c1, c0, 0	@ write control reg

    /* Adjust stack and jump to our virtual address. */
    ldr r1, =HAL_OFFSET
    sub sp, sp, r1
    sub pc, pc, r1

    nop     @ For PC jump above
    nop     @ For PC jump abovez

	/* Now running in virt space, we can remove 1:1 mapping now - or NOT.
	 * TODO: Current version should not remove the temporary mapping.
	 * 		 We should instead copy it to 
	mov r0, #0
	ldr r3, =(HAL_PHYS_START >> 20)
	str r0, [r4, r3, lsl #2] */

	sub pc, lr, r1				@ correct lr to virt space
