// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "crypto/ecc_engine_ecc_hw_async.h"


/**
 * Enumeration of all the states for the ECC async SW state machine.
 */
enum {
	ECC_ENGINE_ECC_HW_ASYNC_STATE_FREE = 0,	/**< SW is in free state. */
	ECC_ENGINE_ECC_HW_ASYNC_STATE_BUSY = 1,	/**< SW is busy processing a command. */
	ECC_ENGINE_ECC_HW_ASYNC_STATE_MAX = 2,	/**< Maximum number of states for the state machine. */
};

/**
 * Internal enumeration to describe the next action that the ECC async SW state machine should
 * take.
 */
enum {
	ECC_ENGINE_ECC_HW_ASYNC_ACTION_SW_ERR = 1,		/**< Generic SW programming error. Ideally, this should never be returned. */
	ECC_ENGINE_ECC_HW_ASYNC_ACTION_HW_BUSY = 2,		/**< HW is busy processing a command. */
	ECC_ENGINE_ECC_HW_ASYNC_ACTION_HW_DONE = 3,		/**< HW has completed processing the current command. */
	ECC_ENGINE_ECC_HW_ASYNC_ACTION_HW_ERROR = 4,	/**< HW has encountered an error. */
	ECC_ENGINE_ECC_HW_ASYNC_ACTION_HW_FREE = 5,		/**< HW is free. */
};


/**
 * Initialize variable context of the ECC HW async driver.
 *
 * @param engine Instance of the ECC HW async engine.
 * @return 0 if the state is successfully initialized, error code otherwise.
 */
int ecc_engine_ecc_hw_async_init_state (struct ecc_engine_ecc_hw_async *engine)
{
	return -1;
}

/**
 * Initialize an ECC engine using the ECC hardware.
 *
 * @param engine The ECC instance to initialize.
 * @param state Pointer to variable context of the ECC HW async driver.
 * @param ecc_hw Pointer to the ECC HW engine.
 *
 * @return 0 if the ECC engine was successfully initialized or an error code.
 */
int ecc_engine_ecc_hw_async_init (struct ecc_engine_ecc_hw_async *engine,
	struct ecc_engine_ecc_hw_async_state *state, struct ecc_hw *ecc_hw)
{
	return -1;
}

/**
 * Release the resources used by an ECC HW instance.
 *
 * @param engine The ECC instance to release.
 */
void ecc_engine_ecc_hw_async_release (struct ecc_engine_ecc_hw_async *engine)
{

}
