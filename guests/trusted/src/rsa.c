#include "rsa.h"
#include "lib.h"
#include "mpi.h"
#include "mprsa.h"


//#include <stdlib.h>
//#include <time.h>



void myrand(char *out, int len);

int rsaEncrypt(char *g_message,char *g_e,char *g_modulus,unsigned char *encrypted){
	mp_int modulus, e;
	int olen;
	mp_err res;

	//XXX srand(time(NULL));
	mp_init(&modulus);
	mp_init(&e);

	mp_read_radix(&modulus, g_modulus,16);
	mp_read_radix(&e, g_e,16);

	printf("Encrypting message, %d bytes:\n\t\"%s\"\n",
			strlen(g_message),
			g_message);

	res = mp_pkcs1v15_encrypt(g_message, strlen(g_message),
			&e, &modulus, encrypted, &olen, myrand);
//	printf("Result:  %s\n", mp_strerror(res));

	if(res == MP_OKAY){
		printf("\n\n\n\t\tEncrypted message length: %d\n", olen);
		printf("Packet data:\n");
		pr_hex(encrypted,128,0);
	}
	else
		printf("RSA encryption failed\n");

	mp_clear(&e);
	mp_clear(&modulus);
	return res;
}

int rsaDecrypt(char *g_message,char *g_d,char *g_modulus,unsigned char *decrypted){
	mp_int modulus, d;
	int olen = (strlen(g_modulus) / 2); //length of modulus in bytes
	mp_err res;

	//XXX srand(time(NULL));
	mp_init(&modulus);
	mp_init(&d);

	mp_read_radix(&modulus, g_modulus,16);
	mp_read_radix(&d, g_d,16);

	printf("\nReceived encrypted message:\n");
	pr_hex(g_message,128,0);

	printf("\nDecrypting message ... \n");
		res = mp_pkcs1v15_decrypt(g_message, olen,
				&d, &modulus, decrypted, &olen);
//		printf("Result:  %s\n", mp_strerror(res));

		if(res != MP_OKAY) {
			goto CLEANUP;
		} else {
			int  ix;

			printf("Decrypted message length: %d\n", olen);
			printf("\nMessage data (AES key):\n");
			for(ix = 0; ix < olen; ix++) {
				//XXXfputc(decrypted[ix], stdout);
			}
			//XXXfputc('\n', stdout);
		}

	CLEANUP:
	mp_clear(&d);
	mp_clear(&modulus);
	return res;
}

void myrand(char *out, int len)
{
	int  ix, val;

	for(ix = 0; ix < len; ix++) {
		do {
			val = rand() % UCHAR_MAX;
		} while(val == 0);
		out[ix] = val;
	}
}

