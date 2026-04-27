// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_crashdump_hw_err_handler_access_err.h"
#include "common/type_cast.h"
#include "crashdump/hsp_crashdump_packet.h"
#include "logging/debug_log.h"
#include "logging/hsp_logging.h"


bool hsp_crashdump_hw_err_handler_access_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	const struct hsp_crashdump_hw_err_handler_access_err *access_err_handler =
		TO_DERIVED_TYPE (TO_DERIVED_TYPE (handler, const struct hsp_crashdump_hw_err_handler, base),
		const struct hsp_crashdump_hw_err_handler_access_err, base);
	int status;
	uint32_t intsts_value = 0;
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data hw_err_data = {0, 0, 0, 0};

	status = access_err_handler->base.creg->map (access_err_handler->base.creg);
	if (status != 0) {
		goto skip_unmap;
	}

	/* Get interrupt status. */
	status = access_err_handler->base.creg->read32 (access_err_handler->base.creg,
		access_err_handler->access_regs +
		CREG_REGS_CREG_ACC_REGS_CREG_ACC_VIO_INTSTS_OFFSET,	&intsts_value);
	if (status != 0) {
		goto failed;
	}

	if (!intsts_value) {
		status = HSP_CRASHDUMP_HW_ERR_HANDLER_ACCESS_ERR_INT_STATUS_0;
		goto failed;
	}

	hw_err_data.int_status = (uint16_t) (intsts_value & 0xFFFF);

	/* Get log data. */
	status = access_err_handler->base.creg->read32 (access_err_handler->base.creg,
		access_err_handler->access_regs +
		CREG_REGS_CREG_ACC_REGS_CREG_ACC_VIO_LOG0_OFFSET, &hw_err_data.status0);
	if (status != 0) {
		goto failed;
	}

	status = access_err_handler->base.creg->read32 (access_err_handler->base.creg,
		access_err_handler->access_regs +
		CREG_REGS_CREG_ACC_REGS_CREG_ACC_VIO_LOG1_OFFSET, &hw_err_data.status1);
	if (status != 0) {
		goto failed;
	}

	/* Clear the pending interrupt from access module. */
	status = access_err_handler->base.creg->write32 (access_err_handler->base.creg,
		access_err_handler->access_regs +
		CREG_REGS_CREG_ACC_REGS_CREG_ACC_VIO_INTSTS_OFFSET,
		CREG_REGS_CREG_ACC_REGS_CREG_ACC_VIO_INTSTS_WRITE_MASK);

failed:
	access_err_handler->base.creg->unmap (access_err_handler->base.creg);

skip_unmap:
	if (status != 0) {
		/* Return false, to cease the interrupt dispatch. */
		return false;
	}

	return hsp_crashdump_hw_err_handler_collect_hsp_crashdump (
		access_err_handler->base.crashdump_handler, param,
		HSP_CRASHDUMP_PACKET_FAULT_CODE_ACCESS_ERR,	&hw_err_data);
}

/**
 * Save HW error opaque data to debug log entries.
 *
 * @param[in] buffer The buffer where crashdump opaque data is stored.
 * @param[in] length The number bytes of crashdump opaque data.
 *
 */
void hsp_crashdump_hw_err_handler_access_err_save_opaque_data (uint32_t *buffer, size_t length)
{
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data *hw_err_data =
		(struct hsp_crashdump_hw_err_handler_crashdump_opaque_data*) buffer;

	/* TODO: Any additional opaque data in the crash dump packet will not get logged. */
	if ((hw_err_data != NULL) &&
		(length >= sizeof (struct hsp_crashdump_hw_err_handler_crashdump_opaque_data))) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_ACCESS, HSP_LOGGING_HW_ERROR_ACCESS_INTSTS_TAG,
			hw_err_data->int_status);
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_ACCESS, HSP_LOGGING_HW_ERROR_ACCESS_ACC_VIO_LOG0_TAG,
			hw_err_data->status0);
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_ACCESS, HSP_LOGGING_HW_ERROR_ACCESS_ACC_VIO_LOG1_TAG,
			hw_err_data->status1);
	}
}

/**
 * Initialize crashdump HSP HW error access error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error access error interrupt handler to initialize.
 * @param[in] crashdump_handler The HSP crashdump handler.
 * @param[in] creg The The CREG register interface (struct Creg_regs).
 * @param[in] access_regs The CREG offset for the hardware access registers.
 *
 * @return 0 if the interrupt handler was successfully initialized or an error code.
 */
int hsp_crashdump_hw_err_handler_access_err_init (
	struct hsp_crashdump_hw_err_handler_access_err *handler,
	const struct hsp_crashdump_handler *crashdump_handler, const struct mmio_register_block *creg,
	size_t access_regs)
{
	int status;

	if ((handler == NULL) || (crashdump_handler == NULL) || (creg == NULL)) {
		return HSP_CRASHDUMP_HW_ERR_HANDLER_ACCESS_ERR_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct hsp_crashdump_hw_err_handler_access_err));

	status = hsp_crashdump_hw_err_handler_init (&handler->base, crashdump_handler, creg,
		hsp_crashdump_hw_err_handler_access_err);

	if (status != 0) {
		return status;
	}

	handler->access_regs = access_regs;

	return 0;
}

/**
 * Release the resources used for crashdump HSP HW error access error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error access error interrupt handler to release.
 */
void hsp_crashdump_hw_err_handler_access_err_release (
	const struct hsp_crashdump_hw_err_handler_access_err *handler)
{
	hsp_crashdump_hw_err_handler_release (&handler->base);
}
