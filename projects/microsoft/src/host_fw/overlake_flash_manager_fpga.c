// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "flash/flash_updater.h"
#include "host_fw/host_flash_manager.h"
#include "host_fw/overlake_flash_manager.h"
#include "host_fw/overlake_flash_manager_fpga.h"
#include "host_fw/overlake_host_id.h"


/**
 * Flash layout for Cyclone V FPGA
 */
#define OVERLAKE_FLASH_C5_BOOT_ADDR						0x0000000
#define OVERLAKE_FLASH_C5_BOOT_MAX_LENGTH				0x0800000
#define OVERLAKE_FLASH_C5_BOOT_BACKUP_ADDR				0x0800000

#define OVERLAKE_FLASH_C5_BOOT_MODE_ADDR				0x1000000
#define OVERLAKE_FLASH_C5_RESERVED_MAX_LENGTH			0x2800000

/**
 * Glacier Peak FPGA Platform ID
 */
static const char fpga_platform_id[] = "A2051-GPMC";


static int overlake_flash_manager_fpga_get_port_id (const struct host_firmware *firmware)
{
	if (firmware == NULL) {
		return -1;
	}

	return OVERLAKE_HOST_PORT_FPGA_C5;
}

static int overlake_flash_manager_fpga_set_boot_mode (struct overlake_flash_manager *manager,
	uint8_t mode)
{
	struct overlake_flash_manager_fpga *mgr = (struct overlake_flash_manager_fpga*) manager;
	int status;
	uint8_t boot_mode;

	if (mgr == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&mgr->base.lock);

	status = overlake_flash_manager_set_boot_mode (&mgr->base, OVERLAKE_FLASH_C5_BOOT_MODE_ADDR,
		mode, true);

	boot_mode = (status == 0) ? mode : OVERLAKE_FPGA_BOOT_MODE_INVALID;
	overlake_flash_manager_update_boot_mode_cache (manager, 0, boot_mode);

	platform_mutex_unlock (&mgr->base.lock);

	return status;
}

static int overlake_flash_manager_fpga_get_boot_mode (struct overlake_flash_manager *manager,
	uint8_t *mode)
{
	if ((manager == NULL) || (mode == NULL)) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if (manager->board_type == OVERLAKE_CASTLE_PEAK) {
		return OVERLAKE_FLASH_MGR_UNSUPPORTED;
	}

	platform_mutex_lock (&manager->lock);
	overlake_flash_manager_get_boot_mode_cache (manager, 0, mode);
	platform_mutex_unlock (&manager->lock);

	return 0;
}

static int overlake_flash_manager_fpga_set_flash_for_rot_access (
	struct overlake_flash_manager *manager)
{
	struct overlake_flash_manager_fpga *mgr = (struct overlake_flash_manager_fpga*) manager;
	int status;
	uint8_t mode;

	if (mgr == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&mgr->base.lock);

	status = overlake_flash_manager_enable_rot_flash_access (manager);
	if (status == 0) {
		status = overlake_flash_manager_get_boot_mode (&mgr->base, OVERLAKE_FLASH_C5_BOOT_MODE_ADDR,
			&mode);

		mode = (status == 0) ? mode : OVERLAKE_FPGA_BOOT_MODE_INVALID;
		overlake_flash_manager_update_boot_mode_cache (manager, 0, mode);
	}

	platform_mutex_unlock (&mgr->base.lock);

	return status;
}

static int overlake_flash_manager_fpga_validate_flash (const struct host_firmware *firmware,
	const struct pfm *pfm, const struct pfm *good_pfm, const struct hash_engine *hash,
	const struct rsa_engine *rsa)
{
	struct overlake_soc_firmware *fw = (struct overlake_soc_firmware*) firmware;
	int status;

	status = overlake_flash_manager_validate_flash (&fw->base, pfm, good_pfm, hash, rsa);
	if (status != 0) {
		return status;
	}

	status = fw->base.backup_flash (&fw->base);

	return status;
}

/**
 * Initialize management of the Overlake FPGA flash device.
 *
 * @param manager The flash manager to initialize.
 * @param flash The SoC flash device.
 * @param control The interface for controlling access to host flash.
 * @param host_state_boot Manager for the boot image flash state.
 * @param board_id The Overlake board ID.
 * @param is_por Boolean value indicating if the source of cerberus reset is power on reset.
 *
 * @return 0 if the manager was successfully initialized or an error code.
 */
int overlake_flash_manager_fpga_init (struct overlake_flash_manager_fpga *manager,
	struct spi_flash *flash, const struct host_control *control,
	const struct host_state_manager *host_state_boot, enum overlake_board_id board_id,
	volatile uint32_t *boot_mode_cache_reg, bool is_por)
{
	int status;
	uint32_t boot_addr;
	uint32_t boot_backup_addr;
	uint32_t boot_max_length;

	if ((manager == NULL) || (flash == NULL) || (control == NULL) || (host_state_boot == NULL)) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if ((board_id < OVERLAKE_BOARD_ID_GLACIER_PEAK_POC) ||
		(board_id >= OVERLAKE_BOARD_ID_CELESTIAL_PEAK)) {
		return OVERLAKE_FLASH_MGR_UNKNOWN_BOARD_ID;
	}

	boot_addr = OVERLAKE_FLASH_C5_BOOT_ADDR;
	boot_backup_addr = OVERLAKE_FLASH_C5_BOOT_BACKUP_ADDR;
	boot_max_length = OVERLAKE_FLASH_C5_BOOT_MAX_LENGTH;

	status = overlake_flash_manager_init_internal (&manager->base, flash, control, NULL,
		host_state_boot, NULL, boot_addr, boot_backup_addr, 0, boot_max_length, fpga_platform_id, 0,
		0, 0, 0, NULL, overlake_get_board_type (board_id));
	if (status != 0) {
		return status;
	}

	manager->base.boot.base.get_port_id = overlake_flash_manager_fpga_get_port_id;
	manager->base.boot.base.validate_flash = overlake_flash_manager_fpga_validate_flash;
	manager->base.set_soc_boot_mode = overlake_flash_manager_fpga_set_boot_mode;
	manager->base.get_soc_boot_mode = overlake_flash_manager_fpga_get_boot_mode;
	manager->base.set_flash_for_rot_access = overlake_flash_manager_fpga_set_flash_for_rot_access;

	manager->base.boot_mode_cache = boot_mode_cache_reg;

	if (is_por) {
		overlake_flash_manager_update_boot_mode_cache (&manager->base, 0,
			OVERLAKE_FPGA_BOOT_MODE_INVALID);
	}

	return 0;
}

/**
 * Initialize management of the Overlake FPGA flash device.  The interface to the flash may be
 * uninitialized, but an initialization manager is provided to ensure it is initialized prior to
 * use.
 *
 * @param manager The flash manager to initialize.
 * @param flash The FPGA flash device.
 * @param control The interface for controlling access to fpga flash.
 * @param host_state_boot Manager for the cyclone V fpga image flash state.
 * @param flash_init The flash initialization manager.
 * @param board_id The Overlake board ID.
 * @param boot_mode_cache_reg The register to hold FPGA boot mode.
 * @param is_por Boolean value indicating if the source of cerberus reset is power on reset.
 *
 * @return 0 if the manager was successfully initialized or an error code.
 */
int overlake_flash_manager_fpga_init_with_managed_flash_initialization (
	struct overlake_flash_manager_fpga *manager, struct spi_flash *flash,
	const struct host_control *control, const struct host_state_manager *host_state_boot,
	const struct host_flash_initialization *flash_init, enum overlake_board_id board_id,
	volatile uint32_t *boot_mode_cache_reg, bool is_por)
{
	int status;

	if (flash_init == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	status = overlake_flash_manager_fpga_init (manager, flash, control, host_state_boot, board_id,
		boot_mode_cache_reg, is_por);
	if (status != 0) {
		return status;
	}

	manager->base.flash_init = flash_init;

	return 0;
}

/**
 * Release a manager the Overlake FPGA flash.
 *
 * @param manager The manager to release.
 */
void overlake_flash_manager_fpga_release (struct overlake_flash_manager_fpga *manager)
{
	if (manager) {
		flash_updater_release (&manager->base.boot.update);
		platform_mutex_free (&manager->base.lock);
	}
}
