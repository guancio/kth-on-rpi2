/*
 * cpu_mem.h
 *
 *  Created on: May 30, 2012
 *      Author: Viktor Do
 */

#ifndef CPU_MEM_H_
#define CPU_MEM_H_

/* MMU */
#define HYPER_FL_PT_SIZE 4096
#define DOMAC	  0x55555555
#define FLD_DOM_ID(dom)  ((dom) << 5)
#define FLD_PT				0x11	 // Page table type
#define FLD_SEC				0x12	 // Section type
#define PT_BC  				0xC		 // bits 2-3 to denote bufferable and cacheable (0b1100)

#define SLD_NOACCESS   		0        // no access for anyone -- just zeros (unless S/R bits are used)
#define SLD_USERNOACCESS   	0x10     // 0b010101010000     priv read/write, user disallowed
#define SLD_USERREADONLY   	0x20     // 0b101010100000     priv read/write, user read
#define SLD_USERREADWRITE  	0x30     // 0b111111110000     priv read/write, user read/write

#define SLD_SP  			0x2		 // bits 0-1 to denote a small page (0b10)

/***********************************************/


#endif /* CPU_MEM_H_ */
