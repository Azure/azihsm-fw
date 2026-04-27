// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef ECC_ENGINE_ECC_HW_ASYNC_STATIC_H_
#define ECC_ENGINE_ECC_HW_ASYNC_STATIC_H_

#include "crypto/ecc_engine_ecc_hw_async.h"


/**
 * Initialize a static instance of a ECC HW async driver.
 *
 * There is no validation done on the arguments.
 *
 * @param ecc_base The ECC engine to use.
 * @param state_ptr Pointer to the variable context of ECC HW async driver.
 * @param ecc_hw_ptr Interface to the ECC HW engine.  This can be a constant
 * instance.
 */
#define	ecc_engine_ecc_hw_async_static_init(ecc_base, state_ptr, ecc_hw_ptr)	{ \
	}


#endif	/* ECC_ENGINE_ECC_HW_ASYNC_STATIC_H_ */
