
/*
 * unoptimized libc functions
 * 
 * NOTE: these functions are all untested!
 * 
 */

#include <uclib.h>

void *memcpy(void *dst, void const *src, int len)
{
    char * pDst = (char *) dst;
    char const * pSrc = (char const *) src;
    while (len--)
    {
        *pDst++ = *pSrc++;
    }
    return (dst);    
}

void * memset ( void * ptr, int value, int num )
{
    char * tmp = (char *) ptr;
    while(num--)
        *tmp++ = value;
    return ptr;
}

int memcmp(const void *ptr1, const void *ptr2, int n)
{
    const char * tmp1 = (const char *) ptr1;
    const char * tmp2 = (const char *) ptr2;
    
    while(n--) {
        int c = (int)*tmp1++ - (int)*tmp2++;
        if(c) return c;
    }
    return 0;
}

/* -------------------------------------------- */

int strlen(const char *str)
{
    int ret = 0;
    while(*str++ != '\0') ret++;
    return ret;
}

char *strcpy(char *dst_, const char *src)
{
    char *dst = dst_;
    do {
        *dst++ = *src;
    } while(*src++ != '\0');
    
    return dst_;
}


char *strncpy(char *dst_, const char *src, int n)
{
    char *dst = dst_;
    do {
        *dst++ = *src;
    } while(*src++ != '\0' && --n);
    
    while(--n)  {
        *dst++ = '\0';
    }
    
    return dst_;
}

int strcmp(const char *s1, const char *s2)
{    
    while(*s2 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}


int strncmp(const char *s1, const char *s2, size_t n)
{
    for ( ; n > 0; s1++, s2++, --n)
	if (*s1 != *s2)
	    return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1);
	else if (*s1 == '\0')
	    return 0;
    return 0;
}

