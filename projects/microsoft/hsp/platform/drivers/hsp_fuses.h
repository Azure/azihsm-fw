// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_FUSES_H_
#define HSP_FUSES_H_

#include "platform_fuses.h"


/**
 * The maximum valid fuse address, ignoring alignment.
 */
#define	HSP_FUSES_MAX_ADDRESS					(HSP_FUSES_TOTAL_LENGTH - 1)

/* Helper macros. */
#define	HSP_FUSES_ADDRESS_TYPE(name)			HSP_FUSES_ ## name ## _ADDRESS
#define	HSP_FUSES_LENGTH_TYPE(name)				HSP_FUSES_ ## name ## _LENGTH
#define	HSP_FUSES_ECC_TYPE(name)				HSP_FUSES_ ## name ## _ECC

/**
 * The start address for a fuse slot.
 */
#define	HSP_FUSES_ADDRESS(name)					HSP_FUSES_ADDRESS_TYPE (name)

/**
 * The number of data bytes contained in a fuse slot.
 */
#define	HSP_FUSES_LENGTH(name)					HSP_FUSES_LENGTH_TYPE (name)

/**
 * The number of ECC bytes contained in a fuse slot.
 */
#define	HSP_FUSES_ECC(name)						HSP_FUSES_ECC_TYPE (name)

/**
 * The total number of bytes contained in a fuse slot including both data and ECC bytes.
 */
#define	HSP_FUSES_SLOT_LENGTH(name)				(HSP_FUSES_LENGTH (name) + HSP_FUSES_ECC (name))

/**
 * The end address for a fuse slot.  This is the first address after the slot end, i.e. 1 plus the
 * last valid address.
 */
#define	HSP_FUSES_END_ADDRESS(name)             \
	(HSP_FUSES_ADDRESS (name) + HSP_FUSES_SLOT_LENGTH (name))

/**
 * The last valid address for fuse slot.
 */
#define	HSP_FUSES_LAST_ADDRESS(name)			(HSP_FUSES_END_ADDRESS (name) - 1)

/**
 * The address for the last fuse word in a slot.
 */
#define	HSP_FUSES_LAST_WORD_ADDRESS(name)		(HSP_FUSES_LAST_ADDRESS (name) & (~0x3u))


#endif	/* HSP_FUSES_H_ */
