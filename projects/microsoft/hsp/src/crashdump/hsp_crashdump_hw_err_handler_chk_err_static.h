// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_HW_ERR_HANDLER_CHK_ERR_STATIC_H_
#define HSP_CRASHDUMP_HW_ERR_HANDLER_CHK_ERR_STATIC_H_

#include "hsp_crashdump_hw_err_handler_chk_err.h"
#include "hsp_crashdump_hw_err_handler_static.h"


/* Internal functions declared to allow for static initialization. */
bool hsp_crashdump_hw_err_handler_chk_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param);

/**
 * Initialize a static instance of a crashdump chk error handling.
 *
 * There is no validation done on the arguments
 * @param[in] hsp_crshdump_handler The HSP crashdump handler pointer.
 * @param[in] creg_regs The Creg register pointer.
 * @param[in] chk_reg_set The CREG offset for the hardware check point registers.
 */
#define hsp_crashdump_hw_err_handler_chk_err_static_init(hsp_crshdump_handler, \
	creg_regs, chk_reg_set) { \
	.base = hsp_crashdump_hw_err_handler_static_init (hsp_crshdump_handler, \
		creg_regs, hsp_crashdump_hw_err_handler_chk_err), \
	.chk_regs = chk_reg_set, \
}


#endif	/* HSP_CRASHDUMP_HW_ERR_HANDLER_CHK_ERR_STATIC_H_ */
