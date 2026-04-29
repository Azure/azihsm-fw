// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_crashdump_hw_err_handler_mpu_err.h"
#include "common/type_cast.h"
#include "crashdump/hsp_crashdump_packet.h"
#include "logging/debug_log.h"
#include "logging/hsp_logging.h"


bool hsp_crashdump_hw_err_handler_mpu_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	const struct hsp_crashdump_hw_err_handler_mpu_err *mpu_err_handler =
		TO_DERIVED_TYPE (TO_DERIVED_TYPE (handler, const struct hsp_crashdump_hw_err_handler, base),
		const struct hsp_crashdump_hw_err_handler_mpu_err, base);
	int status;
	bool known_status0_err = false;
	uint32_t intsts_value = 0;
	uint32_t err_reg_offset[HSP_CRASHDUMP_HW_ERR_HANDLER_MPU_ERR_NUM_ERR_REGS] = {
		mpu_err_handler->mpu_dram_regs + CREG_REGS_SPDRAM_MPU_REGS_SPDRAM_MPU_STATUS_OFFSET,
		mpu_err_handler->mpu_iram_regs + CREG_REGS_SPIRAM_MPU_REGS_SPIRAM_MPU_STATUS_OFFSET,
		mpu_err_handler->mpu_rom_regs + CREG_REGS_SPROM_MPU_REGS_SPROM_MPU_STATUS_OFFSET
	};
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data hw_err_data =
	{0, HSP_LOGGING_HW_ERROR_MPU_SPDRAM_MPU_STATUS_TAG, 0, 0};

	status = mpu_err_handler->base.creg->map (mpu_err_handler->base.creg);
	if (status != 0) {
		goto skip_unmap;
	}

	/* Get interrupt status. */
	status = mpu_err_handler->base.creg->read32 (mpu_err_handler->base.creg,
		mpu_err_handler->mpu_regs + CREG_REGS_MPU_CREG_MPU_REGS_MPU_INTSTS_OFFSET, &intsts_value);
	if (status != 0) {
		goto failed;
	}

	if (!intsts_value) {
		status = HSP_CRASHDUMP_HW_ERR_HANDLER_MPU_ERR_INT_STATUS_0;
		goto failed;
	}

	hw_err_data.int_status = (uint16_t) (intsts_value & 0xFFFF);

	/* Get the location where the error occurred. */
	if (intsts_value & CREG_REGS_MPU_CREG_MPU_REGS_MPU_INTSTS_SPIRAM_ACC_VIO_FIELD_MASK) {
		hw_err_data.err_tag = HSP_LOGGING_HW_ERROR_MPU_SPIRAM_MPU_STATUS_TAG;
		known_status0_err = true;
	}
	else if (intsts_value & CREG_REGS_MPU_CREG_MPU_REGS_MPU_INTSTS_SPDRAM_ACC_VIO_FIELD_MASK) {
		hw_err_data.err_tag = HSP_LOGGING_HW_ERROR_MPU_SPDRAM_MPU_STATUS_TAG;
		known_status0_err = true;
	}
	else if (intsts_value & CREG_REGS_MPU_CREG_MPU_REGS_MPU_INTSTS_SPROM_ACC_VIO_FIELD_MASK) {
		hw_err_data.err_tag = HSP_LOGGING_HW_ERROR_MPU_SPROM_MPU_STATUS_TAG;
		known_status0_err = true;
	}

	if (known_status0_err) {
		status = mpu_err_handler->base.creg->read32 (mpu_err_handler->base.creg,
			err_reg_offset[hw_err_data.err_tag - 1], &hw_err_data.status0);
		if (status != 0) {
			goto failed;
		}
	}

	/* Clear the pending interrupt from MPU module. */
	status = mpu_err_handler->base.creg->write32 (mpu_err_handler->base.creg,
		mpu_err_handler->mpu_regs + CREG_REGS_MPU_CREG_MPU_REGS_MPU_INTSTS_OFFSET,
		CREG_REGS_MPU_CREG_MPU_REGS_MPU_INTSTS_WRITE_MASK);

failed:
	mpu_err_handler->base.creg->unmap (mpu_err_handler->base.creg);

skip_unmap:
	if (status != 0) {
		/* Return false to cease the interrupt dispatch. */
		return false;
	}

	return hsp_crashdump_hw_err_handler_collect_hsp_crashdump (
		mpu_err_handler->base.crashdump_handler, param, HSP_CRASHDUMP_PACKET_FAULT_CODE_MPU_ERR,
		&hw_err_data);
}

/**
 * Save HW error opaque data to debug log entries.
 *
 * @param[in] buffer The buffer where crashdump opaque data is stored.
 * @param[in] length The number bytes of crashdump opaque data.
 *
 */
void hsp_crashdump_hw_err_handler_mpu_err_save_opaque_data (uint32_t *buffer, size_t length)
{
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data *hw_err_data =
		(struct hsp_crashdump_hw_err_handler_crashdump_opaque_data*) buffer;

	/* TODO: Any additional opaque data in the crash dump packet will not get logged. */
	if ((hw_err_data != NULL) &&
		(length >= sizeof (struct hsp_crashdump_hw_err_handler_crashdump_opaque_data))) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_MPU, HSP_LOGGING_HW_ERROR_MPU_INTSTS_TAG, hw_err_data->int_status);
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_HW_ERROR_MPU, hw_err_data->err_tag, hw_err_data->status0);
	}
}

/**
 * Initialize crashdump HSP HW error MPU error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error MPU error interrupt handler to initialize.
 * @param[in] crashdump_handler The HSP crashdump handler.
 * @param[in] creg The The CREG register interface (struct Creg_regs).
 * @param[in] mpu_regs The CREG offset for the hardware MPU registers.
 * @param[in] mpu_dram_regs The CREG offset for the hardware MPU SPDRAM registers..
 * @param[in] mpu_iram_regs The CREG offset for the hardware MPU SPIRAM registers.
 * @param[in] mpu_rom_regs The CREG offset for the hardware MPU SPROM registers.
 *
 * @return 0 if the interrupt handler was successfully initialized or an error code.
 */
int hsp_crashdump_hw_err_handler_mpu_err_init (
	struct hsp_crashdump_hw_err_handler_mpu_err *handler,
	const struct hsp_crashdump_handler *crashdump_handler, const struct mmio_register_block *creg,
	size_t mpu_regs, size_t mpu_dram_regs, size_t mpu_iram_regs, size_t mpu_rom_regs)
{
	int status;

	if ((handler == NULL) || (crashdump_handler == NULL) || (creg == NULL)) {
		return HSP_CRASHDUMP_HW_ERR_HANDLER_MPU_ERR_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct hsp_crashdump_hw_err_handler_mpu_err));

	status = hsp_crashdump_hw_err_handler_init (&handler->base, crashdump_handler, creg,
		hsp_crashdump_hw_err_handler_mpu_err);

	if (status != 0) {
		return status;
	}

	handler->mpu_regs = mpu_regs;
	handler->mpu_dram_regs = mpu_dram_regs;
	handler->mpu_iram_regs = mpu_iram_regs;
	handler->mpu_rom_regs = mpu_rom_regs;

	return 0;
}

/**
 * Release the resources used for crashdump HSP HW error MPU error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error MPU error interrupt handler to release.
 */
void hsp_crashdump_hw_err_handler_mpu_err_release (
	const struct hsp_crashdump_hw_err_handler_mpu_err *handler)
{
	hsp_crashdump_hw_err_handler_release (&handler->base);
}
