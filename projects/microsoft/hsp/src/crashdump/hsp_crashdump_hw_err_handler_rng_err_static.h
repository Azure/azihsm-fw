// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_HW_ERR_HANDLER_RNG_ERR_STATIC_H_
#define HSP_CRASHDUMP_HW_ERR_HANDLER_RNG_ERR_STATIC_H_

#include "hsp_crashdump_hw_err_handler_rng_err.h"
#include "hsp_crashdump_hw_err_handler_static.h"


/* Internal functions declared to allow for static initialization. */
bool hsp_crashdump_hw_err_handler_rng_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param);

/**
 * Initialize a static instance of a crashdump RNG error handling.
 *
 * There is no validation done on the arguments
 * @param[in] hsp_crshdump_handler The HSP crashdump handler pointer.
 * @param[in] creg_regs The Creg register pointer.
 * @param[in] crypto_reg_set The CREG offset for the hardware crypto registers.
 * @param[in] rng_regs_ptr The pointer of RNG registers referred for error logging.
 * @param[in] error_ptr The entry to error state management interface.
*/
#define hsp_crashdump_hw_err_handler_rng_err_static_init(hsp_crshdump_handler, \
	creg_regs, crypto_reg_set, rng_regs_ptr, error_ptr) { \
	.base = hsp_crashdump_hw_err_handler_static_init (hsp_crshdump_handler, \
		creg_regs, hsp_crashdump_hw_err_handler_rng_err), \
	.crypto_regs = crypto_reg_set, \
	.rng_regs = rng_regs_ptr, \
	.error = error_ptr, \
}


#endif	/* HSP_CRASHDUMP_HW_ERR_HANDLER_RNG_ERR_STATIC_H_ */
