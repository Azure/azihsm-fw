// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "common/array_size.h"
#include "common/unused.h"
#include "drivers/hsp_fuses.h"
#include "firmware/identity_renewal.h"
#include "splibs/inc/spcryptotypes.h"


/**
 * Get an identity renewal counter from SW1 fuses.
 *
 * @param identity The identity renewal to retrieve.
 * @param offset Offset within the data structure for the counter, represented in dwords.
 * @param renewal Output for the counter value in fuses.
 *
 * @return 0 if the renewal counter was read successfully or an error code.
 */
static int identity_renewal_get_renewal (const struct identity_renewal *identity, uint16_t offset,
	uint32_t *renewal)
{
	struct hsp_fuses_sw1 sw1;
	size_t i;
	int status;

	if ((identity == NULL) || (renewal == NULL)) {
		return IDENTITY_RENEWAL_INVALID_ARGUMENT;
	}

	status = identity->fuses->read_registered_sw_fuses (identity->fuses, HSP_FUSES_ADDRESS (SW1),
		(uint8_t*) &sw1, sizeof (sw1));
	if (status != 0) {
		return status;
	}

	*renewal = 0;

	/* Read all redundant copies of the renewal data and set all bits. */
	for (i = 0; i < ARRAY_SIZE (sw1.data); i++) {
		*renewal |= ((uint32_t*) &sw1.data[i])[offset];
	}

	return 0;
}

/**
 * Update an identity renewal counter into SW1 fuses.
 *
 * @param identity The identity being renewed.
 * @param renewal The counter to program.
 * @param offset Offset within the data structure for the counter.
 *
 * @return 0 if the renewal counter was updated successfully or an error code.
 */
static int identity_renewal_set_renewal (const struct identity_renewal *identity, uint32_t renewal,
	uint16_t offset)
{
	size_t copies = ARRAY_SIZE (((struct hsp_fuses_sw1*) NULL)->data);
	size_t i;
	int status;

	if (identity == NULL) {
		return IDENTITY_RENEWAL_INVALID_ARGUMENT;
	}

	/* Program all redundant copies of the counter.  Do not fail on an error for a single copy. */
	status = -1;
	for (i = 0; i < copies; i++) {
		int prog_status = identity->fuses->program_sw_fuses (identity->fuses,
			(HSP_FUSES_ADDRESS (SW1) + (i * sizeof (struct hsp_fuses_sw1_data)) + offset), &renewal,
			1);

		if (status != 0) {
			/* Latch successful program operations, or report the last failure. */
			status = prog_status;
		}
	}

	return status;
}

int identity_renewal_get_dme_renewal (const struct identity_renewal *identity, uint32_t *dme)
{
	uint16_t dme_offset = IN_DWORDS (offsetof (struct hsp_fuses_sw1_data, dme_renewal));

	return identity_renewal_get_renewal (identity, dme_offset, dme);
}

int identity_renewal_set_dme_renewal (const struct identity_renewal *identity, uint32_t dme)
{
	uint16_t dme_offset = offsetof (struct hsp_fuses_sw1_data, dme_renewal);

	return identity_renewal_set_renewal (identity, dme, dme_offset);
}

int identity_renewal_get_dice_renewal (const struct identity_renewal *identity, uint32_t *dice)
{
	uint16_t dice_offset = IN_DWORDS (offsetof (struct hsp_fuses_sw1_data, dice_renewal));

	return identity_renewal_get_renewal (identity, dice_offset, dice);
}

int identity_renewal_set_dice_renewal (const struct identity_renewal *identity, uint32_t dice)
{
	uint16_t dice_offset = offsetof (struct hsp_fuses_sw1_data, dice_renewal);

	return identity_renewal_set_renewal (identity, dice, dice_offset);
}

/**
 * Initialize the handler for device identity renewal.
 *
 * @param identity The handler instance to initialize.
 * @param fuses Interface the fuse controller where renewal information is stored.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int identity_renewal_init (struct identity_renewal *identity,
	const struct fuse_controller_interface *fuses)
{
	if ((identity == NULL) || (fuses == NULL)) {
		return IDENTITY_RENEWAL_INVALID_ARGUMENT;
	}

	memset (identity, 0, sizeof (struct identity_renewal));

	identity->get_dme_renewal = identity_renewal_get_dme_renewal;
	identity->set_dme_renewal = identity_renewal_set_dme_renewal;
	identity->get_dice_renewal = identity_renewal_get_dice_renewal;
	identity->set_dice_renewal = identity_renewal_set_dice_renewal;

	identity->fuses = fuses;

	return 0;
}

/**
 * Release the resources used by a handler for device identity renewal.
 *
 * @param identity The handler instance to release.
 */
void identity_renewal_release (const struct identity_renewal *identity)
{
	UNUSED (identity);
}
