// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "cmd_device_hsp.h"
#include "hsp_top.h"
#include "platform_config.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "drivers/hsp_fuses.h"
#include "splibs/inc/spcryptotypes.h"


int cmd_device_hsp_get_uuid (const struct cmd_device *device, uint8_t *buffer, size_t buf_len)
{
	const struct cmd_device_hsp *hsp = (const struct cmd_device_hsp*) device;
	uint32_t dword_buffer[IN_DWORDS (HSP_FUSES_LENGTH (SOCID))];
	size_t i;

	if ((hsp == NULL) || (buffer == NULL)) {
		return CMD_DEVICE_INVALID_ARGUMENT;
	}

	if (HSP_FUSES_LENGTH (SOCID) > buf_len) {
		return CMD_DEVICE_UUID_BUFFER_TOO_SMALL;
	}

	for (i = 0; i < IN_DWORDS (HSP_FUSES_LENGTH (SOCID)); i++) {
		/* Need to copy to an intermediate buffer to ensure memory alignment and dword accesses. */
		dword_buffer[i] = hsp->fuse_regs->SOCID.SOCID[i];
	}

	memcpy (buffer, (uint8_t*) dword_buffer, HSP_FUSES_LENGTH (SOCID));

	return HSP_FUSES_LENGTH (SOCID);
}

int cmd_device_hsp_reset (const struct cmd_device *device)
{
	const struct cmd_device_hsp *hsp = (const struct cmd_device_hsp*) device;
	const uint32_t spin_time = ((HSP_CLOCK_FREQUENCY_HZ / 1000) * 25);
	volatile uint32_t spin_count = 0;

	if (hsp == NULL) {
		return CMD_DEVICE_INVALID_ARGUMENT;
	}

	hsp->sw_regs->HSP_FATAL_SW_ERR = 1;

	/* Setting this bit does not immediately trigger a CPU reset.  Additional instructions will
	 * execute, resulting in this function returning a failure.  Add a delay here to give the HW
	 * time to trigger the reset before reporting a failure.
	 *
	 * Spin to avoid any dependencies on the calling context and not yield the CPU with a reset
	 * pending. */
	while (spin_count < spin_time) {
		spin_count++;
	}

	return CMD_DEVICE_RESET_FAILED;
}

int cmd_device_hsp_get_reset_counter (const struct cmd_device *device, uint8_t type, uint8_t port,
	uint16_t *counter)
{
	const struct cmd_device_hsp *hsp = (const struct cmd_device_hsp*) device;
	int status;

	if ((hsp == NULL) || (counter == NULL)) {
		return CMD_DEVICE_INVALID_ARGUMENT;
	}

	if ((type > 1) || (port > 1)) {
		return CMD_DEVICE_INVALID_COUNTER;
	}

	status = counter_manager_registers_get_counter (hsp->counter, type, port);
	if (!ROT_IS_ERROR (status)) {
		buffer_unaligned_write16 (counter, (uint16_t) status);
		status = 0;
	}

	return status;
}

int cmd_device_hsp_get_heap_stats (const struct cmd_device *device,
	struct cmd_device_heap_stats *heap)
{
	if ((device == NULL) || (heap == NULL)) {
		return CMD_DEVICE_INVALID_ARGUMENT;
	}

	/* The default implementation provides no heap statistics. */
	memset (heap, 0xff, sizeof (struct cmd_device_heap_stats));

	return 0;
}

/**
 * Initialize a device command handler for HSP.  The API for retrieving heap statistics will not be
 * initialized.
 *
 * @param device The handler to initialize.
 * @param counter The reset counter manager for the device.
 * @param fuse_regs Register interface to the fuse controller.
 * @param sw_regs Register interface for software management registers, including HSP reset.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int cmd_device_hsp_init_no_heap_stats (struct cmd_device_hsp *device,
	struct counter_manager_registers *counter, const struct Gfc_regs *fuse_regs,
	struct Creg_regs_misc_creg_sw_regs *sw_regs)
{
	if ((device == NULL) || (counter == NULL) || (fuse_regs == NULL)) {
		return CMD_DEVICE_INVALID_ARGUMENT;
	}

	memset (device, 0, sizeof (struct cmd_device_hsp));

	device->base.get_uuid = cmd_device_hsp_get_uuid;
	device->base.reset = cmd_device_hsp_reset;
	device->base.get_reset_counter = cmd_device_hsp_get_reset_counter;

	device->counter = counter;
	device->fuse_regs = fuse_regs;
	device->sw_regs = sw_regs;

	return 0;
}

/**
 * Initialize a device command handler for HSP.
 *
 * @param device The handler to initialize.
 * @param counter The reset counter manager for the device.
 * @param fuse_regs Register interface to the fuse controller.
 * @param sw_regs Register interface for software management registers, including HSP reset.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int cmd_device_hsp_init (struct cmd_device_hsp *device,	struct counter_manager_registers *counter,
	const struct Gfc_regs *fuse_regs, struct Creg_regs_misc_creg_sw_regs *sw_regs)
{
	int status;

	status = cmd_device_hsp_init_no_heap_stats (device, counter, fuse_regs, sw_regs);

#ifdef CMD_ENABLE_HEAP_STATS
	if (status == 0) {
		device->base.get_heap_stats = cmd_device_hsp_get_heap_stats;
	}
#endif

	return status;
}

/**
 * Release an HSP device command handler.
 *
 * @param device The handler to release.
 */
void cmd_device_hsp_release (const struct cmd_device_hsp *device)
{
	UNUSED (device);
}
