// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef UID_EFUSE_STATIC_H_
#define UID_EFUSE_STATIC_H_

#include "drivers/uid_efuse.h"


/**
 * Initialize a static UID eFuse instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the UID eFuse instance.
 * @param base_addr Base address of UID eFuses.
 */
#define uid_efuse_static_init(state_ptr, base_addr) { \
		.state = state_ptr, \
		.addr = base_addr \
	}


#endif	/* UID_EFUSE_STATIC_H_ */
