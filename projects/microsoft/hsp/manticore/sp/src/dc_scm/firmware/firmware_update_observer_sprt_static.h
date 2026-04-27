// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIRMWARE_UPDATE_OBSERVER_SPRT_STATIC_H_
#define FIRMWARE_UPDATE_OBSERVER_SPRT_STATIC_H_

#include "firmware_update_observer_sprt.h"


/* Internal functions declared to allow for static initialization. */
void firmware_update_observer_sprt_on_update_applied (
	const struct firmware_update_observer *observer);


/**
 * Constant initializer for the firmware update event handlers.
 */
#define	FIRMWARE_UPDATE_OBSERVER_SPRT_API_INIT  { \
		.on_update_start = NULL, \
		.on_prepare_update = NULL, \
		.on_update_applied = firmware_update_observer_sprt_on_update_applied, \
	}


/**
 * Initialize a static instance for a firmware update observer used to track SPRT impactless update
 * state.
 *
 * There is no validation done on the arguments.
 *
 * @param impactful_ptr Interface to use for checking whether the update was impactful.
 */
#define	firmware_update_observer_sprt_static_init(impactful_ptr)	{ \
		.base = FIRMWARE_UPDATE_OBSERVER_SPRT_API_INIT, \
		.impactful = impactful_ptr, \
	}


#endif	/* FIRMWARE_UPDATE_OBSERVER_SPRT_STATIC_H_ */
