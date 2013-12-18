/*
 * trustedservice.c
 *
 *  Created on: 30 mar 2011
 *      Author: Master
 */
#include "trusted_service.h"
//#include "dmacRegisters.h"
#include "hypercalls.h"

#include "rsa.h"
#include "aescbc.h"
#include "aes/sha2.h"
#include "mpi/mprsa.h"
#include "mpi/mwc.h"

#include <memlib.h>

//TODO remove buffer and use only decrypted for immediate storage? Check string lengths in program.
//TODO Use memset to reset sensitive memory
FLASH_DATA static unsigned char encrypted[50000+16]; //data in flash memory is encrypted (16 is size of IV added to message)

static char decrypted[50000];
static unsigned char buffer[50000+16];  //buffer to hold the transfered data from flash
static char contract[50000];

static unsigned char encryptedAESKey[128]; //key is encrypted with RSA
static unsigned char decryptedAESKey[128]; //key is encrypted with RSA
static unsigned int  nbytes;

// 1024-bit RSA keys in HEX
static char *g_modulus =	"f0f4fa85b4ad3f6d9171995085b31640c4c8ed28e5c9eb5106f62acea46ee83c"
										"0c575f03ab918d9aee62fdd5fc7b5a350d4775618f646583ef6a0c50985123ac"
										"4271ae3cdaba97f5d6527217971f2cc0bdbbfa2886afedfe783e1b170ca5e279"
										"e6fd07e9efffd99c1f4f35c30644f86227cfc32a5253a89ebfb22862f1085b35";

static char *g_d =			"e2d4cc0e18834b859af8c4ea6fa2a29d406322177112bfaa9c921ac4433980f8"
										"1e6a15b0ffcf5aedf1e250b12428ff479803a035c26631c69d18491589fe4043"
										"f2cf8d591c93256dda125bb1466a2199fab20081b6806ddda740b73e35e73c63"
										"e794ba34197aa423064286feb2e019b4521a05405b27b2f232619a00bedeac95";

static char *g_e = "10001";



/////////////////////   MACROS   ///////////////////

#define LOGT(_FMT, ...)  printf( "TEST DMA - TASK: " _FMT,  ## __VA_ARGS__)
////////////////////////////////////////////////////

static void useDMA();
void trustedRPCHandler(unsigned callNum, void* params);
static void finishRPC();
//XXX put back static
void initFlashData();

static unsigned int count = 0;

static void getContract(TrustedContractArgs *args){
	int rsaErr = 0, aesErr = 0;
	//hello(REP_TRUSTED_NAME);
	printf("In trusted mode getContract()");

	//X1XXuseDMA(); //DMA the encrypted data into trusted space
	rsaErr = rsaDecrypt(encryptedAESKey,g_d,g_modulus,decryptedAESKey);
	if(rsaErr != 0){
		printf("RSA encryption failed.\n");
		//*args->success = 0;
	}
	else{
		aesErr = aesDecrypt(decryptedAESKey,encrypted,decrypted,nbytes);
	}

	if(aesErr != 0){
		printf("AES encryption failed.\n");
		//*args->success = 0;
	}
	else if(aesErr == 0 && rsaErr == 0){
		//*(args)->success = 1;
		memcpy(contract,decrypted,nbytes);
	}
	/*Clear decrypted aeskey*/
	memset(decryptedAESKey,0, 128);

}

static char data[50000] = "Until modern times cryptography referred almost exclusively to encryption, which is the process of converting ordinary information (called plaintext) into unintelligible gibberish (called ciphertext).[7] Decryption is the reverse, in other words, moving from the unintelligible ciphertext back to plaintext. A cipher (or cypher) is a pair of algorithms that create the encryption and the reversing decryption. The detailed operation of a cipher is controlled both by the algorithm and in each instance by a key. This is a secret (ideally known only to the communicants), usually a short string of characters, which is needed to decrypt the ciphertext. A cryptosystem is the ordered list of elements of finite possible plaintexts, finite possible cyphertexts, finite possible keys, and the encryption and decryption algorithms which correspond to each key. Keys are important, as ciphers without variable keys can be trivially broken with only the knowledge of the cipher used and are therefore useless (or even counter-productive) for most purposes. Historically, ciphers were often used directly for encryption or decryption without additional procedures such as authentication or integrity checks.\n";
void read(){
	printf("%s\n", contract);
}

void initFlashData(int *success){
	int aesErr = 0; //error handling
	int rsaErr = 0;

//	hello(REP_TRUSTED_NAME);
	printf("In trusted mode initFlashData\n");
//	TRUSTED_DATA static char data[50000] = "Messenger: Choose your next words carefully, Leonidas. They may be your last as king.\nKing Leonidas: [to himself: thinking] \"Earth and water\"?\n[Leonidas unsheathes and points his sword at the Messenger's throat]\nMessenger: Madman! You're a madman!\nKing Leonidas: Earth and water? You'll find plenty of both down there.\nMessenger: No man, Persian or Greek, no man threatens a messenger!\nKing Leonidas: You bring the crowns and heads of conquered kings to my city steps. You insult my queen. You threaten my people with slavery and death! Oh, I've chosen my words carefully, Persian. Perhaps you should have done the same!\nMessenger: This is blasphemy! This is madness!\nKing Leonidas: Madness...?\n[shouting]\nKing Leonidas: THIS IS SPARTA!\n[Kicks the messenger down the well]\n";

//	32 bytes HEX key digits used as key to AES-128
//  TRUSTED_DATA static char sessionKey[33] = "0123456789abcdeffedcba9876543210"; AES key
    char sessionKey[32];
    generateAESKey(sessionKey); //generates a random 16 byte AES key (AES-128)

    nbytes = strlen(data);	//setting the global variable, used in aesDecrypt

    printf("\nEncrypting message with random generated AES-128 one time session key:\n%s",sessionKey);
	aesErr = aesEncrypt(sessionKey,data,encrypted);
	memcpy(contract,encrypted,nbytes);
#if 1
	if(aesErr != 0){
		printf("AES encryption failed.\n");
//		*success = 0;
	}
	else{
		printf("\nTry to print encrypted data! \n %s \n\n", encrypted);
		printf("\n Encrypting AES session key with RSA\n");
		rsaErr = rsaEncrypt(sessionKey,g_e,g_modulus,encryptedAESKey);
	}
	if(rsaErr != 0){
		printf("RSA encryption failed.\n");
		//*success = 0;
	}
	else if(aesErr == 0 && rsaErr == 0){
	//	*success = 1;
	}
#endif

}

static void calculateSHA(char *message, unsigned char *hval){
	sha256_ctx		sha_ctx[1];

	sha256_begin(sha_ctx);
	sha256_hash(message, strlen(decrypted), sha_ctx);
	sha256_end(hval, sha_ctx);
}

static void verifyContract(TrustedSignArgs *args){
	//hello(REP_TRUSTED_NAME);
	printf("In trusted mode verifyContract()");

	char *ptx;
	int olen;

	unsigned char *g_e = "10001"; //public key is always 0x10001, will not effect security

	mp_int  modulus, e;
	mp_err err;
	mp_init(&modulus);
	mp_init(&e);

	mp_read_radix(&modulus, (args->modulus), 16);
	mp_read_radix(&e, g_e, 16);

	err = mp_pkcs1v15_verify(args->signature,128,&e,&modulus,&ptx,&olen);
	if(err != 0){
		printf("Verify contract failed\n");
	}
	else{
		printf("\nThis is the hash value we got from sender\n");
		pr_hex(ptx,SHA256_DIGEST_SIZE,1); //print hashvalue that we got from signature //crashes here sometimes

		args->contract[30] = 'g';//sabotage message
		printf("\nThis is our own calculated hash value!\n");
		unsigned char	hval[SHA256_DIGEST_SIZE];
		calculateSHA(args->contract,hval);	//input contact, output hvalue

		printf("\nSHA-256 TRUSTED DIGEST: \n");
		pr_hex(hval,SHA256_DIGEST_SIZE,1);

		if(!memcmp(hval,ptx,32)){
			printf("Message signature is valid!\n");
		}
		else
			printf("Message signature is wrong. Signature failed\n");
		printf("Size of message:%d\n",nbytes);

		mp_clear(&e);
		mp_clear(&modulus);

	}

}

// void trusted_rpc_handler(unsigned callNum, void *params)
 void handler_rpc(unsigned callNum, void *params)
{
	switch(callNum){
	case 0:
		/*Only initialize seed and heap once*/
		if (count == 0){
			printf("Initializing Trusted Service\n");
			_main((uint32_t)params);
			count++;
		}
		else
			printf("Already initialized trusted service");
		break;
	case 1:
		if(count !=0)
			initFlashData(params);
		else
			printf("Trusted service not initialized");
		break;
	case 2:
		//if(count !=0)
		//verifyContract(params);
		break;
	case 3:
		if(count !=0)
			getContract(params);
		else
			printf("Trusted service not initialized");
		break;
	case 4:
		read();
		break;
	default:
		printf("Unknown trusted operation: %d\n", callNum);
	}
	finish_rpc();
}

void finish_rpc()
{
	ISSUE_HYPERCALL(HYPERCALL_END_RPC);
}





void _main(int seed){
	printf("Seed value: %x", seed);

	srand_mwc(seed);
	/*Initialize the heap*/
	init_heap();
	//memcpy(contract,data,nbytes);

}


