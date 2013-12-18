#ifndef _RSA_H_
#define _RSA_H_

typedef unsigned int huge_t;

/* Structure for RSA public keys. */
int rsaEncrypt(char *g_message,char *g_e,char *g_modulus,unsigned char *encrypted);
int rsaDecrypt(char *g_message,char *g_d,char *g_modulus,unsigned char *decrypted);

#endif
