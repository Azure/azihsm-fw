// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INTRUSION_MANAGER_MSFT_H_
#define INTRUSION_MANAGER_MSFT_H_

#include "intrusion/intrusion_manager.h"
#include "msft_protocol/mctp_notifier_msft.h"


/**
 * An Microsoft intrusion manager that handle intrusion event and supports notification
 * of intrusion state specific to Microsoft Platform.
 */
struct intrusion_manager_msft {
	struct intrusion_manager base;							/**< Base manager instance. */
	const struct mctp_notifier_interface *mctp_notifier;	/**< MCTP notifier instance. */
};


int intrusion_manager_msft_init (struct intrusion_manager_msft *manager,
	const struct intrusion_state *state, const struct hash_engine *hash, struct pcr_store *pcr,
	uint16_t measurement, const struct mctp_notifier_interface *mctp_notifier);
void intrusion_manager_msft_release (struct intrusion_manager_msft *manager);


#endif	/* INTRUSION_MANAGER_MSFT_H_ */
