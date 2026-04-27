// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_crashdump_hw_err_handler_rng_err.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "crashdump/hsp_crashdump_packet.h"
#include "logging/debug_log.h"
#include "logging/hsp_logging.h"


bool hsp_crashdump_hw_err_handler_rng_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	const struct hsp_crashdump_hw_err_handler_rng_err *rng_err_handler =
		TO_DERIVED_TYPE (TO_DERIVED_TYPE (handler, const struct hsp_crashdump_hw_err_handler, base),
		const struct hsp_crashdump_hw_err_handler_rng_err, base);
	int status;
	uint32_t enabled_interrupt = 0;
	uint32_t intsts_value = 0;
	struct debug_log_entry_info rng_log = {};
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data hw_err_data = {0, 0, 0, 0};

	UNUSED (param);

	status = rng_err_handler->base.creg->map (rng_err_handler->base.creg);
	if (status != 0) {
		goto skip_all_unmaps;
	}

	status = rng_err_handler->rng_regs->map (rng_err_handler->rng_regs);
	if (status != 0) {
		goto skip_rng_unmap;
	}

	/* TODO: Remove the logic below that disables the RNG error interrupt. Currently, the RNG error
	 * interrupt is disabled within the interrupt handler to prevent it from being triggered
	 * multiple times while the handler is still processing the current interrupt. This is a
	 * temporary workaround and should be removed. */
	status = rng_err_handler->base.creg->read32 (rng_err_handler->base.creg,
		rng_err_handler->crypto_regs + CREG_REGS_CREG_CRYPTO_GROUP_CRYPTO_ERR_INTEN_OFFSET,
		&enabled_interrupt);
	if (status != 0) {
		goto failed;
	}

	enabled_interrupt &= ~HSP_CRASHDUMP_HW_ERR_HANDLER_RNG_ERR_INT_MASKS;
	status = rng_err_handler->base.creg->write32 (rng_err_handler->base.creg,
		rng_err_handler->crypto_regs + CREG_REGS_CREG_CRYPTO_GROUP_CRYPTO_ERR_INTEN_OFFSET,
		enabled_interrupt);
	if (status != 0) {
		goto failed;
	}

	/* Get interrupt status. */
	status = rng_err_handler->base.creg->read32 (rng_err_handler->base.creg,
		rng_err_handler->crypto_regs +
		CREG_REGS_CREG_CRYPTO_GROUP_CRYPTO_ERR_INTSTS_OFFSET, &intsts_value);
	if (status != 0) {
		goto failed;
	}

	if (!intsts_value) {
		status = HSP_CRASHDUMP_HW_ERR_HANDLER_RNG_ERR_INT_STATUS_0;
		goto failed;
	}

	hw_err_data.int_status = (uint16_t) (intsts_value & 0xFFFF);

	/* Get status log. */
	status = rng_err_handler->rng_regs->read32 (rng_err_handler->rng_regs, RNG_REGS_STATUS_OFFSET,
		&hw_err_data.status0);
	if (status != 0) {
		goto failed;
	}

	/* Clear the pending interrupt from crypto module. */
	status = rng_err_handler->base.creg->write32 (rng_err_handler->base.creg,
		rng_err_handler->crypto_regs + CREG_REGS_CREG_CRYPTO_GROUP_CRYPTO_ERR_INTSTS_OFFSET,
		HSP_CRASHDUMP_HW_ERR_HANDLER_RNG_ERR_INT_MASKS);

failed:
	rng_err_handler->rng_regs->unmap (rng_err_handler->rng_regs);

skip_rng_unmap:
	rng_err_handler->base.creg->unmap (rng_err_handler->base.creg);

skip_all_unmaps:
	if (status != 0) {
		/* Return false to cease the interrupt dispatch. */
		return false;
	}

	/* Log the error. */
	rng_log.severity = DEBUG_LOG_SEVERITY_ERROR;
	rng_log.component = DEBUG_LOG_COMPONENT_HSP;
	rng_log.msg_index = HSP_LOGGING_HW_ERROR_RNG;
	rng_log.arg1 = HSP_LOGGING_HW_ERROR_RNG_STATUS_LOG_TAG;
	rng_log.arg2 = hw_err_data.status0;
	rng_log.format = 1;

	/* Enter in error state */
	rng_err_handler->error->enter_error_state (rng_err_handler->error, &rng_log);

	return true;
}

/**
 * Save HW error opaque data to debug log entries.
 *
 * @param[in] buffer The buffer where crashdump opaque data is stored.
 * @param[in] length The number bytes of crashdump opaque data.
 *
 */
void hsp_crashdump_hw_err_handler_rng_err_save_opaque_data (uint32_t *buffer, size_t length)
{
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data *hw_err_data =
		(struct hsp_crashdump_hw_err_handler_crashdump_opaque_data*) buffer;

	/* TODO: Any additional opaque data in the crash dump packet will not get logged. */
	if ((hw_err_data != NULL) &&
		(length >= sizeof (struct hsp_crashdump_hw_err_handler_crashdump_opaque_data))) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_RNG, HSP_LOGGING_HW_ERROR_RNG_INTSTS_TAG, hw_err_data->int_status);
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_RNG, HSP_LOGGING_HW_ERROR_RNG_STATUS_LOG_TAG,
			hw_err_data->status0);
	}
}

/**
 * Initialize crashdump HSP HW error RNG error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error RNG error interrupt handler to initialize.
 * @param[in] crashdump_handler The HSP crashdump handler.
 * @param[in] creg The The CREG register interface (struct Creg_regs).
 * @param[in] crypto_regs The CREG offset for the hardware crypto registers.
 * @param[in] rng_regs The RNG registers referred for error logging.
 * @param[in] error The entry to error state management interface.
 *
 * @return 0 if the interrupt handler was successfully initialized or an error code.
 */
int hsp_crashdump_hw_err_handler_rng_err_init (
	struct hsp_crashdump_hw_err_handler_rng_err *handler,
	const struct hsp_crashdump_handler *crashdump_handler, const struct mmio_register_block *creg,
	size_t crypto_regs, const struct mmio_register_block *rng_regs,
	const struct error_state_entry_interface *error)
{
	int status;

	if ((handler == NULL) || (crashdump_handler == NULL) || (creg == NULL) ||
		(rng_regs == NULL) || (error == NULL)) {
		return HSP_CRASHDUMP_HW_ERR_HANDLER_RNG_ERR_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct hsp_crashdump_hw_err_handler_rng_err));

	status = hsp_crashdump_hw_err_handler_init (&handler->base, crashdump_handler, creg,
		hsp_crashdump_hw_err_handler_rng_err);

	if (status != 0) {
		return status;
	}

	handler->crypto_regs = crypto_regs;
	handler->rng_regs = rng_regs;
	handler->error = error;

	return 0;
}

/**
 * Release the resources used for crashdump HSP HW error RNG error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error RNG error interrupt handler to release.
 */
void hsp_crashdump_hw_err_handler_rng_err_release (
	const struct hsp_crashdump_hw_err_handler_rng_err *handler)
{
	hsp_crashdump_hw_err_handler_release (&handler->base);
}
