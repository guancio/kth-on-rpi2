#ifndef _UC_TYPES_H_
#define _UC_TYPES_H_

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef unsigned int addr_t; /* XXX: this wont work on 64-bit systems */
typedef addr_t size_t;


#define BOOL int
#define FALSE 0
#define TRUE 1

#ifndef NULL
 #define NULL 0
#endif

#define BASE_REG volatile uint32_t *



typedef enum return_value_ {
    RV_OK = 0,
    RV_BAD_ARG = -127,
    RV_BAD_PERM,
    RV_BAD_OTHER
} return_value;


/* data types */
typedef return_value (*cpu_callback)(uint32_t, uint32_t, uint32_t);


#endif /* _UC_TYPES_H_ */
