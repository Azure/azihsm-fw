// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_SOC_REV_H_
#define MANTICORE_SOC_REV_H_


/**
 * Tag to apply to the SOCID for the A0 chip revision.
 */
#define	MANTICORE_ROM_A0_SOCID_TAG			0x00

/**
 * Tag to apply to the SOCID for the B0 chip revision.
 */
#define	MANTICORE_ROM_B0_SOCID_TAG			0x10

/**
 * Indicate if the chip is A0 based on the SOCID.
 *
 * @param x The first word of the device SOCID.
 */
#define	MANTICORE_IS_A0(x)					(((x) & 0xff) == MANTICORE_ROM_A0_SOCID_TAG)


#endif /* MANTICORE_SOC_REV_H_ */
