/*
 * rand mwc.h
 *
 *  Created on: Apr 8, 2011
 *      Author: vic
 */

#ifndef MWC_H_
#define MWC_H_

//#include <stdint.h>
#include "lib.h"
//rand function

void srand_mwc2(uint32_t);

uint32_t rand_mwc(); //will return 0 if one of the seed functions haven't been called

//seed functions
void srand_mwc(); //Automatically generates a seed





#endif /* MWC_H_ */
