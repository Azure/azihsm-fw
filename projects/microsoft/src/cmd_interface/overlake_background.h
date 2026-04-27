// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_BACKGROUND_H_
#define OVERLAKE_BACKGROUND_H_

#include "cmd_interface/cmd_interface.h"


/**
 * Extension for running Overlake commands in the background.
 */
struct overlake_background {
	/**
	 * Process a request to decrypt a payload with a device attestation key.
	 *
	 * While the decrypt may actually run in the background, this call blocks until it is complete.
	 *
	 * @param task The background task context.
	 * @param request The request to process.
	 *
	 * @return 0 if the request was successful or an error code.
	 */
	int (*decrypt_payload) (struct overlake_background *task, struct cmd_interface_msg *request);
};


#endif	/* OVERLAKE_BACKGROUND_H_ */
