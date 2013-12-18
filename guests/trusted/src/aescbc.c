/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 20/12/2007
*/

//  An example of the use of AES (Rijndael) for file encryption.  This code
//  implements AES in CBC mode with ciphertext stealing when the file length
//  is greater than one block (16 bytes).  This code is an example of how to
//  use AES and is not intended for real use since it does not provide any
//  file integrity checking.
//
//  The Command line is:
//
//      aesxam input_file_name output_file_name [D|E] hexadecimalkey
//
//  where E gives encryption and D decryption of the input file into the
//  output file using the given hexadecimal key string.  The later is a
//  hexadecimal sequence of 32, 48 or 64 digits.  Examples to encrypt or
//  decrypt aes.c into aes.enc are:
//
//      aesxam file.c file.enc E 0123456789abcdeffedcba9876543210
//
//      aesxam file.enc file2.c D 0123456789abcdeffedcba9876543210
//
//  which should return a file 'file2.c' identical to 'file.c'
//
//  CIPHERTEXT STEALING
//
//  Ciphertext stealing modifies the encryption of the last two CBC
//  blocks. It can be applied invariably to the last two plaintext
//  blocks or only applied when the last block is a partial one. In
//  this code it is only applied if there is a partial block.  For
//  a plaintext consisting of N blocks, with the last block possibly
//  a partial one, ciphertext stealing works as shown below (note the
//  reversal of the last two ciphertext blocks).  During decryption
//  the part of the C:N-1 block that is not transmitted (X) can be
//  obtained from the decryption of the penultimate ciphertext block
//  since the bytes in X are xored with the zero padding appended to
//  the last plaintext block.
//
//  This is a picture of the processing of the last
//  plaintext blocks during encryption:
//
//    +---------+   +---------+   +---------+   +-------+-+
//    |  P:N-4  |   |  P:N-3  |   |  P:N-2  |   | P:N-1 |0|
//    +---------+   +---------+   +---------+   +-------+-+
//         |             |             |             |
//         v             v             v             v
//  +----->x      +----->x      +----->x      +----->x   x = xor
//  |      |      |      |      |      |      |      |
//  |      v      |      v      |      v      |      v
//  |    +---+    |    +---+    |    +---+    |    +---+
//  |    | E |    |    | E |    |    | E |    |    | E |
//  |    +---+    |    +---+    |    +---+    |    +---+
//  |      |      |      |      |      |      |      |
//  |      |      |      |      |      v      |  +---+
//  |      |      |      |      | +-------+-+ |  |
//  |      |      |      |      | | C:N-1 |X| |  |
//  |      |      |      |      | +-------+-+ ^  |
//  |      |      |      |      |     ||      |  |
//  |      |      |      |      |     |+------+  |
//  |      |      |      |      |     +----------|--+
//  |      |      |      |      |                |  |
//  |      |      |      |      |      +---------+  |
//  |      |      |      |      |      |            |
//  |      v      |      v      |      v            v
//  | +---------+ | +---------+ | +---------+   +-------+
// -+ |  C:N-4  |-+ |  C:N-3  |-+ |  C:N-2  |   | C:N-1 |
//    +---------+   +---------+   +---------+   +-------+
//
//  And this is a picture of the processing of the last
//  ciphertext blocks during decryption:
//
//    +---------+   +---------+   +---------+   +-------+
// -+ |  C:N-4  |-+ |  C:N-3  |-+ |  C:N-2  |   | C:N-1 |
//  | +---------+ | +---------+ | +---------+   +-------+
//  |      |      |      |      |      |            |
//  |      v      |      v      |      v   +--------|----+
//  |    +---+    |    +---+    |    +---+ |  +--<--+    |
//  |    | D |    |    | D |    |    | D | |  |     |    |
//  |    +---+    |    +---+    |    +---+ |  |     v    v
//  |      |      |      |      |      |   ^  | +-------+-+
//  |      v      |      v      |      v   |  | | C:N-1 |X|
//  +----->x      +----->x      | +-------+-+ | +-------+-+
//         |             |      | |       |X| |      |
//         |             |      | +-------+-+ |      v
//         |             |      |     |       |    +---+
//         |             |      |     |       v    | D |
//         |             |      |     +------>x    +---+
//         |             |      |             |      |
//         |             |      +----->x<-----|------+   x = xor
//         |             |             |      +-----+
//         |             |             |            |
//         v             v             v            v
//    +---------+   +---------+   +---------+   +-------+
//    |  P:N-4  |   |  P:N-3  |   |  P:N-2  |   | P:N-1 |
//    +---------+   +---------+   +---------+   +-------+
/**Changed it to work with memory operations instead of reading from FILE -Viktor Do*/
//#include "stdio.h"
//#include <ctype.h>
//#include <string.h>
#include "trusted_service.h"
#include "mwc.h"
#include "sha2.h"

#include "aes.h"


#define BLOCK_LEN   16

#define OK           0
#define READ_ERROR  -7
#define WRITE_ERROR -8


void pr_hex(const unsigned char *bytes, int nbytes,int compact)
/* Print bytes in hex format + newline */
{
   int i;
   int k = 0;
   if(compact){
	   for (i = 0; i < nbytes; i++){
		   printf("%2", bytes[i]);
	   }
   }
   else{
	   for (i = 0; i < nbytes; i++){
		   printf("%2 ", bytes[i]);
		   k++;
		   if (k >=16){
			   printf("\n");
			   k= 0;
		   }
	   }
   }
   printf("\n");
}

//  A Pseudo Random Number Generator (PRNG) used for the
//  Initialisation Vector. The PRNG is George Marsaglia's
//  Multiply-With-Carry (MWC) PRNG that concatenates two
//  16-bit MWC generators:
//      x(n)=36969 * x(n-1) + carry mod 2^16
//      y(n)=18000 * y(n-1) + carry mod 2^16
//  to produce a combined PRNG with a period of about 2^60.
//	overkill for IV?
void fillrand(unsigned char *buf, const int len)
{
//XXX	srand(0);
	srand_mwc(); // Automatically generates a seed, must be called once before calling rand_mwc()
	int i;
	for(i = 0; i < len;i++){
		buf[i] = (rand_mwc() %256);
	}
    printf("\nRandom generated IV for AES128-CBC:\n");
    pr_hex(buf,len,0);


}

void generateAESKey(char *AES){
//XXX	srand(0);
	int i ;
	int tmp = 0;
	for(i = 0; i < 32 ; i++){
		tmp = rand_mwc() % 16;
		if(tmp < 10 ){
			AES[i] = tmp + 48; //ascii table numbers
		}
		else{
			AES[i] = tmp + 55;
		}
	}

	pr_hex(AES,32,0);
//	AES[32] = '\0';
}

int encfile(aes_encrypt_ctx ctx[1],unsigned char *encrypted,unsigned char *message,unsigned int nbytes)
{
	unsigned char dbuf[3 * BLOCK_LEN];
    unsigned long i, len, wlen = BLOCK_LEN;

    int readBlockPointer = 0;
    int writeBlockPointer = 0;
    // When ciphertext stealing is used, we three ciphertext blocks so
    // we use a buffer that is three times the block length.  The buffer
    // pointers b1, b2 and b3 point to the buffer positions of three
    // ciphertext blocks, b3 being the most recent and b1 being the
    // oldest. We start with the IV in b1 and the block to be decrypted
    // in b2.

    // set a random IV

    fillrand(dbuf, BLOCK_LEN);


    // read the first file block
    if(BLOCK_LEN > nbytes)
    	len = nbytes;
    else
    	len = BLOCK_LEN;

    memcpy((char*)dbuf + BLOCK_LEN,message,len); //read
    readBlockPointer += BLOCK_LEN;

    if(len < BLOCK_LEN)
    {   // if the file length is less than one block

        // xor the file bytes with the IV bytes
        for(i = 0; i < len; ++i)
            dbuf[i + BLOCK_LEN] ^= dbuf[i];

        // encrypt the top 16 bytes of the buffer
        aes_encrypt(dbuf + len, dbuf + len, ctx);

        len += BLOCK_LEN;
        // write the IV and the encrypted file bytes

        memcpy(encrypted,dbuf,len);
        writeBlockPointer += BLOCK_LEN;
        return OK;
    }
    else    // if the file length is more 16 bytes
    {
    	unsigned char *b1 = dbuf, *b2 = b1 + BLOCK_LEN, *b3 = b2 + BLOCK_LEN, *bt;

        // write the IV

    	memcpy(encrypted,dbuf,BLOCK_LEN); //write
    	writeBlockPointer += BLOCK_LEN;

        for( ; ; )
        {
            // read the next block to see if ciphertext stealing is needed
      //      len = (unsigned long)fread((char*)b3, 1, BLOCK_LEN, fin);

            if(BLOCK_LEN > (nbytes-readBlockPointer))//strlen(dragons2 + readBlockPointer))
            	len = nbytes-readBlockPointer;
            else
            	len = BLOCK_LEN;

            memcpy(b3,message+readBlockPointer,len); //read
            readBlockPointer += BLOCK_LEN;

            // do CBC chaining prior to encryption for current block (in b2)
            for(i = 0; i < BLOCK_LEN; ++i)
                b1[i] ^= b2[i];

            // encrypt the block (now in b1)
            aes_encrypt(b1, b1, ctx);

            if(len != 0 && len != BLOCK_LEN)    // use ciphertext stealing
            {

            	// set the length of the last block
                wlen = len;

                // xor ciphertext into last block
                for(i = 0; i < len; ++i)
                    b3[i] ^= b1[i];

                // move 'stolen' ciphertext into last block
                for(i = len; i < BLOCK_LEN; ++i)
                    b3[i] = b1[i];

                // encrypt this block
                aes_encrypt(b3, b3, ctx);

                // and write it as the second to last encrypted block

                memcpy(encrypted + writeBlockPointer,b3,BLOCK_LEN); //write
                writeBlockPointer += BLOCK_LEN;
            }

            // write the encrypted block

            memcpy(encrypted + writeBlockPointer,b1,wlen);
            writeBlockPointer += BLOCK_LEN;

            if(len != BLOCK_LEN)
            	return OK;

            // advance the buffer pointers
            bt = b3, b3 = b2, b2 = b1, b1 = bt;
        }
    }
}

//TODO check if the memcopy is correct here, mistake before that i forgot that string one extra byte to hold null termination of the string
int decfile(aes_decrypt_ctx ctx[1],unsigned char *encrypted,char *decrypted,unsigned int nbytes)
{

	unsigned char dbuf[3 * BLOCK_LEN], buf[BLOCK_LEN];
    unsigned long i, len, wlen = BLOCK_LEN;
    int readBlockPointer = 0;
    int writeBlockPointer = 0;

    // When ciphertext stealing is used, we three ciphertext blocks so
    // we use a buffer that is three times the block length.  The buffer
    // pointers b1, b2 and b3 point to the buffer positions of three
    // ciphertext blocks, b3 being the most recent and b1 being the
    // oldest. We start with the IV in b1 and the block to be decrypted
    // in b2.


    if(BLOCK_LEN > nbytes)
    	len = nbytes + BLOCK_LEN;
    else
    	len = 2*BLOCK_LEN;

    memcpy((char*)dbuf, encrypted,len);
    readBlockPointer += BLOCK_LEN * 2;

    if(len < 2 * BLOCK_LEN) // the original file is less than one block in length
    {
    	len -= BLOCK_LEN;
        // decrypt from position len to position len + BLOCK_LEN
        aes_decrypt(dbuf + len, dbuf + len, ctx);

        // undo the CBC chaining
        for(i = 0; i < len; ++i)
            dbuf[i] ^= dbuf[i + BLOCK_LEN];

        // output the decrypted bytes

        memcpy(decrypted,dbuf,len);
        return OK;
    }
    else
    {   unsigned char *b1 = dbuf, *b2 = b1 + BLOCK_LEN, *b3 = b2 + BLOCK_LEN, *bt;

        for( ; ; )  // while some ciphertext remains, prepare to decrypt block b2
        {
            int nBytesLeft = nbytes-readBlockPointer - BLOCK_LEN  + (BLOCK_LEN *2);
            if(nBytesLeft < BLOCK_LEN)
            	len = nBytesLeft;
            else
           		len = BLOCK_LEN;

        	// read in the next block to see if ciphertext stealing is needed
        	memcpy(b3,encrypted + readBlockPointer,len);
        	readBlockPointer += BLOCK_LEN;

            // decrypt the b2 block
            aes_decrypt(b2, buf, ctx);


            if(len == 0 || len == BLOCK_LEN)    // no ciphertext stealing
            {
                // unchain CBC using the previous ciphertext block in b1
                for(i = 0; i < BLOCK_LEN; ++i)
                    buf[i] ^= b1[i];
            }
            else    // partial last block - use ciphertext stealing
            {
                wlen = len;

                // produce last 'len' bytes of plaintext by xoring with
                // the lowest 'len' bytes of next block b3 - C[N-1]
                for(i = 0; i < len; ++i)
                    buf[i] ^= b3[i];

                // reconstruct the C[N-1] block in b3 by adding in the
                // last (BLOCK_LEN - len) bytes of C[N-2] in b2
                for(i = len; i < BLOCK_LEN; ++i)
                    b3[i] = buf[i];

                // decrypt the C[N-1] block in b3
                aes_decrypt(b3, b3, ctx);

                // produce the last but one plaintext block by xoring with
                // the last but two ciphertext block
                for(i = 0; i < BLOCK_LEN; ++i)
                    b3[i] ^= b1[i];

                // write decrypted plaintext blocks

                memcpy(decrypted + writeBlockPointer,b3,BLOCK_LEN);
                writeBlockPointer += BLOCK_LEN;
            }

            // write the decrypted plaintext block

            memcpy(decrypted + writeBlockPointer,buf,wlen);
            writeBlockPointer += BLOCK_LEN;

            if(len != BLOCK_LEN)
                return OK;

            // advance the buffer pointers
            bt = b1, b1 = b2, b2 = b3, b3 = bt;
        }
    }
}

int aesEncrypt(char *sessionKey,char *data, char *encrypted){

	char ch, key[32];
	int		i, by = 0, key_len, err = 0;

    /**The input to an encryption process must be binary data so we must convert the
     * string into bytes. After decryption we convert it back into a string*/

	static unsigned char *message;
    unsigned int nbytes = strlen(data);	//is this needed?

    message = (unsigned char*)data;
    message = malloc(nbytes);
    memcpy(message, (unsigned char*)data, nbytes);

    aes_init();

    i = 0;          // this is a count for the input digits processed
//    printf("\nOriginal message in bytes\n");
//    pr_hex(message,nbytes,0);

    while(i < 64 && *sessionKey)        // the maximum key length is 32 bytes and
    {                           // hence at most 64 hexadecimal digits
        ch = toupper(*sessionKey++);    // process a hexadecimal digit
    	if(ch >= '0' && ch <= '9')
            by = (by << 4) + ch - '0';
        else if(ch >= 'A' && ch <= 'F')
            by = (by << 4) + ch - 'A' + 10;
        else                    // error if not hexadecimal
        {
        	printf("key must be in hexadecimal notation\n");
            err = -1;
            return err;
        }

        // store a key byte for each pair of hexadecimal digits
        if(i++ & 1)
            key[i / 2 - 1] = by & 0xff;
    }

    if(*sessionKey)
    {
    	printf("The key value is too long\n");
        err = -2;
        return err;
    }
    else if(i < 32 || (i & 15))
    {
    	printf("The key length must be 32, 48 or 64 hexadecimal digits\n");
        err = -3;
        return err;
    }

    key_len = i / 2;
    aes_encrypt_ctx ctx[1];

    aes_encrypt_key((unsigned char*)key, key_len, ctx);

    err = encfile(ctx, encrypted,message,nbytes);

    printf("\nHere is encrypted message in bytes:\n");
    pr_hex(encrypted,nbytes, 0);
    free(message);
    return err;
}

//TODO compute nbytes urself
int aesDecrypt(unsigned char *sessionKey,unsigned char *data, char *decrypted,int nbytes){

	char ch, key[32];
	int		i, by = 0, key_len, err = 0;

    /**The input to an encryption process must be binary data so we must convert the
     * string into bytes. After decryption we convert it back into a string*/

    aes_init();

    i = 0;          // this is a count for the input digits processed
    while(i < 64 && *sessionKey)        // the maximum key length is 32 bytes and
    {                           // hence at most 64 hexadecimal digits
        ch = toupper(*sessionKey++);    // process a hexadecimal digit
        if(ch >= '0' && ch <= '9')
            by = (by << 4) + ch - '0';
        else if(ch >= 'A' && ch <= 'F')
            by = (by << 4) + ch - 'A' + 10;
        else                    // error if not hexadecimal
        {
            printf("key must be in hexadecimal notation\n");
            err = -2;
            return err;
        }

        // store a key byte for each pair of hexadecimal digits
        if(i++ & 1)
            key[i / 2 - 1] = by & 0xff;
    }

    if(*sessionKey)
    {
        printf("The key value is too long\n");
        err = -3;
        return err;
    }
    else if(i < 32 || (i & 15))
    {
        printf("The key length must be 32, 48 or 64 hexadecimal digits\n");
        err = -4;
        return err;
    }
    key_len = i / 2;

    aes_decrypt_ctx ctx[1];
    aes_decrypt_key((unsigned char*)key, key_len, ctx);

    err = decfile(ctx,data,decrypted,nbytes);

    char *string;

    string = malloc(nbytes + 1);
    memcpy(string, decrypted, nbytes);
    string[nbytes] = '\0';

    printf("\nUsing AES key to decrypt message.\nHere is decrypted message: \n%s\n",string);
    printf("\n");
//    printf("\nDecrypted message in bytes: \n");
//    pr_hex(decrypted,nbytes,0);
    free(string);
    return err;
}
