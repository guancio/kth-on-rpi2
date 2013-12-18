#ifndef _HYPER_CONFIG_H_
#define _HYPER_CONFIG_H_

#define HC_NGUESTMODES 4

#define HC_GM_TRUSTED   0
#define HC_GM_KERNEL    1
#define HC_GM_TASK      2
#define HC_GM_INTERRUPT 3

#define HC_DOM_DEFAULT 	0
#define HC_DOM_KERNEL 	1
#define HC_DOM_TASK 	2
#define HC_DOM_TRUSTED 	3

#define MAX_TRUSTED_SIZE	   0x1000000	//15MB

#endif
