// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_STATIC_H_
#define AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_STATIC_H_

#include "authorized_execution_identity_renewal.h"
#include "logging/hsp_logging.h"


/* Internal functions declared to allow for static initialization. */
int authorized_execution_identity_renewal_execute_dme (const struct authorized_execution *execution,
	const uint8_t *data, size_t length, bool *reset_req);
int authorized_execution_identity_renewal_execute_dice (
	const struct authorized_execution *execution, const uint8_t *data, size_t length,
	bool *reset_req);


/**
 * Constant initializer for the execution API for DME.
 */
#define	AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_API_INIT_DME	{ \
		.execute = authorized_execution_identity_renewal_execute_dme, \
		.validate_data = authorized_execution_validate_data, \
		.get_status_identifiers = authorized_execution_get_status_identifiers, \
	}

/**
 * Constant initializer for the execution API for DICE.
 */
#define	AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_API_INIT_DICE	{ \
		.execute = authorized_execution_identity_renewal_execute_dice, \
		.validate_data = authorized_execution_validate_data, \
		.get_status_identifiers = authorized_execution_get_status_identifiers, \
	}


/**
 * Initialize a static authorized execution context to renew the device's DME key.
 *
 * There is no validation done on the arguments.
 *
 * @param identity_ptr Interface to the hardware management of the DME renewal counter.
 */
#define	authorized_execution_identity_renewal_static_init_dme(identity_ptr)	{ \
		.base = AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_API_INIT_DME, \
		.identity = identity_ptr, \
	}

/**
 * Initialize a static authorized execution context to renew the DICE identity key.
 *
 * There is no validation done on the arguments.
 *
 * @param identity_ptr Interface to the hardware management of the DICE renewal counter.
 */
#define	authorized_execution_identity_renewal_static_init_dice(identity_ptr)	{ \
		.base = AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_API_INIT_DICE, \
		.identity = identity_ptr, \
	}


#endif	/* AUTHORIZED_EXECUTION_IDENTITY_RENEWAL_STATIC_H_ */
