
#ifndef _LIB_H_
#define _LIB_H_

#include <uclib.h>
//#include <memlib.h>

struct guest_data {
    uint32_t adr_pa;
    uint32_t adr_va;
};

extern struct guest_data guest_data;

#endif /* _LIB_H_ */
