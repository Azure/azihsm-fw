// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CODE_PATH_INTEGRITY_H_
#define CODE_PATH_INTEGRITY_H_

#include "crypto/rng.h"
#include "drivers/checkpoint.h"
#include "drivers/fuse_controller_interface.h"


/**
 * Manager for code path integrity checks.  This will be a singleton, constant instance within the
 * application.
 */
struct code_path_integrity {
	const struct rng_engine *rng;					/**< Source for random delays when generating tracing output. */
	const struct checkpoint *chkpt;					/**< Hardware checkpoint driver interface. */
	const struct fuse_controller_interface *fuses;	/**< Driver interface to the fuse state. */

	/**
	 * Function to call when there is a checkpoint failure.
	 *
	 * @param status The failure status reported by the checkpoint driver.
	 */
	void (*chkpt_fail) (int status);
};


/* One of these macros must be called in every application to ensure proper configuration of the
 * singleton instance. */

/**
 * Define and initialize the singleton instance for code path integrity enforcement.
 *
 * None of the parameters are validated an all MUST NOT be null.
 *
 * @param cpi_name Name to assign to the instance.
 * @param rng_ptr Interface to the random number generator.
 * @param chkpt_ptr Driver for the hardware checkpoint monitor.
 * @param fuse_ptr Interface to the fuse controller.
 * @param fail The function to call on checkpoint failures.
 */
#define	CODE_PATH_INTEGRITY_HANDLER(cpi_name, rng_ptr, chkpt_ptr, fuse_ptr, fail)   \
	const struct code_path_integrity cpi_name = { \
		.rng = rng_ptr, \
		.chkpt = chkpt_ptr, \
		.fuses = fuse_ptr, \
		.chkpt_fail = fail, \
	}; \
    \
	const struct code_path_integrity *const global_cpi = &cpi_name;

/**
 * Disable code path integrity enforcement.
 */
#define	CODE_PATH_INTEGRITY_NONE    \
	const struct code_path_integrity *const global_cpi = NULL;


void code_path_integrity_message_trace (uint32_t code);
void code_path_integrity_secure_message_trace (uint32_t code);
void code_path_integrity_secure_message_no_trace (uint32_t code);

void code_path_integrity_checkpoint_start (const HSP_CHKPT_CONFIG *config);
void code_path_integrity_checkpoint_hand_off (const HSP_CHKPT_CONFIG *current,
	const HSP_CHKPT_CONFIG *next);
void code_path_integrity_checkpoint (const HSP_CHKPT_CONFIG *config);

void code_path_integrity_random_delay ();


#endif	/* CODE_PATH_INTEGRITY_H_ */
