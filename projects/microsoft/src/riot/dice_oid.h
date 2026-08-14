// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef DICE_OID_H_
#define DICE_OID_H_

#include <stddef.h>
#include <stdint.h>


/* List of available OIDs for Device ID CSRs. */

/* 1.3.6.1.4.1.311.102.1.10.1 */
extern const uint8_t DICE_OID_CERBERUS[];
#define	DICE_OID_CERBERUS_LENGTH	11

/* 1.3.6.1.4.1.311.102.1.11.1 */
extern const uint8_t DICE_OID_OVERLAKE[];
#define	DICE_OID_OVERLAKE_LENGTH	11

/* 1.3.6.1.4.1.311.102.1.13.1 */
extern const uint8_t DICE_OID_OMC[];
#define	DICE_OID_OMC_LENGTH			11

/* 1.3.6.1.4.1.311.102.1.20.1 */
extern const uint8_t DICE_OID_CORSICA[];
#define	DICE_OID_CORSICA_LENGTH		11

/* 1.3.6.1.4.1.311.102.1.50.1 */
extern const uint8_t DICE_OID_MANTICORE[];
#define	DICE_OID_MANTICORE_LENGTH	11

/* 1.3.6.1.4.1.311.102.1.1.1 */
extern const uint8_t DICE_OID_ATHENA[];
#define	DICE_OID_ATHENA_LENGTH		11

/* 1.3.6.1.4.1.311.102.1.2.1 */
extern const uint8_t DICE_OID_PIONEER[];
#define	DICE_OID_PIONEER_LENGTH		11

/* 1.3.6.1.4.1.311.102.1.4.1 */
extern const uint8_t DICE_OID_KINGSGATE[];
#define	DICE_OID_KINGSGATE_LENGTH	11

/* 1.3.6.1.4.1.311.102.1.5.1 */
extern const uint8_t DICE_OID_BRAGA[];
#define	DICE_OID_BRAGA_LENGTH		11

/* 1.3.6.1.4.1.311.102.1.6.1 */
extern const uint8_t DICE_OID_BRAGA_R[];
#define	DICE_OID_BRAGA_R_LENGTH		11

/* 1.3.6.1.4.1.311.102.1.12.1 */
extern const uint8_t DICE_OID_TAHOE[];
#define	DICE_OID_TAHOE_LENGTH		11


#endif	/* DICE_OID_H_ */
