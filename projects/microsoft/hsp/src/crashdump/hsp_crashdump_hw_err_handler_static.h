// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_HW_ERR_HANDLER_STATIC_H_
#define HSP_CRASHDUMP_HW_ERR_HANDLER_STATIC_H_

#include "hsp_crashdump_hw_err_handler.h"

/**
 * Initialize a static instance of a crashdump HW error handling.
 *
 * There is no validation done on the arguments
 * @param[in] hsp_crshdump_handler The HSP crashdump handler pointer.
 * @param[in] creg_regs The Creg register pointer.
 * @param[in] hsp_hw_err_handler The HSP HW error interrupt handler pointer.
 */
#define hsp_crashdump_hw_err_handler_static_init(hsp_crshdump_handler, \
	creg_regs, hsp_hw_err_handler) { \
	.crashdump_handler = hsp_crshdump_handler, \
	.creg = creg_regs, \
	.base.handle_interrupt = hsp_hw_err_handler, \
}


#endif	/* HSP_CRASHDUMP_HW_ERR_HANDLER_STATIC_H_ */
