/*
  mprsa.h

  Simple implementation of RSA encryption and signing based on the
  PKCS #1, v. 1.5 encoding standard.  Uses the MPI library for the
  mathematical computations.

  by Michael J. Fromberger <http://www.dartmouth.edu/~sting/>
  Copyright (C) 2001 Michael J. Fromberger, All Rights Reserved

  $Id: mprsa.h,v 1.1 2004/02/08 04:29:29 sting Exp $
 */

#ifndef MPRSA_H_
#define MPRSA_H_

#include "mpi.h"

/* Function to fill a buffer with nonzero random bytes */
typedef void (*rnd_f)(char *, int);

/* Convert integer to octet string, per PKCS#1 v.1.5 */
mp_err   mp_i2osp(mp_int *x, char *out, int len);

/* Convert octed string to integer, per PKCS#1 v.1.5 */
mp_err   mp_os2ip(mp_int *x, char *in, int len);

/* Basic RSA encryption operation 
   msg       - integer representative of message to be encrypted, < modulus
   e         - encryption exponent
   modulus   - encryption key modulus
   cipher    - OUTPUT: encrypted result (may be same as msg)
 */
mp_err   mp_rsaep(mp_int *msg, mp_int *e, mp_int *modulus, mp_int *cipher);

/* Basic RSA decryption operation
   cipher    - integer representative of message to be decrypted, < modulus
   d         - decryption exponent
   modulus   - decryption key modulus
   msg       - OUTPUT: decrypted result (may be same as cipher) 
 */
mp_err   mp_rsadp(mp_int *cipher, mp_int *d, mp_int *modulus, mp_int *msg);

/* Basic RSA signing operation */
mp_err   mp_rsasp(mp_int *msg, mp_int *d, mp_int *modulus, mp_int *sig);

/* Basic RSA verification operation */
mp_err   mp_rsavp(mp_int *sig, mp_int *e, mp_int *modulus, mp_int *msg);

/* PKCS#1 v.1.5 message padding and encoding
   msg       - input message
   mlen      - length of input message, in bytes
   emsg      - output buffer
   emlen     - length of output buffer
   rand      - function to generate random nonzero bytes for padding

   Returns MP_OKAY if encoding is successful, MP_RANGE if message
   length is too long for encoding.
 */
mp_err   mp_pkcs1v15_encode(char *msg, int mlen, 
			    char *emsg, int emlen, rnd_f rand);

/* PKCS#1 v1.5 padded message decoding
   emsg      - encoded message
   emlen     - length of encoded message, in bytes
   msg       - output buffer, may be same as input buffer
   mlen      - OUTPUT: length of message stored into output buffer

   Returns MP_OKAY if decoding is successful, MP_RANGE if the encoded
   message is less than the minimum required length, MP_UNDEF if the 
   encoded message has an invalid format.
 */
mp_err   mp_pkcs1v15_decode(char *emsg, int emlen, char *msg, int *mlen);

/* Encrypt a message using RSA and PKCS#1 v.1.5 padding
   msg       - input message
   mlen      - length of input message, in bytes
   e         - encryption exponent
   modulus   - encryption key modulus
   *out      - OUTPUT: pointer to buffer holding encrypted output
   *olen     - OUTPUT: length of buffer holding output, in bytes
   rand      - function to generate random nonzero bytes for padding
 */
mp_err   mp_pkcs1v15_encrypt(char *msg, int mlen,
			     mp_int *e, mp_int *modulus,
			     char *out, int *olen,
			     rnd_f rand);

/* Decrypt a message using RSA and PKCS#1 v.1.5 padding
   msg       - input message (ciphertext)
   mlen      - length of input message, in bytes
   d         - decryption exponent
   modulus   - decryption key modulus
   *out      - OUTPUT: pointer to buffer holding decrypted output
   *olen     - OUTPUT: length of buffer holding output, in bytes
 */
mp_err   mp_pkcs1v15_decrypt(char *msg, int mlen,
			     mp_int *d, mp_int *modulus,
			     char *out, int *olen);

/* Return the maximum length a message can be using the given modulus
   under the PKCS#1 v.1.5 encoding scheme
 */
int      mp_pkcs1v15_maxlen(mp_int *modulus);

mp_err 	mp_pkcs1v15_sign();
mp_err 	mp_pkcs1v15_verify();

#endif /* MPRSA_H_ */

