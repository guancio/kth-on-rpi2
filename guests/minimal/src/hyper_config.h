#ifndef _HYPER_CONFIG_H_
#define _HYPER_CONFIG_H_

#define HC_NGUESTMODES 4
#define HC_NRPCHANDLERS 0
#define HC_NMEMREGIONS 5   // change to 5 in normal case
#define HC_NTRANSITIONAREAS 1
#define HC_NDOMAINPOOLS 0

#define HC_GM_TRUSTED   0
#define HC_GM_KERNEL    1
#define HC_GM_INTERRUPT 2
#define HC_GM_TASK      3

#define HC_RPCHANDLER_KERNEL 0
#define HC_RPCHANDLER_TRUSTED 1

#define HC_TRANSITION_KERNEL 0

#endif
