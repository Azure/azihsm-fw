// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_H_
#define AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_H_

#include "common/authorized_execution.h"
#include "firmware/identity_renewal.h"


/**
 * Provides an authorized execution context for renewing DICE or DME identity keys.
 */
struct authorized_execution_identity_renewal {
	struct authorized_execution base;			/**< Base execution API. */
	const struct identity_renewal *identity;	/**< Handler for managing renewal counters. */
};


int authorized_execution_identity_renewal_init_dme (
	struct authorized_execution_identity_renewal *execution,
	const struct identity_renewal *identity);
int authorized_execution_identity_renewal_init_dice (
	struct authorized_execution_identity_renewal *execution,
	const struct identity_renewal *identity);
void authorized_execution_identity_renewal_release (
	const struct authorized_execution_identity_renewal *execution);


#endif	/* AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_H_ */
