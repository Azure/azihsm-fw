// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_crashdump_hw_err_handler.h"
#include "common/unused.h"
#include "crashdump/hsp_crashdump_logging.h"


/**
 * Collect and save the HW error and crashdump information.
 *
 * @param[in] handler The HSP crashdump handler.
 * @param[in] param The HSP trap context.
 * @param[in] fault_code The fault code to indicate the HW module where HW error occurred.
 * @param[in] hw_err_data The HW error data to be stored.
 *
 * @return true if interrupt handled, else false.
 */
bool hsp_crashdump_hw_err_handler_collect_hsp_crashdump (
	const struct hsp_crashdump_handler *handler, uintptr_t param, uint16_t fault_code,
	struct hsp_crashdump_hw_err_handler_crashdump_opaque_data *hw_err_data)
{
	uint8_t packet_buffer[sizeof (struct hsp_crashdump_packet_hsp_production_packet) +
		sizeof (struct hsp_crashdump_hw_err_handler_crashdump_opaque_data)];
	struct hsp_crashdump_packet_hsp_production_packet *packet =
		(struct hsp_crashdump_packet_hsp_production_packet*) packet_buffer;

	if (handler == NULL) {
		return false;
	}

	/* Collect crashdump information, store it on crashdump packet. */
	hsp_crashdump_logging_collect_crashdump (fault_code, CRASH_DUMP_CRASH_TYPE_CRASH,
		(struct hsp_trap_context*) param, handler->fw_version, handler->fw_version_len, packet);

	/* Save HW error data. */
	packet->header.payload_size +=
		sizeof (struct hsp_crashdump_hw_err_handler_crashdump_opaque_data);
	memcpy (&packet_buffer[sizeof (struct hsp_crashdump_packet_hsp_production_packet)],	hw_err_data,
		sizeof (struct hsp_crashdump_hw_err_handler_crashdump_opaque_data));

	/* Save crashdump to persistent ram. */
	hsp_crashdump_logging_save_crashdump_to_persistent_ram (
		(struct hsp_crashdump_packet_hsp_production_packet*) packet_buffer,	handler->persistent_ram,
		handler->persistent_ram_size);

	/* Reset SPRT, 1SP will make warm boot. */
	handler->reset ();

	/* Never get here. */
	return true;
}

/**
 * Initialize crashdump HSP HW error bus error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error interrupt handler to initialize.
 * @param[in] crashdump_handler The HSP crashdump handler.
 * @param[in] creg The The CREG register interface (struct Creg_regs).
 * @param[in] hsp_hw_err_handler The HSP HW error interrupt handler pointer.
 *
 * @return 0 if the interrupt handler was successfully initialized or an error code.
 */
int hsp_crashdump_hw_err_handler_init (
	struct hsp_crashdump_hw_err_handler *handler,
	const struct hsp_crashdump_handler *crashdump_handler, const struct mmio_register_block *creg,
	hsp_interrupt_handler_func hsp_hw_err_handler)
{
	if ((handler == NULL) || (crashdump_handler == NULL) || (creg == NULL) ||
		(hsp_hw_err_handler == NULL)) {
		return HSP_CRASHDUMP_HW_ERR_HANDLER_ERR_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct hsp_crashdump_hw_err_handler));

	handler->crashdump_handler = crashdump_handler;
	handler->creg = creg;
	handler->base.handle_interrupt = hsp_hw_err_handler;

	return 0;
}

/**
 * Release the resources used for crashdump HSP HW error bus error interrupt handler.
 *
 * @param[in] handler The crashdump HSP HW error bus error interrupt handler to release.
 */
void hsp_crashdump_hw_err_handler_release (
	const struct hsp_crashdump_hw_err_handler *handler)
{
	UNUSED (handler);
}
