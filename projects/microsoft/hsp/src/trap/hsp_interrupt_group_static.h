// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_INTERRUPT_GROUP_STATIC_H_
#define HSP_INTERRUPT_GROUP_STATIC_H_

#include "trap/hsp_interrupt_group.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal function exposed for initialization */
bool hsp_interrupt_group_handle_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param);

/**
 * Initialize a static instance of a group of HSP interrupt handlers.
 *
 * There is no validation done on the arguments
 *
 * @param vec_ptr The ptr to an array of handlers
 * @param vec_count The size of the vector
 * @param break_on_first Flag to indicate breaking on first handled or to notify all handlers
 */
#define hsp_interrupt_group_static_init(vec_ptr, vec_count, break_on_first) { \
		.base = hsp_interrupt_handler_static_init (hsp_interrupt_group_handle_interrupt), \
		.count = vec_count, \
		.vector = vec_ptr, \
		.first_handled = break_on_first \
	}


#endif	/* HSP_INTERRUPT_GROUP_STATIC_H_ */
