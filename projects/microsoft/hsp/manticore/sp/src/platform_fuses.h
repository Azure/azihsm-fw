// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef PLATFORM_FUSES_H_
#define PLATFORM_FUSES_H_

#include <stdint.h>


/* HSP fuse defintion for Manticore. */

/* NOTE:  DO NOT PACK THESE STRUCTURES.
 *
 * They are naturally aligned to word boundaries and packing them causes the compiler to use byte
 * accesses for all fields.  This has two big drawbacks:
 *		1.  It generates more code.  Instead of a single load word instruction, the compiler
 *			generates 10 instructions (4 byte loads, 3 shift, 3 or).  For every field access.
 *		2.  The GFC registers these structures map to must be word accessed.  Using byte accesses on
 *			these registers results in 0s being read.  With the byte accesses used by packed
 *			structures, you will never get accurate data when cast to register addresses. */

/**
 * Single copy of the redundant data that is stored in SW0.
 */
struct hsp_fuses_sw0_data {
	uint32_t svn_1sp;	/**< Current SVN value for the key manifest and 1SP image. */
};

/**
 * Structure of the SW0 fuse slot.
 */
struct hsp_fuses_sw0 {
	uint64_t rng_calibration;			/**< Calibration value for the HW RNG. */
	struct hsp_fuses_sw0_data data[2];	/**< Redundant copies of the values stored in SW0. */
};

/**
 * Single copy of the redundant data that is stored in SW1.
 */
struct hsp_fuses_sw1_data {
	uint32_t dme_renewal;			/**< Renewal counter for the DME key. */
	uint32_t dice_renewal;			/**< Renewal counter for the DICE UDS. */
	uint64_t svn_fw_key_manifest;	/**< Current SVN value for the firmware key manifest. */
	uint64_t svn_fw_package;		/**< Current SVN value for the firmware package. */
};

/**
 * Structure of the SW1 fuse slot.
 */
struct hsp_fuses_sw1 {
	struct hsp_fuses_sw1_data data[2];	/**< Redundant copies of the values store in SW1. */
};

/**
 * The total number of bytes in the fuse block.
 */
#define	HSP_FUSES_TOTAL_LENGTH					1024


/* Base addresses and lengths of each HSP fuse slot defined.  The lengths represent the amount of
 * data stored in each slot, without ECC.  The amount of ECC overhead is a separate value.  If this
 * overhead is 0, that means there is no ECC for that slot.
 *
 * A length of 0 means the slot doesn't exist for this design. */

#define	HSP_FUSES_SS_ADDRESS					0x0000
#define	HSP_FUSES_SS_LENGTH						0x10
#define	HSP_FUSES_SS_ECC						0

#define	HSP_FUSES_EMC_ADDRESS					0x0010
#define	HSP_FUSES_EMC_LENGTH					0x3c
#define	HSP_FUSES_EMC_ECC						0

#define	HSP_FUSES_AEB_ADDRESS					0x004c
#define	HSP_FUSES_AEB_LENGTH					0x10
#define	HSP_FUSES_AEB_ECC						0

#define	HSP_FUSES_SYS_ADDRESS					0x0000
#define	HSP_FUSES_SYS_LENGTH					0
#define	HSP_FUSES_SYS_ECC						0

#define	HSP_FUSES_SOCID_ADDRESS					0x005c
#define	HSP_FUSES_SOCID_LENGTH					0x10
#define	HSP_FUSES_SOCID_ECC						4

#define	HSP_FUSES_KEY0_ADDRESS					0x0070
#define	HSP_FUSES_KEY0_LENGTH					0x34
#define	HSP_FUSES_KEY0_ECC						16

#define	HSP_FUSES_KEY1_ADDRESS					0x00b4
#define	HSP_FUSES_KEY1_LENGTH					0x34
#define	HSP_FUSES_KEY1_ECC						16

#define	HSP_FUSES_KEY2_ADDRESS					0x00f8
#define	HSP_FUSES_KEY2_LENGTH					0x34
#define	HSP_FUSES_KEY2_ECC						16

#define	HSP_FUSES_KEY3_ADDRESS					0x013c
#define	HSP_FUSES_KEY3_LENGTH					0x34
#define	HSP_FUSES_KEY3_ECC						16

#define	HSP_FUSES_SW0_ADDRESS					0x0180
#define	HSP_FUSES_SW0_LENGTH					0x10
#define	HSP_FUSES_SW0_ECC						0

#define	HSP_FUSES_SW1_ADDRESS					0x0190
#define	HSP_FUSES_SW1_LENGTH					0x30
#define	HSP_FUSES_SW1_ECC						0

#define	HSP_FUSES_SW0_ECC_ADDRESS				0x01c0
#define	HSP_FUSES_SW0_ECC_LENGTH				0x30
#define	HSP_FUSES_SW0_ECC_ECC					12

#define	HSP_FUSES_SW1_ECC_ADDRESS				0x01fc
#define	HSP_FUSES_SW1_ECC_LENGTH				0x30
#define	HSP_FUSES_SW1_ECC_ECC					12

#define	HSP_FUSES_SW2_ECC_ADDRESS				0x0238
#define	HSP_FUSES_SW2_ECC_LENGTH				0x30
#define	HSP_FUSES_SW2_ECC_ECC					12

#define	HSP_FUSES_SW3_ECC_ADDRESS				0x0274
#define	HSP_FUSES_SW3_ECC_LENGTH				0x30
#define	HSP_FUSES_SW3_ECC_ECC					12

#define	HSP_FUSES_SW4_ECC_ADDRESS				0x0000
#define	HSP_FUSES_SW4_ECC_LENGTH				0
#define	HSP_FUSES_SW4_ECC_ECC					0

#define	HSP_FUSES_SW5_ECC_ADDRESS				0x0000
#define	HSP_FUSES_SW5_ECC_LENGTH				0
#define	HSP_FUSES_SW5_ECC_ECC					0

#define	HSP_FUSES_SW6_ECC_ADDRESS				0x0000
#define	HSP_FUSES_SW6_ECC_LENGTH				0
#define	HSP_FUSES_SW6_ECC_ECC					0

#define	HSP_FUSES_RSVD0_ADDRESS					0x02b0
#define	HSP_FUSES_RSVD0_LENGTH					0x100
#define	HSP_FUSES_RSVD0_ECC						0

#define	HSP_FUSES_RSVD1_ADDRESS					0x03b0
#define	HSP_FUSES_RSVD1_LENGTH					0x50
#define	HSP_FUSES_RSVD1_ECC						0

#define	HSP_FUSES_RSVD2_ADDRESS					0x0000
#define	HSP_FUSES_RSVD2_LENGTH					0
#define	HSP_FUSES_RSVD2_ECC						0

#define	HSP_FUSES_RSVD3_ADDRESS					0x0000
#define	HSP_FUSES_RSVD3_LENGTH					0
#define	HSP_FUSES_RSVD3_ECC						0


#endif	/* PLATFORM_FUSES_H_ */
