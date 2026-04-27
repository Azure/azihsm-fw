// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_HW_ERR_HANDLER_WDT_ERR_STATIC_H_
#define HSP_CRASHDUMP_HW_ERR_HANDLER_WDT_ERR_STATIC_H_

#include "hsp_crashdump_hw_err_handler_static.h"
#include "hsp_crashdump_hw_err_handler_wdt_err.h"


/* Internal functions declared to allow for static initialization. */
bool hsp_crashdump_hw_err_handler_wdt_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param);

/**
 * Initialize a static instance of a crashdump WDT error handling.
 *
 * There is no validation done on the arguments
 * @param[in] hsp_crshdump_handler The HSP crashdump handler pointer.
 * @param[in] creg_regs The Creg register pointer.
 * @param[in] wdt_reg_set The CREG offset for the hardware WDT registers.
 */
#define hsp_crashdump_hw_err_handler_wdt_err_static_init(hsp_crshdump_handler, \
	creg_regs, wdt_reg_set) { \
	.base = hsp_crashdump_hw_err_handler_static_init (hsp_crshdump_handler, \
		creg_regs, hsp_crashdump_hw_err_handler_wdt_err), \
	.wdt_regs = wdt_reg_set, \
}


#endif	/* HSP_CRASHDUMP_HW_ERR_HANDLER_WDT_ERR_STATIC_H_ */
