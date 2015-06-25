
#ifndef _UC_STDLIB_H_
#define _UC_STDLIB_H_

/*
 * minimal stdlib
 */

#include <uclib_types.h>


//#define DEBUG_OUTPUT

#ifdef DEBUG_OUTPUT
	#define debug(fmt, args...) printf("\tHV: "fmt, ##args)
#else
	#define debug while(0)
#endif


extern void *memcpy(void *dst_, const void *src, int n);
extern void *memset ( void * ptr, int value, int num );
extern int memcmp(const void *ptr1, const void *ptr2, int n);

extern int strlen(const char *str);
extern char *strcpy(char *dst_, const char *src);
extern char *strncpy(char *dst_, const char *src, int n);
extern int strcmp(const char *s1, const char *s2);

extern void hprintf(const char *fmt, ...);

/* these are not implemented in uclib, only prototyped for future reference */
extern void *malloc(size_t );
extern void free(void *);

#endif /* _UC_STDLIB_ */
