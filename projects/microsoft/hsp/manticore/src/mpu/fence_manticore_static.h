// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FENCE_MANTICORE_STATIC_H_
#define FENCE_MANTICORE_STATIC_H_

#include "fence_manticore.h"


/* Forward declarations */
int fence_manticore_apply (const struct fence_interface *fence,
	const struct fence_policy_block *fence_blocks, uint32_t blocks_count);

/**
 * Static initializer for fence interface API
 */
#define FENCE_MANTICORE_API_INIT { \
	.apply = fence_manticore_apply, \
}

/**
 * Static initializer for manticore memory fencing interface
 *
 * @param fence_registers_arg - memory fencing registers block
 */
#define fence_manticore_static_init(fence_registers_arg) { \
	.base = FENCE_MANTICORE_API_INIT, \
	.fence_registers = fence_registers_arg, \
}


#endif	// FENCE_MANTICORE_STATIC_H_
