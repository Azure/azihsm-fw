// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "soc_crashdump_handler.h"
#include "soc_crashdump_interface.h"
#include "common/unused.h"
#include "crashdump/hsp_crashdump_logging.h"
#include "dc_scm/cp_fp_dtcm_memory.h"
#include "logging/crash_dump_logging.h"
#include "logging/debug_log.h"
#include "logging/manticore_logging.h"
#if SOC_CRASHDUMP_HANDLER_PRINTF
#include "platform_io_api.h"
#endif


/* TODO: CP crashdump dTCM addresses will be replaced by GSRAM locations
 * whenever CP code relocates them to GSRAM. */

/**
 * ARM core global addresses.
 */
static const uint32_t soc_crashdump_interface_status_addr[] = {
	CPU_CP0_DTCM_STATUS_GLOBAL_ADDR,
	CPU_CP1_DTCM_STATUS_GLOBAL_ADDR,
	CPU_FP0_DTCM_STATUS_GLOBAL_ADDR,
	CPU_FP1_DTCM_STATUS_GLOBAL_ADDR,
	CPU_FP2_DTCM_STATUS_GLOBAL_ADDR
};

static const uint32_t soc_crashdump_interface_crashdump_addr[] = {
	CPU_CP0_DTCM_CRASHDUMP_GLOBAL_START,
	CPU_CP1_DTCM_CRASHDUMP_GLOBAL_START,
	CPU_FP0_DTCM_CRASHDUMP_GLOBAL_START,
	CPU_FP1_DTCM_CRASHDUMP_GLOBAL_START,
	CPU_FP2_DTCM_CRASHDUMP_GLOBAL_START
};


int soc_crashdump_interface_trigger_crash_int (const struct soc_crashdump_interface *api)
{
	int status;

	status = api->mmio->map (api->mmio);
	if (status != 0) {
		return status;
	}

	status = api->mmio->write32 (api->mmio, offsetof (Tcon_t, wakeup1Cnt), 1);
	if (status != 0) {
		goto exit;
	}

	status = mmio_register_block_write_bits (api->mmio, offsetof (Tcon_t, wakeupCtrl),
		WAKEUP_CTR1_WAKEUP_ENABLE_BIT, WAKEUP_CTR1_WAKEUP_ENABLE_BIT_COUNT, 0x01 << 1);
	if (status != 0) {
		goto exit;
	}

	status = mmio_register_block_write_bits (api->mmio, offsetof (Tcon_t, wakeupCtrl),
		WAKEUP_CTR1_WKINTR_LEVEL_EN_BIT, WAKEUP_CTR1_WKINTR_LEVEL_EN_BIT_COUNT, 0x01 << 1);

exit:
	api->mmio->unmap (api->mmio);

	return status;
}

static int soc_crashdump_interface_write_soc_memory (const struct soc_crashdump_interface *api,
	uint32_t soc_address, void *buffer, size_t size)
{
	int status;
	uint32_t *mapped_address;

	status = api->dmb->map_soc_address (api->dmb, soc_address, size, HSP_DMB_ACCESS_WRITE,
		(void**) &mapped_address);
	if (status == 0) {
		memcpy (mapped_address, buffer, size);
		api->dmb->unmap_soc_address (api->dmb, mapped_address);
	}

	return status;
}

static int soc_crashdump_interface_read_soc_memory (const struct soc_crashdump_interface *api,
	uint32_t soc_address, void *buffer, size_t size)
{
	int status;
	uint32_t *mapped_address;

	status = api->dmb->map_soc_address (api->dmb, soc_address, size, 0, (void**) &mapped_address);
	if (status == 0) {
		memcpy (buffer, mapped_address, size);
		api->dmb->unmap_soc_address (api->dmb, mapped_address);
	}

	return status;
}

int soc_crashdump_interface_get_all_core_status (const struct soc_crashdump_interface *api,
	enum soc_crashdump_arm_core_id *failed_core_id, uint32_t *arm_core_status)
{
	int status;
	enum soc_crashdump_arm_core_id id;

	for (id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID; id < SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS;
		id++) {
		status = soc_crashdump_interface_read_soc_memory (api,
			soc_crashdump_interface_status_addr[id], &arm_core_status[id],
			sizeof (arm_core_status[id]));
		if (status != 0) {
			*failed_core_id = id;

			return status;
		}
	}

	return status;
}

int soc_crashdump_interface_set_all_core_status (const struct soc_crashdump_interface *api,
	enum soc_crashdump_arm_core_id *failed_core_id)
{
	int status;
	uint32_t core_status = 1;
	enum soc_crashdump_arm_core_id id;

	for (id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID; id < SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS;
		id++) {
		status = soc_crashdump_interface_write_soc_memory (api,
			soc_crashdump_interface_status_addr[id], &core_status, sizeof (core_status));
		if (status != 0) {
			*failed_core_id = id;

			return status;
		}
	}

	return status;
}

int soc_crashdump_interface_clear_core_status (const struct soc_crashdump_interface *api,
	enum soc_crashdump_arm_core_id id)
{
	int status;
	uint32_t core_status = 0;

	status = soc_crashdump_interface_write_soc_memory (api, soc_crashdump_interface_status_addr[id],
		&core_status, sizeof (core_status));

	return status;
}

static void soc_crashdump_interface_save_crashdump_to_debug_log (
	struct soc_crashdump_packet_arm_production_packet *pkt, const uint8_t *fw_version)
{
	size_t length = sizeof (struct crash_dump_packet_header) + pkt->header.payload_size;
	size_t dev_length;
	uint32_t *dev_data;

	/* Save crashdump header to debug log. */
	hsp_crashdump_logging_save_crashdump_header_to_debug_log (&pkt->header,
		(const uint32_t*) fw_version);

	/* Save portion of payload defined for Cerberus Open Source to debug log. */
	crash_dump_arm_log (&pkt->payload.common_regs, false);

	/* Save debug/development data to debug log. */
	if (length > sizeof (struct soc_crashdump_packet_arm_production_packet)) {
		dev_length = length - sizeof (struct soc_crashdump_packet_arm_production_packet);
		dev_data = (uint32_t*) ((uint8_t*) pkt +
			sizeof (struct soc_crashdump_packet_arm_production_packet));
		crash_dump_logging_save_opaque_data (dev_data, dev_length);
	}

	/* Flush debug logs. */
	debug_log_flush ();
}

int soc_crashdump_interface_get_crashdumps_from_cores (
	const struct soc_crashdump_interface *api, const uint8_t *fw_version, bool *available,
	enum soc_crashdump_arm_core_id *failed_core_id)
{
	int status;
	enum soc_crashdump_arm_core_id id;
	struct soc_crashdump_packet_arm_production_packet *pkt;
	uint16_t local_payload_size = 0;
	uint32_t wait_times;

	/* Collect crashdumps from all ARM cores if they are available. */
	for (id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID; id < SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS;
		id++) {
		status = api->dmb->map_soc_address (api->dmb, soc_crashdump_interface_crashdump_addr[id],
			sizeof (struct crash_dump_packet_header), 0, (void**) &pkt);

		if (status != 0) {
			*failed_core_id = id;

			return status;
		}

		/* Pulling the cash dump header to see if the crashdump is available or not. */
		for (wait_times = 0; wait_times < SOC_CRASHDUMP_INTERFACE_NUM_TIMES_PULLING;
			wait_times++) {
			available[id] = (pkt->header.magic == HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_COMITTED);
			if (available[id] == true) {
				local_payload_size = pkt->header.payload_size;
				break;
			}
		}

		api->dmb->unmap_soc_address (api->dmb, pkt);

		/* Crashdump is available. */
		if ((available[id] == true) && (fw_version != NULL)) {
			/* Get crashdump packet pointer. */
			status = api->dmb->map_soc_address (api->dmb,
				soc_crashdump_interface_crashdump_addr[id],
				sizeof (struct crash_dump_packet_header) + local_payload_size, HSP_DMB_ACCESS_WRITE,
				(void**) &pkt);

			if (status != 0) {
				*failed_core_id = id;

				return status;
			}

			/* Crashdump version crossing crashdumps of cores on the same crash
			* should be consistent. */
			pkt->header.crashdump_version = HSP_CRASHDUMP_VERSION;

			/* Save crashdump into debug log. */
			soc_crashdump_interface_save_crashdump_to_debug_log ((void*) pkt, fw_version);

			/* Set the magic # to dirty on ARM side to make the crashdump invalid. */
			pkt->header.magic = HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_DIRTY;

			api->dmb->unmap_soc_address (api->dmb, pkt);
		}
	}

	return status;
}

/**
 * Wrapper function to get reset_device callback function called.
 *
 * @param[in] api The SoC interface instance.
 *
 * @return 0 if reset succeeded or an error code.
 */
int soc_crashdump_interface_reset (const struct soc_crashdump_interface *soc_api)
{
	if ((!soc_api) || (!soc_api->reset_device)) {
		return SOC_CRASHDUMP_INTERFACE_INVALID_ARGUMENT;
	}

	soc_api->reset_device ();

	return 0;
}

/**
 * Initialize crashdump SoC API.
 *
 * @param[in] soc_api The SoC interface instance.
 * @param[in] dmb The HSP DMB device driver instance to be used as MMU and allow access
 * to SoC memory space.
 * @param[in] mmio The interface object used to access registers via MMIO.
 * @param[in] crash_count The crash count.
 * @param[in] reset_callback Reset callback. Once it is invoked, it would get device into warm reset.
 *
 * @return 0 if the crashdump SoC API  was successfully initialized or an error code.
 */
int soc_crashdump_interface_init (struct soc_crashdump_interface *soc_api,
	const struct hsp_dmb *dmb, const struct mmio_register_block *mmio, uint32_t *crash_count,
	hsp_crashdump_handler_reset_callback reset_callback)
{
	if ((soc_api == NULL) || (dmb == NULL) || (mmio == NULL) ||
		(crash_count == NULL) || (reset_callback == NULL)) {
		return SOC_CRASHDUMP_INTERFACE_INVALID_ARGUMENT;
	}

	memset (soc_api, 0, sizeof (struct soc_crashdump_interface));

	soc_api->dmb = dmb;
	soc_api->mmio = mmio;
	soc_api->crash_count = crash_count;
	soc_api->reset = soc_crashdump_interface_reset;
	soc_api->trigger_crash_int = soc_crashdump_interface_trigger_crash_int;
	soc_api->get_all_core_status = soc_crashdump_interface_get_all_core_status;
	soc_api->set_all_core_status = soc_crashdump_interface_set_all_core_status;
	soc_api->clear_core_status = soc_crashdump_interface_clear_core_status;
	soc_api->get_crashdumps_from_cores = soc_crashdump_interface_get_crashdumps_from_cores;
	soc_api->reset_device = reset_callback;

	return 0;
}

/**
 * Release the resources used for soc crashdump interface.
 *
 * @param[in] soc_api The soc crashdump interface.
 * @param[in] dmb The HSP DMB device driver instance to be used as MMU and allow access
 * to SoC memory space.
 */
void soc_crashdump_interface_release (const struct soc_crashdump_interface *soc_api,
	const struct hsp_dmb *dmb)
{
	UNUSED (soc_api);
	UNUSED (dmb);
}
