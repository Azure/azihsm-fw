// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_INTERRUPT_HANDLER_STATIC_H_
#define HSP_INTERRUPT_HANDLER_STATIC_H_

#include "hsp_interrupt_handler.h"


/**
 * Constant initializer for an interrupt handler instance
 *
 * There is no validation done on the arguments
 *
 * @param handle_intr The interrupt handler routine
 */
#define hsp_interrupt_handler_static_init(handle_intr) {\
		.handle_interrupt = handle_intr \
	}


#endif	/* HSP_INTERRUPT_HANDLER_STATIC_H_ */
