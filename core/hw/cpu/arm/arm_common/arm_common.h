/*
 * arm_common.h
 *
 *  Created on: May 31, 2012
 *      Author: Viktor Do
 */

#ifndef _ARM_COMMON_H_
#define _ARM_COMMON_H_


/* First some sanity checks */

#ifndef HAL_PHYS_START
 #error "HAL_PHYS_START was not defined"
#endif

#ifndef HAL_VIRT_START
 #error "HAL_VIRT_START was not defined"
#endif

#define HAL_OFFSET ((HAL_PHYS_START)-(HAL_VIRT_START))


#define GET_VIRT(phy)  (((unsigned char *)(phy)) - HAL_OFFSET)
#define GET_PHYS(vir)  (((unsigned char *)(vir)) + HAL_OFFSET)


/* ARM CPU MODES */
#define ARM_MODE_MASK       0x1f
#define ARM_MODE_USER       0x10
#define ARM_MODE_FIQ        0x11
#define ARM_MODE_IRQ        0x12
#define ARM_MODE_SUPERVISOR 0x13
#define ARM_MODE_ABORT      0x17
#define ARM_MODE_UNDEFINED  0x1b
#define ARM_MODE_SYSTEM     0x1f

#define ARM_IRQ_MASK 0x80
#define ARM_FIQ_MASK 0x40
#define ARM_INTERRUPT_MASK ( ARM_IRQ_MASK | ARM_FIQ_MASK)

/* COP */
#define COP_SYSTEM "p15"

#define COP_WRITE(cop, func, reg) COP_WRITE2(cop, "0", func, reg)
/*        asm volatile("mcr " cop ", 0 , %0," func :: "r" (reg)) */

#define COP_READ(cop, func, reg) COP_READ2(cop, "0", func, reg)
/*	asm volatile("mrc " cop ", 0 , %0, " func : "=r" (reg)) */


/* these also set op1 which is otherwise 0 */
#define COP_WRITE2(cop, op1, func, reg) \
   asm volatile("mcr " cop "," op1 ", %0," func :: "r" (reg))

#define COP_READ2(cop, op1, func, reg) \
   asm volatile("mrc " cop "," op1 ", %0, " func : "=r" (reg))

#if ARM_ARCH >= 7
	#define isb() __asm__ __volatile__ ("isb" : : : "memory")
	#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
	#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")

#else
	#define dmb() /* empty */
	#define isb() /* empty */ 
	#define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                                  : : "r" (0) : "memory")

#endif


#ifndef __ASSEMBLER__

/* interrupt */
typedef enum {
    IRQ_MASK = 0x80,
    FIQ_MASK = 0x40,

    INTERRUPT_MASK = (IRQ_MASK | FIQ_MASK)
} interrupt_state;

typedef enum {
    V_IMPL_RESET = 0,
    V_UNDEF,
    V_SWI,
    V_PREFETCH_ABORT,
    V_DATA_ABORT,
    V_NONE,
    V_IRQ,
    V_FIQ,
    V_ARM_SYSCALL,
    V_RET_FAST_SYSCALL,
    V_RET_FROM_EXCEPTION,
} interrupt_vector;

/* CPU context */
typedef struct context_
{
    uint32_t reg[13]; //The 13 general-purpose registers of the ARM processor core.
    uint32_t sp; //The stack pointer.
    uint32_t lr; //The link register.
    uint32_t pc; //The program counter.
    uint32_t psr; //The program status register.
} context;

#endif /* ! __ASSEMBLER__ */


#endif /* _ARM_COMMON_H_ */
