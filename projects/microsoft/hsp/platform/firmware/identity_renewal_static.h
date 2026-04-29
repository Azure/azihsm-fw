// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef IDENTITY_RENEWAL_STATIC_H_
#define IDENTITY_RENEWAL_STATIC_H_

#include "firmware/identity_renewal.h"


/* Internal functions declared to allow for static initialization. */
int identity_renewal_get_dme_renewal (const struct identity_renewal *identity, uint32_t *dme);
int identity_renewal_set_dme_renewal (const struct identity_renewal *identity, uint32_t dme);
int identity_renewal_get_dice_renewal (const struct identity_renewal *identity, uint32_t *dice);
int identity_renewal_set_dice_renewal (const struct identity_renewal *identity, uint32_t dice);


/**
 * Initialize a static instance for identity renewal management.
 *
 * There is no validation done on the arguments.
 *
 * @param fuses_ptr Base address of the hardware registers for the fuses.
 */
#define	identity_renewal_static_init(fuses_ptr)	{ \
		.get_dme_renewal = identity_renewal_get_dme_renewal, \
		.set_dme_renewal = identity_renewal_set_dme_renewal, \
		.get_dice_renewal = identity_renewal_get_dice_renewal, \
		.set_dice_renewal = identity_renewal_set_dice_renewal, \
		.fuses = fuses_ptr, \
	}


#endif	/* IDENTITY_RENEWAL_STATIC_H_ */
