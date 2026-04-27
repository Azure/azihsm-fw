// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef ECC_ENGINE_ECC_HW_ASYNC_H_
#define ECC_ENGINE_ECC_HW_ASYNC_H_

#include "crypto/ecc.h"
#include "crypto/ecc_hw.h"


/**
 * Variable context for the ECC HW async block.
 */
struct ecc_engine_ecc_hw_async_state {
	uint8_t current_state;	/**< Current state of the ECC HW async engine. */
};

/**
 * Interface for using the ECC hardware block for ECC operations.
 */
struct ecc_engine_ecc_hw_async {
	struct ecc_engine base;							/**< The base ECC engine. */
	struct ecc_engine_ecc_hw_async_state *state;	/**< The variable state of the engine. */
	struct ecc_hw *ecc_hw;							/**< The ECC HW engine to use. */
};


int ecc_engine_ecc_hw_async_init (struct ecc_engine_ecc_hw_async *engine,
	struct ecc_engine_ecc_hw_async_state *state, struct ecc_hw *ecc_hw);
int ecc_engine_ecc_hw_async_init_state (struct ecc_engine_ecc_hw_async *engine);
void ecc_engine_ecc_hw_async_release (struct ecc_engine_ecc_hw_async *engine);


#endif	/* ECC_ENGINE_ECC_HW_ASYNC_H_ */
