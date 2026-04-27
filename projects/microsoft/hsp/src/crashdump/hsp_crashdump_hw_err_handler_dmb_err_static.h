// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_HW_ERR_HANDLER_DMB_ERR_STATIC_H_
#define HSP_CRASHDUMP_HW_ERR_HANDLER_DMB_ERR_STATIC_H_

#include "hsp_crashdump_hw_err_handler_dmb_err.h"
#include "hsp_crashdump_hw_err_handler_static.h"


/* Internal functions declared to allow for static initialization. */
bool hsp_crashdump_hw_err_handler_dmb_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param);

/**
 * Initialize a static instance of a crashdump DMB error handling.
 *
 * There is no validation done on the arguments
 * @param[in] hsp_crshdump_handler The HSP crashdump handler pointer.
 * @param[in] creg_regs The Creg register pointer.
 * @param[in] creg_dmb_reg_set DMB CREG offset referred for DMB error interrupt handling.
 * @param[in] dmb_regs_ptr The pointer of DMB registers referred for error logging.
 */
#define hsp_crashdump_hw_err_handler_dmb_err_static_init(hsp_crshdump_handler, \
	creg_regs, creg_dmb_reg_set, dmb_regs_ptr) { \
	.base = hsp_crashdump_hw_err_handler_static_init (hsp_crshdump_handler, \
		creg_regs, hsp_crashdump_hw_err_handler_dmb_err), \
	.creg_dmb_regs = creg_dmb_reg_set, \
	.dmb_regs = dmb_regs_ptr, \
}


#endif	/* HSP_CRASHDUMP_HW_ERR_HANDLER_DMB_ERR_STATIC_H_ */
