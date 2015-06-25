/*
 * linux_signal.h
 *
 *  Created on: May 8, 2013
 *      Author: viktordo
 */

/*Contains Linux ARM specific signal code*/

#ifndef LINUX_SIGNAL_H_
#define LINUX_SIGNAL_H_

#define CONFIG_VECTORS_BASE 0xFFFF0000

#define __NR_OABI_SYSCALL_BASE	0x900000

#define __NR_SYSCALL_BASE	__NR_OABI_SYSCALL_BASE

#define __NR_restart_syscall		(__NR_SYSCALL_BASE+  0)
#define __NR_sigreturn			(__NR_SYSCALL_BASE+119)
#define __NR_rt_sigreturn		(__NR_SYSCALL_BASE+173)

#define KERN_SIGRETURN_CODE	(CONFIG_VECTORS_BASE + 0x00000500)
#define KERN_RESTART_CODE	(KERN_SIGRETURN_CODE + sizeof(sigreturn_codes))

/*b
 * For ARM syscalls, we encode the syscall number into the instruction.
 */
#define SWI_SYS_SIGRETURN	(0xef000000|(__NR_sigreturn)|(__NR_OABI_SYSCALL_BASE))
#define SWI_SYS_RT_SIGRETURN	(0xef000000|(__NR_rt_sigreturn)|(__NR_OABI_SYSCALL_BASE))
#define SWI_SYS_RESTART		(0xef000000|__NR_restart_syscall|__NR_OABI_SYSCALL_BASE)

/*
 * With EABI, the syscall number has to be loaded into r7.
 */
#define MOV_R7_NR_SIGRETURN	(0xe3a07000 | (__NR_sigreturn - __NR_SYSCALL_BASE))
#define MOV_R7_NR_RT_SIGRETURN	(0xe3a07000 | (__NR_rt_sigreturn - __NR_SYSCALL_BASE))

/*
 * For Thumb syscalls, we pass the syscall number via r7.  We therefore
 * need two 16-bit instructions.
 */
#define SWI_THUMB_SIGRETURN	(0xdf00 << 16 | 0x2700 | (__NR_sigreturn - __NR_SYSCALL_BASE))
#define SWI_THUMB_RT_SIGRETURN	(0xdf00 << 16 | 0x2700 | (__NR_rt_sigreturn - __NR_SYSCALL_BASE))


#endif /* LINUX_SIGNAL_H_ */
