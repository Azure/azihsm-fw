// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_crashdump_hw_err_handler_dmb_err.h"
#include "common/type_cast.h"
#include "crashdump/hsp_crashdump_packet.h"
#include "logging/debug_log.h"
#include "logging/hsp_logging.h"


bool hsp_crashdump_hw_err_handler_dmb_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	const struct hsp_crashdump_hw_err_handler_dmb_err *dmb_err_handler =
		TO_DERIVED_TYPE (TO_DERIVED_TYPE (handler, const struct hsp_crashdump_hw_err_handler, base),
		const struct hsp_crashdump_hw_err_handler_dmb_err, base);
	int status;
	uint32_t reg_value = 0;
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data hw_err_data = {0, 0, 0, 0};

	status = dmb_err_handler->base.creg->map (dmb_err_handler->base.creg);
	if (status != 0) {
		goto skip_all_unmaps;
	}

	status = dmb_err_handler->dmb_regs->map (dmb_err_handler->dmb_regs);
	if (status != 0) {
		goto skip_dmb_unmap;
	}

	/* Get interrupt status. */
	status = dmb_err_handler->base.creg->read32 (dmb_err_handler->base.creg,
		dmb_err_handler->creg_dmb_regs + CREG_REGS_CREG_DMB_REGS_DMB_INTSTS_OFFSET, &reg_value);
	if (status != 0) {
		goto failed;
	}

	if (!reg_value) {
		status = HSP_CRASHDUMP_HW_ERR_HANDLER_DMB_ERR_INT_STATUS_0;
		goto failed;
	}

	hw_err_data.int_status = (reg_value & 0xFFFF);

	/* Get logs. */
	status = dmb_err_handler->dmb_regs->read32 (dmb_err_handler->dmb_regs,
		DMB_REG_DMB_ERRLOG1_OFFSET,	&hw_err_data.status0);
	if (status != 0) {
		goto failed;
	}

	status = dmb_err_handler->dmb_regs->read32 (dmb_err_handler->dmb_regs,
		DMB_REG_DMB_ERRLOG2_OFFSET,	&hw_err_data.status1);
	if (status != 0) {
		goto failed;
	}

	/* Clear the pending interrupt from DMB module. */
	status = dmb_err_handler->base.creg->write32 (dmb_err_handler->base.creg,
		dmb_err_handler->creg_dmb_regs + CREG_REGS_CREG_DMB_REGS_DMB_INTSTS_OFFSET,
		CREG_REGS_CREG_DMB_REGS_DMB_INTSTS_WRITE_MASK);

failed:
	dmb_err_handler->dmb_regs->unmap (dmb_err_handler->dmb_regs);

skip_dmb_unmap:
	dmb_err_handler->base.creg->unmap (dmb_err_handler->base.creg);

skip_all_unmaps:
	if (status != 0) {
		/* Return false to cease the interrupt dispatch. */
		return false;
	}

	return hsp_crashdump_hw_err_handler_collect_hsp_crashdump (
		dmb_err_handler->base.crashdump_handler, param, HSP_CRASHDUMP_PACKET_FAULT_CODE_DMB_ERR,
		&hw_err_data);
}

/**
 * Save HW error opaque data to debug log entries.
 *
 * @param[in] buffer The buffer where crashdump opaque data is stored.
 * @param[in] length The number bytes of crashdump opaque data.
 *
 */
void hsp_crashdump_hw_err_handler_dmb_err_save_opaque_data (uint32_t *buffer, size_t length)
{
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data *hw_err_data =
		(struct hsp_crashdump_hw_err_handler_crashdump_opaque_data*) buffer;

	/* TODO: Any additional opaque data in the crash dump packet will not get logged. */
	if ((hw_err_data != NULL) &&
		(length >= sizeof (struct hsp_crashdump_hw_err_handler_crashdump_opaque_data))) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_DMB, HSP_LOGGING_HW_ERROR_DMB_INTSTS_TAG, hw_err_data->int_status);
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_DMB, HSP_LOGGING_HW_ERROR_DMB_ERRLOG1_TAG, hw_err_data->status0);
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_DMB, HSP_LOGGING_HW_ERROR_DMB_ERRLOG2_TAG, hw_err_data->status1);
	}
}

/**
 * Initialize crashdump HSP HW error DMB error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error DMB error interrupt handler to initialize.
 * @param[in] crashdump_handler The HSP crashdump handler.
 * @param[in] creg The The CREG register interface (struct Creg_regs).
 * @param[in] dmb_regs The CREG offset for the hardware DMB registers.
 * @param[in] creg_dmb_regs The DMB CREG offset referred for DMB error interrupt handling.
 * @param[in] dmb_regs The DMB registers referred for error logging.
 *
 * @return 0 if the interrupt handler was successfully initialized or an error code.
 */
int hsp_crashdump_hw_err_handler_dmb_err_init (
	struct hsp_crashdump_hw_err_handler_dmb_err *handler,
	const struct hsp_crashdump_handler *crashdump_handler, const struct mmio_register_block *creg,
	size_t creg_dmb_regs, const struct mmio_register_block *dmb_regs)
{
	int status;

	if ((handler == NULL) || (crashdump_handler == NULL) || (creg == NULL) ||
		(dmb_regs == NULL)) {
		return HSP_CRASHDUMP_HW_ERR_HANDLER_DMB_ERR_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct hsp_crashdump_hw_err_handler_dmb_err));

	status = hsp_crashdump_hw_err_handler_init (&handler->base, crashdump_handler, creg,
		hsp_crashdump_hw_err_handler_dmb_err);

	if (status != 0) {
		return status;
	}

	handler->creg_dmb_regs = creg_dmb_regs;
	handler->dmb_regs = dmb_regs;

	return 0;
}

/**
 * Release the resources used for crashdump HSP HW error DMB error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error DMB error interrupt handler to release.
 */
void hsp_crashdump_hw_err_handler_dmb_err_release (
	const struct hsp_crashdump_hw_err_handler_dmb_err *handler)
{
	hsp_crashdump_hw_err_handler_release (&handler->base);
}
