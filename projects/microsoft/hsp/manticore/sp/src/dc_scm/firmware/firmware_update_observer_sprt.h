// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIRMWARE_UPDATE_OBSERVER_SPRT_H_
#define FIRMWARE_UPDATE_OBSERVER_SPRT_H_

#include "firmware/firmware_update_observer.h"
#include "firmware/impactful_update_interface.h"


/**
 * Handler for firmware update events to enable SPRT state tracking.
 */
struct firmware_update_observer_sprt {
	struct firmware_update_observer base;				/**< Base observer interface. */
	const struct impactful_update_interface *impactful;	/**< Interface to check for impactful updates. */
};


int firmware_update_observer_sprt_init (struct firmware_update_observer_sprt *observer,
	const struct impactful_update_interface *impactful);
void firmware_update_observer_sprt_release (const struct firmware_update_observer_sprt *observer);


#endif	/* FIRMWARE_UPDATE_OBSERVER_SPRT_H_ */
