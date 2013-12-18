/*
This is a heavily modified version of the PRNG described below.
The original was inefficient, unsafe, non-portable, and did not work as intended.
When built in Visual Studio 2008 on Windows XP, this is slightly faster than rand()
-Thesh
*/

/* Mother **************************************************************
|       George Marsaglia's The mother of all random number generators
|             producing uniformly distributed pseudo random 32 bit values
|             witht period about 2^250.
|
|       The arrays mother1 and mother2 store carry values in their
|             first element, and random 16 bit numbers in elements 1 to 8.
|             These random numbers are moved to elements 2 to 9 and a new
|             carry and number are generated and placed in elements 0 and 1.
|       The arrays mother1 and mother2 are filled with random 16 bit values
|             on first call of Mother by another generator.  mStart is the
|             switch.
|
|       Returns:
|       A 32 bit random number is obtained by combining the output of the
|             two generators and returned in *pSeed.  It is also scaled by
|             2^32-1 and returned as a double between 0 and 1
|
|       SEED:
|       The inital value of *pSeed may be any long value
|
|       Bob Wheeler 8/8/94
*/

#if defined(MWC_WIN_RAND_S)
#define _CRT_RAND_S
#include <stdlib.h>
#elif defined(MWC_DEV_URANDOM)
#include <stdio.h>
#endif

#include <time.h>
//#include <string.h>
//#include <stdlib.h>
#include "mwc.h"
//#include "hypertypes.h"

static uint16_t mother1[18];
static uint16_t mother2[18];
static uint16_t *m1=NULL, *m2=NULL;
static uint16_t mStart=1;

#define shift16 16
#define m16Mask 0xFFFF            // mask for lower 16 bits
#define MWC_WIN_RAND_S 1

void srand_mwc2(uint32_t seed) {
   uint32_t number;
   uint16_t sNumber;
   uint16_t n, *p;
   sNumber = seed & m16Mask;   // The low 16 bits
   number = seed;
   p=mother1+9;
   n=18;
   while (n--) {
      number = 30903 * sNumber + (number >> 16);
      *p++ = sNumber = number & m16Mask;
      if (n==9)
         p=mother2+9;
   }
   mStart=0;

   m1 = mother1+9;
   m2 = mother2+9;
}

void srand_mwc(int seed) {
	//XXXsrand_mwc2((uint32_t)time(NULL));
	srand_mwc2(seed); //For now until we can get a random number
}

uint32_t rand_mwc() {
   uint32_t number1, number2;

   if (mStart) return 0;

   /* Form the linear combinations */
   number1 = m1[0] +
      1941 * m1[1] + 1860  * m1[2] +
      1812 * m1[3] + 1776  * m1[4] +
      1492 * m1[5] + 1215  * m1[6] +
      1066 * m1[7] + 12013 * m1[8];

   number2 = m2[0] +
      1111 * m2[1] + 2222  * m2[2] +
      3333 * m2[3] + 4444  * m2[4] +
      5555 * m2[5] + 6666  * m2[6] +
      7777 * m2[7] + 9272  * m2[8];

   /* This has the same effect as moving elements 0-7 to 1-8, but avoids copying memory on every call */
   if (m1 == mother1) {
      memcpy(mother1+11,mother1+1,7*sizeof(uint16_t));
      memcpy(mother2+11,mother2+1,7*sizeof(uint16_t));
      m1 = mother1+9;
      m2 = mother2+9;
   }
   else {
      m1--;
      m2--;
   }

   //treat m1 and m2 as arrays of unsigned integers to prevent conversion issues
   /* Save the high bits of numberi as the new carry */
   m1[0]=number1 >> shift16;
   m2[0]=number2 >> shift16;

   /* Put the low bits of numberi into motheri[1] */
   m1[1]=m16Mask & number1;
   m2[1]=m16Mask & number2;

   /* Combine the two 16 bit random numbers into one 32 bit */

   return (((uint32_t)m1[1])<<16) | (uint32_t)m2[1];
}

