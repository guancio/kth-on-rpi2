/*
 * cpu.h
 *
 *  Created on: May 31, 2012
 *      Author: Viktor Do
 */

#ifndef CPU_H_
#define CPU_H_

#include "arm_common.h"
#include "cpu_cop.h"
#include "cpu_mem.h"

/**********************************************
 * CACHE
 **********************************************/

/* Operations */
enum cache_op { FLUSH = 0, CLEAN = 1};


#endif /* CPU_H_ */
