
#ifndef _UTIL_MACROS_H_
#define _UTIL_MACROS_H_


/* 
 * bit field helper macors. 
 * these two require definition of xxx_SHIFT and xxx_MASK
 */
#define BF_EXTRACT(type, val) ( ((val) >> (type##_SHIFT)) & (type##_MASK))

#define BF_INSERT(type, val, newval) (((val) & ~((type##_MASK) << (type##_SHIFT))) \
                                      | ((newval) << (type##_SHIFT)))


/* align macros */
#define ALIGN_UP(adr,size) (((adr) + (size) -1) &  ~ ((size) -1))
#define ALIGN_DOWN(adr,size) ((adr)  &  ~ ((size) -1))



#endif /* _UTIL_MACROS_ */
