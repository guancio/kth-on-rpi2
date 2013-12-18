#ifndef _HW_H_
#define _HW_H_

#include <types.h>

#include <cpu.h>
#include <board.h>
#include <soc.h>
#include <hw_hal.h>


/* panic */
#define panic(msg) panic_(msg, __FILE__, __LINE__)
extern void panic_(char *, char *, int);
extern void printf();

#endif /*_HW_H_*/
