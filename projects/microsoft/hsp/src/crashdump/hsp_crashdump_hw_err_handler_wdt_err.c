// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_crashdump_hw_err_handler_wdt_err.h"
#include "common/type_cast.h"
#include "crashdump/hsp_crashdump_packet.h"
#include "logging/debug_log.h"
#include "logging/hsp_logging.h"


bool hsp_crashdump_hw_err_handler_wdt_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	const struct hsp_crashdump_hw_err_handler_wdt_err *wdt_err_handler =
		TO_DERIVED_TYPE (TO_DERIVED_TYPE (handler, const struct hsp_crashdump_hw_err_handler, base),
		const struct hsp_crashdump_hw_err_handler_wdt_err, base);
	int status;
	uint32_t reg_value = 0;
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data hw_err_data = {
		0,
		HSP_LOGGING_HW_ERROR_WDT_LOG_DBG_R_1_TAG, 0, 0
	};

	status = wdt_err_handler->base.creg->map (wdt_err_handler->base.creg);
	if (status != 0) {
		goto skip_unmap;
	}

	/* Get interrupt status. */
	status = wdt_err_handler->base.creg->read32 (wdt_err_handler->base.creg,
		wdt_err_handler->wdt_regs + CREG_REGS_WDT_REGS_WDT_INTSTS_OFFSET, &reg_value);
	if (status != 0) {
		goto failed;
	}

	if (!reg_value) {
		status = HSP_CRASHDUMP_HW_ERR_HANDLER_WDT_ERR_INT_STATUS_0;
		goto failed;
	}

	hw_err_data.int_status = (uint16_t) (reg_value & 0xFFFF);

	/* Get the Dlock and then verify if it is valid or not. */
	status = wdt_err_handler->base.creg->read32 (wdt_err_handler->base.creg,
		wdt_err_handler->wdt_regs + CREG_REGS_WDT_REGS_LOG_DLOCK_1_OFFSET, &reg_value);
	if (status != 0) {
		goto failed;
	}

	if (reg_value & CREG_REGS_WDT_REGS_LOG_DLOCK_1_VALID_MSB) {
		hw_err_data.status0 = reg_value;
		hw_err_data.err_tag = HSP_LOGGING_HW_ERROR_WDT_LOG_DLOCK_1_TAG;
		goto done;
	}

	/* Get the DBG_W and then verify if it is valid or not. */
	status = wdt_err_handler->base.creg->read32 (wdt_err_handler->base.creg,
		wdt_err_handler->wdt_regs + CREG_REGS_WDT_REGS_LOG_DBG_W_1_OFFSET, &reg_value);
	if (status != 0) {
		goto failed;
	}

	if (reg_value & CREG_REGS_WDT_REGS_LOG_DBG_W_1_VALID_MSB) {
		/* Save DBG_W error. */
		hw_err_data.status0 = reg_value;
		hw_err_data.err_tag = HSP_LOGGING_HW_ERROR_WDT_LOG_DBG_W_1_TAG;

		/* Save SP_W error as well. */
		status = wdt_err_handler->base.creg->read32 (wdt_err_handler->base.creg,
			wdt_err_handler->wdt_regs + CREG_REGS_WDT_REGS_LOG_SP_W_OFFSET, &reg_value);
		if (status != 0) {
			goto failed;
		}

		hw_err_data.status1 = reg_value;
		goto done;
	}

	/* Get the DBG_R and then verify if it is valid or not. */
	status = wdt_err_handler->base.creg->read32 (wdt_err_handler->base.creg,
		wdt_err_handler->wdt_regs + CREG_REGS_WDT_REGS_LOG_DBG_R_1_OFFSET, &reg_value);
	if (status != 0) {
		goto failed;
	}

	if (reg_value & CREG_REGS_WDT_REGS_LOG_DBG_R_1_VALID_MSB) {
		/* Save DBG_R error. */
		hw_err_data.status0 = reg_value;
		hw_err_data.err_tag = HSP_LOGGING_HW_ERROR_WDT_LOG_DBG_R_1_TAG;

		/* Save SP_R error as well. */
		status = wdt_err_handler->base.creg->read32 (wdt_err_handler->base.creg,
			wdt_err_handler->wdt_regs + CREG_REGS_WDT_REGS_LOG_SP_R_OFFSET, &reg_value);
		if (status != 0) {
			goto failed;
		}

		hw_err_data.status1 = reg_value;
	}

done:
	/* Clear the pending interrupt from WDT module. */
	status = wdt_err_handler->base.creg->write32 (wdt_err_handler->base.creg,
		wdt_err_handler->wdt_regs + CREG_REGS_WDT_REGS_WDT_INTSTS_OFFSET,
		CREG_REGS_WDT_REGS_WDT_INTSTS_WRITE_MASK);

failed:
	wdt_err_handler->base.creg->unmap (wdt_err_handler->base.creg);

skip_unmap:
	if (status != 0) {
		/* Return false to cease the interrupt dispatch. */
		return false;
	}

	return hsp_crashdump_hw_err_handler_collect_hsp_crashdump (
		wdt_err_handler->base.crashdump_handler, param, HSP_CRASHDUMP_PACKET_FAULT_CODE_WDT_ERR,
		&hw_err_data);
}

/**
 * Save HW error opaque data to debug log entries.
 *
 * @param[in] buffer The buffer where crashdump opaque data is stored.
 * @param[in] length The number bytes of crashdump opaque data.
 *
 */
void hsp_crashdump_hw_err_handler_wdt_err_save_opaque_data (uint32_t *buffer, size_t length)
{
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data *hw_err_data =
		(struct hsp_crashdump_hw_err_handler_crashdump_opaque_data*) buffer;

	/* TODO: Any additional opaque data in the crash dump packet will not get logged. */
	if ((hw_err_data != NULL) &&
		(length >= sizeof (struct hsp_crashdump_hw_err_handler_crashdump_opaque_data))) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_WDT, HSP_LOGGING_HW_ERROR_WDT_INTSTS_TAG, hw_err_data->int_status);
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_WDT, hw_err_data->err_tag, hw_err_data->status0);

		if (hw_err_data->err_tag != HSP_LOGGING_HW_ERROR_WDT_LOG_DLOCK_1_TAG) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
				HSP_LOGGING_HW_ERROR_WDT, hw_err_data->err_tag + 2, hw_err_data->status1);
		}
	}
}

/**
 * Initialize crashdump HSP HW error WDT error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error WDT error interrupt handler to initialize.
 * @param[in] crashdump_handler The HSP crashdump handler.
 * @param[in] creg The The CREG register interface (struct Creg_regs).
 * @param[in] wdt_regs The CREG offset for the hardware WDT registers.
 *
 * @return 0 if the interrupt handler was successfully initialized or an error code.
 */
int hsp_crashdump_hw_err_handler_wdt_err_init (
	struct hsp_crashdump_hw_err_handler_wdt_err *handler,
	const struct hsp_crashdump_handler *crashdump_handler, const struct mmio_register_block *creg,
	size_t wdt_regs)
{
	int status;

	if ((handler == NULL) || (crashdump_handler == NULL) || (creg == NULL)) {
		return HSP_CRASHDUMP_HW_ERR_HANDLER_WDT_ERR_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct hsp_crashdump_hw_err_handler_wdt_err));

	status = hsp_crashdump_hw_err_handler_init (&handler->base, crashdump_handler, creg,
		hsp_crashdump_hw_err_handler_wdt_err);

	if (status != 0) {
		return status;
	}

	handler->wdt_regs = wdt_regs;

	return 0;
}

/**
 * Release the resources used for crashdump HSP HW error WDT error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error WDT error interrupt handler to release.
 */
void hsp_crashdump_hw_err_handler_wdt_err_release (
	const struct hsp_crashdump_hw_err_handler_wdt_err *handler)
{
	hsp_crashdump_hw_err_handler_release (&handler->base);
}
