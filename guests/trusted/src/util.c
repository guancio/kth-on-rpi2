#include "lib.h"
#include <memlib.h>

uint32_t rand(void)
{
   static uint32_t z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
   uint32_t b;
   b  = ((z1 << 6) ^ z1) >> 13;
   z1 = ((z1 & 4294967294U) << 18) ^ b;
   b  = ((z2 << 2) ^ z2) >> 27;
   z2 = ((z2 & 4294967288U) << 2) ^ b;
   b  = ((z3 << 13) ^ z3) >> 21;
   z3 = ((z3 & 4294967280U) << 7) ^ b;
   b  = ((z4 << 3) ^ z4) >> 12;
   z4 = ((z4 & 4294967168U) << 13) ^ b;
   return (z1 ^ z2 ^ z3 ^ z4);
}



#define CHAR_BIT 8

/* This function will return absoulte value of n*/
uint32_t abs(int n)
{
  int const mask = n >> (sizeof(int) * CHAR_BIT - 1);
  return ((n + mask) ^ mask);
}

void __bad_assert(){
	printf("CRASH!\n");
}

/*
 * Author from
 * https://www.student.cs.uwaterloo.ca/~cs350/common/os161-src-html/memmove_8c-source.html*/
typedef unsigned long uintptr_t;

#include <types.h>
void *memmove(void *dst, const void *src, size_t len)
{
         size_t i;
         /*
          * If the buffers don't overlap, it doesn't matter what direction
          * we copy in. If they do, it does, so just assume they always do.
          * We don't concern ourselves with the possibility that the region
          * to copy might roll over across the top of memory, because it's
          * not going to happen.
          *
          * If the destination is above the source, we have to copy
          * back to front to avoid overwriting the data we want to
          * copy.
          *
          *      dest:       dddddddd
          *      src:    ssssssss   ^
          *              |   ^  |___|
          *              |___|
          *
          * If the destination is below the source, we have to copy
          * front to back.
          *
          *      dest:   dddddddd
          *      src:    ^   ssssssss
          *              |___|  ^   |
          *                     |___|
          */

         if ((uintptr_t)dst < (uintptr_t)src) {
                 /*
                  * As author/maintainer of libc, take advantage of the
                  * fact that we know memcpy copies forwards.
                  */
                 return memcpy(dst, src, len);
        }

         /*
          * Copy by words in the common case. Look in memcpy.c for more
          * information.
          */

         if ((uintptr_t)dst % sizeof(long) == 0 &&
             (uintptr_t)src % sizeof(long) == 0 &&
             len % sizeof(long) == 0) {

                 long *d = dst;
                 const long *s = src;

                 /*
                  * The reason we copy index i-1 and test i>0 is that
                  * i is unsigned - so testing i>=0 doesn't work.
                  */

                 for (i=len/sizeof(long); i>0; i--) {
                         d[i-1] = s[i-1];
                 }
         }
         else {
                 char *d = dst;
                 const char *s = src;

                 for (i=len; i>0; i--) {
                         d[i-1] = s[i-1];
                 }
         }

         return dst;
 }


/*Below functions only work for ASCII */
int toupper(char ch)
{
	if((ch >= 'a') && (ch <= 'z'))
            return ( 'A' + (ch - 'a'));
	return ch;
}

int tolower(char ch)
{
	if((ch >= 'A') && (ch <= 'Z'))
		return ('a' + (ch - 'A'));
	return ch;
}

int isupper(int ch)
{
    return (ch >= 'A' && ch <= 'Z');
}

int islower(int ch)
{
    return (ch >= 'a' && ch <= 'z');
}
int isdigit(char c)
{
    return (c >= '0' && c <= '9');
}
