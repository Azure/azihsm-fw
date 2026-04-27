// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CCS_KSU_KAT_VECTORS_H_
#define CCS_KSU_KAT_VECTORS_H_

#include <stdint.h>
#include "splibs/inc/spcryptotypes.h"


extern const SP_MSG_384 CCS_KSU_KAT_VECTORS_HMAC_SHA256_KEY;
extern const SP_MSG_384 CCS_KSU_KAT_VECTORS_HMAC_SHA384_KEY;

extern const SP_MSG_384 CCS_KSU_KAT_VECTORS_KDF256_CONTEXT;
extern const SP_MSG_384 CCS_KSU_KAT_VECTORS_KDF384_CONTEXT;
extern const uint8_t CCS_KSU_KAT_VECTORS_KDF256_HMAC[];
extern const uint8_t CCS_KSU_KAT_VECTORS_KDF384_HMAC[];


#endif	/* CCS_KSU_KAT_VECTORS_H_ */
