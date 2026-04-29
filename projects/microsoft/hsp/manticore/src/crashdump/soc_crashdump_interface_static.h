// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_CRASHDUMP_INTERFACE_STATIC_H_
#define SOC_CRASHDUMP_INTERFACE_STATIC_H_

#include "soc_crashdump_handler.h"
#include "soc_crashdump_interface.h"


/* Internal functions declared to allow for static initialization. */
int soc_crashdump_interface_trigger_crash_int (const struct soc_crashdump_interface *api);
int soc_crashdump_interface_get_all_core_status (const struct soc_crashdump_interface *api,
	enum soc_crashdump_arm_core_id *failed_core_id, uint32_t *arm_core_status);
int soc_crashdump_interface_set_all_core_status (const struct soc_crashdump_interface *api,
	enum soc_crashdump_arm_core_id *failed_core_id);
int soc_crashdump_interface_clear_core_status (const struct soc_crashdump_interface *api,
	enum soc_crashdump_arm_core_id id);
int soc_crashdump_interface_get_crashdumps_from_cores (
	const struct soc_crashdump_interface *api, const uint8_t *fw_version, bool *available,
	enum soc_crashdump_arm_core_id *failed_core_id);
int soc_crashdump_interface_reset (const struct soc_crashdump_interface *soc_api);

/**
 * Initializer for the crashdump SoC API.
 *
 * There is no validation done on the arguments
 * @param[in] reset_callback Reset callback. Once it is invoked, it would get device into warm reset.
 * @param[in] dmb_ptr The HSP DMB device driver instance to be used as MMU.
 * @param[in] mmio_ptr The interface object used to access registers via MMIO.
 * @param[in] crash_count_ptr Persistent ram space for the crash count.
 *
 */
#define	soc_crashdump_interface_static_init(reset_callback, dmb_ptr, mmio_ptr, crash_count_ptr) { \
		.reset_device = reset_callback, \
		.dmb = dmb_ptr, \
		.mmio = mmio_ptr, \
		.crash_count = crash_count_ptr, \
		.trigger_crash_int = soc_crashdump_interface_trigger_crash_int, \
		.get_all_core_status = soc_crashdump_interface_get_all_core_status, \
		.set_all_core_status = soc_crashdump_interface_set_all_core_status, \
		.clear_core_status = soc_crashdump_interface_clear_core_status, \
		.get_crashdumps_from_cores = soc_crashdump_interface_get_crashdumps_from_cores, \
		.reset = soc_crashdump_interface_reset, \
	}


#endif	/* SOC_CRASHDUMP_INTERFACE_STATIC_H_ */
