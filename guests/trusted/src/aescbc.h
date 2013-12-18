/*
 * aescbc.h
 *
 *  Created on: 13 apr 2011
 *      Author: Master
 */

#ifndef AESCBC_H_
#define AESCBC_H_
#include "trusted_service.h"

void pr_hex();

int aesEncrypt(char *sessionKey,char *data, char *encrypted);
int aesDecrypt(unsigned char *sessionKey,unsigned char *data, char *decrypted,int nbytes);
void generateAESKey(char *AES);

#endif /* AESCBC_H_ */
