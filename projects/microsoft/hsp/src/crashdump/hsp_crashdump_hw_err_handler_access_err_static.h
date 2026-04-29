// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_HW_ERR_HANDLER_ACCESS_ERR_STATIC_H_
#define HSP_CRASHDUMP_HW_ERR_HANDLER_ACCESS_ERR_STATIC_H_

#include "hsp_crashdump_hw_err_handler_access_err.h"
#include "hsp_crashdump_hw_err_handler_static.h"


/* Internal functions declared to allow for static initialization. */
bool hsp_crashdump_hw_err_handler_access_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param);

/**
 * Initialize a static instance of a crashdump access error handling.
 *
 * There is no validation done on the arguments
 * @param[in] hsp_crshdump_handler The HSP crashdump handler pointer.
 * @param[in] creg_regs The Creg register pointer.
 * @param[in] access_reg_set The CREG offset for the hardware access registers.
 */
#define hsp_crashdump_hw_err_handler_access_err_static_init(hsp_crshdump_handler, \
	creg_regs, access_reg_set) { \
	.base = hsp_crashdump_hw_err_handler_static_init (hsp_crshdump_handler, \
		creg_regs, hsp_crashdump_hw_err_handler_access_err), \
	.access_regs = access_reg_set, \
}


#endif	/* HSP_CRASHDUMP_HW_ERR_HANDLER_ACCESS_ERR_STATIC_H_ */
