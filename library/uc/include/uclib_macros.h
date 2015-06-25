#ifndef _UC_MACROS_H_
#define _UC_MACROS_H_


/* define PRIVATE */
#ifdef COMPILE_HOST
 #define PRIVATE /* nothing. we want to access private functions for testing */
#else
 #define PRIVATE static
#endif


#endif /* _UC_MACROS_H_ */
