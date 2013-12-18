/*
 * trusted_service.h
 *
 *  Created on: 30 mar 2011
 *      Author: Master
 */

#ifndef TRUSTED_SERVICE_H_
#define TRUSTED_SERVICE_H_

typedef struct hyperargs_{
	char *source;
	char *dest;
}hyperargs;

typedef struct trusted_args_contract_{
	char contract[50000];
	int *success;
}TrustedContractArgs;

typedef struct trusted_args_sign_{
//	char signature[128];
	char *contract;//[50000];
	char *modulus;
	int *success;
	char *signature; //TODO remove only for testing
}TrustedSignArgs;


#define SHAREDRPC_FUNCTION __attribute__((section("shared_rpc_functions")))
#define SHAREDRPC_DATA __attribute__((section("shared_rpc_data")))

#define FLASH_DATA

#endif /* TRUSTED_SERVICE_H_ */
