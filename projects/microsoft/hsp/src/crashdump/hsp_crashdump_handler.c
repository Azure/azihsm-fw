// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_crashdump_handler.h"
#include "hsp_crashdump_logging.h"
#include "common/type_cast.h"
#include "common/unused.h"


/**
 * HSP interrupt handler that collects the crashdump, saves it into persistent RAM,
 * and then makes a warm reset.
 *
 * @param[in] handler The interface for handling HSP interrupts.
 * @param[in] param The HSP trap context.
 *
 * @return true if interrupt handled, else false.
 */
bool hsp_crashdump_handler_handle_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	const struct hsp_crashdump_handler *crashdump_handler = TO_DERIVED_TYPE (handler,
		const struct hsp_crashdump_handler, base);

	if (handler == NULL) {
		return false;
	}

	hsp_crashdump_handler_collect_hsp_crashdump (crashdump_handler,
		(struct hsp_trap_context*) param, HSP_CRASHDUMP_PACKET_FAULT_CODE_PANIC,
		CRASH_DUMP_CRASH_TYPE_CRASH);

	/* Reset SPRT, 1SP will make warm boot. */
	crashdump_handler->reset ();

	/* Never get here. */
	return true;
}

/**
 * Initialize crashdump HSP handler.
 *
 * @param[in] handler The crashdump HSP handler to initialize.
 * @param[in] fw_version The build version number for the firmware package.
 * @param[in] fw_version_len The length of build version number for the firmware package.
 * @param[in] reset Reset callback function. Once it is invoked, it would get device into warm reset.
 * @param[in] persistent_ram The persistent RAM, such as sticky registers, used to store crashdump.
 * @param[in] persistent_ram_size The size of persistent RAM used to store crashdump.
 *
 * @return 0 if the interrupt handler was successfully initialized or an error code.
 */
int hsp_crashdump_handler_init (struct hsp_crashdump_handler *handler, const uint8_t *fw_version,
	size_t fw_version_len, hsp_crashdump_handler_reset_callback reset, uint32_t *persistent_ram,
	size_t persistent_ram_size)
{
	if ((handler == NULL) || (fw_version == NULL) || (fw_version_len == 0) ||
		(reset == NULL) || (persistent_ram == NULL) || (persistent_ram_size == 0)) {
		return HSP_CRASHDUMP_HANDLER_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct hsp_crashdump_handler));

	handler->base.handle_interrupt = hsp_crashdump_handler_handle_interrupt;
	handler->fw_version = fw_version;
	handler->fw_version_len = fw_version_len;
	handler->reset = reset;
	handler->persistent_ram = persistent_ram;
	handler->persistent_ram_size = persistent_ram_size;

	return 0;
}

/**
 * Release the resources used for crashdump HSP handler.
 *
 * @param[in] handler The crashdump handler to release.
 */
void hsp_crashdump_handler_release (const struct hsp_crashdump_handler *handler)
{
	UNUSED (handler);
}

/**
 * Collect the crashdump details from the provided context and save it to persistent RAM.
 *
 * @param handler The crashdump handler.  If this is null, nothing is done.
 * @param context Trap context containing the details to include a crashdump.  If this is null,
 * nothing is done.
 * @param fault_code The type of fault that occurred.
 * @param crash_type The type of crash.
 */
void hsp_crashdump_handler_collect_hsp_crashdump (const struct hsp_crashdump_handler *handler,
	struct hsp_trap_context *context, uint16_t fault_code, uint8_t crash_type)
{
	struct hsp_crashdump_packet_hsp_production_packet packet;

	if ((handler == NULL) || (context == NULL)) {
		return;
	}

	/* Collect crashdump information, store it on crashdump packet. */
	hsp_crashdump_logging_collect_crashdump (fault_code, crash_type, context, handler->fw_version,
		handler->fw_version_len, &packet);

	/* Save crashdump to persistent ram. */
	hsp_crashdump_logging_save_crashdump_to_persistent_ram (&packet, handler->persistent_ram,
		handler->persistent_ram_size);
}
