/*
    mpprime.h

    by Michael J. Fromberger <http://www.dartmouth.edu/~sting/>
    Copyright (C) 1997 Michael J. Fromberger, All Rights Reserved

    Utilities for finding and working with prime and pseudo-prime
    integers

    $Id: mpprime.h,v 1.1 2004/02/08 04:29:29 sting Exp $
 */

#ifndef _H_MP_PRIME_
#define _H_MP_PRIME_

#include "mpi.h"

extern int prime_tab_size;   /* number of primes available */

/* Tests for divisibility    */
mp_err  mpp_divis(mp_int *a, mp_int *b);
mp_err  mpp_divis_d(mp_int *a, mp_digit d);

/* Random selection          */
mp_err  mpp_random(mp_int *a);
mp_err  mpp_random_size(mp_int *a, mp_size prec);

/* Pseudo-primality testing  */
mp_err  mpp_divis_vector(mp_int *a, mp_digit *vec, int size, int *which);
mp_err  mpp_divis_primes(mp_int *a, mp_digit *np);
mp_err  mpp_fermat(mp_int *a, mp_digit w);
mp_err  mpp_pprime(mp_int *a, int nt);

#endif /* end _H_MP_PRIME_ */
