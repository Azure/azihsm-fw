// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/unused.h"
#include "flash/flash_util.h"
#include "host_fw/host_flash_manager.h"
#include "host_fw/host_logging.h"
#include "host_fw/omc_flash_manager.h"
#include "host_fw/overlake_host_id.h"
#include "host_fw/overlake_logging.h"


#define OMC_CAP_FLASH_BOOT_ADDR					0x000000
#define OMC_CAP_FLASH_BOOT_MAX_LENGTH			0x4000000
#define OMC_CAP_FLASH_MAX_LENGTH				0x10000000
#define OMC_CAP_FLASH_IMAGE_PARTITIONS_LENGTH	0xF440000

/**
 * CastlePeak SOC Platform ID
 */
static const char overlake_platform_id[] = "A2065";


/**
 * Enable RoT access to the SoC flash.
 *
 * @param manager The manager for the flash to access.
 *
 * @return 0 if flash access has been enabled or an error code.
 */
int omc_flash_manager_enable_rot_flash_access (struct omc_flash_manager *manager)
{
	int status;

	status = manager->control->enable_processor_flash_access (manager->control, false);
	if (status != 0) {
		return status;
	}

	if (manager->soc_control != NULL) {
		/* PMC can fail to revert flash back to extended SPI I/O mode after booting Tahoe in
		 * octal mode. This causes OMC to read incorrect data even after pulling down the ospi
		 * mux and fail to initialize or program the device. If enabling OMC flash access,
		 * need to reset spi configuration to extended spi I/O mode required for OMC to
		 * communicate with flash. */
		status = manager->soc_control->force_flash_reset (manager->soc_control);
		if (status != 0) {
			return status;
		}
	}

	if (manager->flash_init) {
		status = host_flash_initialization_initialize_flash (manager->flash_init);
		if (status != 0) {
			return status;
		}
	}

	return host_flash_manager_configure_flash_for_rot_access (manager->flash);
}

static int omc_flash_manager_set_flash_for_rot_access (struct omc_flash_manager *manager)
{
	int status;

	if (manager == NULL) {
		return OMC_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);
	status = omc_flash_manager_enable_rot_flash_access (manager);
	platform_mutex_unlock (&manager->lock);

	return status;
}

static int omc_flash_manager_set_flash_for_soc_access (struct omc_flash_manager *manager)
{
	int status;

	if (manager == NULL) {
		return OMC_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);

	if (!manager->boot.lock_flash) {
		status = manager->control->enable_processor_flash_access (manager->control, true);
		if (status == 0) {
			/* As soon as we give the processor access to the flash, we need to assume it is
			 * dirty. */
			host_state_manager_save_inactive_dirty (manager->boot.host_state, true);
		}
	}
	else {
		status = OMC_FLASH_MGR_UPDATE_IN_PROGRESS;
	}

	platform_mutex_unlock (&manager->lock);

	return status;
}

static int omc_flash_manager_has_blocking_update (struct omc_flash_manager *manager)
{
	int status;

	if (manager == NULL) {
		return OMC_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);
	if (manager->boot.lock_flash) {
		status = OMC_FLASH_MGR_UPDATE_IN_PROGRESS;
	}
	else {
		status = 0;
	}
	platform_mutex_unlock (&manager->lock);

	return status;
}

static int omc_flash_manager_cancel_active_updates (struct omc_flash_manager *manager)
{
	if (manager == NULL) {
		return OMC_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);
	manager->boot.update_allowed = false;
	manager->boot.lock_flash = false;
	platform_mutex_unlock (&manager->lock);

	return 0;
}

static struct omc_soc_firmware* omc_flash_manager_get_boot_image (
	struct omc_flash_manager *manager)
{
	if (manager != NULL) {
		return &manager->boot;
	}
	else {
		return NULL;
	}
}

/**
 * Erase the entire SOC flash
 *
 * @param manager The flash manager.
 *
 * @return 0 if successful or an error code.
 */
static int omc_flash_manager_erase_soc_flash (struct omc_flash_manager *manager)
{
	int status;

	if (manager == NULL) {
		return OMC_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);

	status = omc_flash_manager_enable_rot_flash_access (manager);
	if (status == 0) {
		status = flash_erase_region_and_verify (&manager->flash->base, 0, OMC_CAP_FLASH_MAX_LENGTH);
	}

	platform_mutex_unlock (&manager->lock);

	return status;
}

/**
 * Erase SOC flash image partitions (active and backup firmware images).
 * This erases flash from 0x0 to 0xF43FFFF, preserving other important flash storage
 * beyond the partition address.
 *
 * @param manager The flash manager.
 *
 * @return 0 if successful or an error code.
 */
static int omc_flash_manager_erase_image_partitions (struct omc_flash_manager *manager)
{
	int status;

	if (manager == NULL) {
		return OMC_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);

	status = omc_flash_manager_enable_rot_flash_access (manager);
	if (status == 0) {
		status = flash_erase_region_and_verify (&manager->flash->base, 0,
			OMC_CAP_FLASH_IMAGE_PARTITIONS_LENGTH);
	}

	platform_mutex_unlock (&manager->lock);

	return status;
}

int omc_flash_manager_validate_flash (const struct host_firmware *firmware, const struct pfm *pfm,
	const struct pfm *good_pfm, const struct hash_engine *hash, const struct rsa_engine *rsa)
{
	int status;
	char *platform_id = NULL;
	struct omc_soc_firmware *fw = (struct omc_soc_firmware*) firmware;

	if ((fw == NULL) || (pfm == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fw->manager->lock);

	status = fw->manager->control->processor_has_flash_access (fw->manager->control);
	if (status == 0) {
		if (fw->platform_id) {
			status = pfm->base.get_platform_id (&pfm->base, &platform_id, 0);
			if (status != 0) {
				goto exit;
			}

			/* Checking for platfom ID here to compare against the hardcoded values for a specific port is
			 * not the correct solution. This works, but it's basically a hack.  The correct way to fix the
			 * issue so we don't end up activating PFM for incorrect port would be to update the
			 * host_flash_manager_validate_flash () API and make sure that RO only region used to calculate
			 * digest belongs to the port for which PFM is being activated. */
			if (strcmp (platform_id, fw->platform_id) != 0) {
				status = OMC_FLASH_MGR_PLATFORM_ID_MISMATCH;
				goto exit;
			}
		}

		if (good_pfm && !host_state_manager_is_inactive_dirty (fw->host_state)) {
			status = host_flash_manager_validate_pfm (pfm, good_pfm, hash, rsa, fw->manager->flash,
				NULL);
		}
		else {
			status = host_flash_manager_validate_flash (pfm, hash, rsa, false, fw->manager->flash,
				NULL);
			if (status == 0) {
				host_state_manager_save_inactive_dirty (fw->host_state, false);
			}

			/* Use validation to mark the end of any active update, even if validation was not
			 * successful. */
			fw->update_allowed = false;
			fw->lock_flash = false;
		}
	}
	else if (status == 1) {
		status = HOST_FIRMWARE_NO_FLASH_ACCESS;
	}

exit:
	if (platform_id) {
		pfm->base.free_platform_id (&pfm->base, platform_id);
	}
	platform_mutex_unlock (&fw->manager->lock);

	return status;
}

static int omc_flash_manager_erase_active_flash_region (const struct host_firmware *firmware,
	const struct pfm *pfm, const struct hash_engine *hash, const struct rsa_engine *rsa)
{
	int status;

	UNUSED (pfm);
	UNUSED (hash);
	UNUSED (rsa);
	struct omc_soc_firmware *fw = (struct omc_soc_firmware*) firmware;

	if (fw == NULL) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	if (fw->max_length == 0) {
		return 0;
	}

	platform_mutex_lock (&fw->manager->lock);

	status = fw->manager->control->processor_has_flash_access (fw->manager->control);
	if (status == 0) {
		/* For CastlePeak, access to backup image is not allowed, active region erase is required*/
		status = flash_blank_check (&fw->manager->flash->base, fw->image_addr, fw->max_length);
		if (status != 0) {
			/* The active region is not blank, so need to erase it. */
			status = flash_erase_region_and_verify (&fw->manager->flash->base, fw->image_addr,
				fw->max_length);
		}
		host_state_manager_save_inactive_dirty (fw->host_state, true);
		fw->update_allowed = true;
		fw->lock_flash = false;
	}
	else if (status == 1) {
		status = HOST_FIRMWARE_NO_FLASH_ACCESS;
	}

	platform_mutex_unlock (&fw->manager->lock);

	return status;
}

static int omc_flash_manager_backup_flash (const struct host_firmware *firmware)
{
	return 0;
}

static int omc_flash_manager_prepare_image_update (const struct host_firmware *firmware,
	size_t total_length, bool lock_flash_access, uint8_t ctrl_flag)
{
	struct omc_soc_firmware *fw = (struct omc_soc_firmware*) firmware;
	int status;

	if (fw == NULL) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fw->manager->lock);

	status = fw->manager->control->processor_has_flash_access (fw->manager->control);
	if (status == 1) {
		status = omc_flash_manager_enable_rot_flash_access (fw->manager);
	}

	if (status == 0) {
		/* Always mark the flash as dirty.
		 * 1. If the flash preparation fails, we can't guarantee that the image is still good, so
		 * better to mark it as dirty. */
		host_state_manager_save_inactive_dirty (fw->host_state, true);

		status = flash_updater_prepare_for_update (&fw->update, total_length);
		if (status == 0) {
			fw->update_allowed = true;
			fw->lock_flash = lock_flash_access;
		}
	}

	platform_mutex_unlock (&fw->manager->lock);

	return status;
}

static int omc_flash_manager_write_image_update (const struct host_firmware *firmware,
	const uint8_t *data, size_t length)
{
	struct omc_soc_firmware *fw = (struct omc_soc_firmware*) firmware;
	int status;

	if (fw == NULL) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fw->manager->lock);

	if (fw->update_allowed) {
		status = fw->manager->control->processor_has_flash_access (fw->manager->control);
		if (status == 0) {
			status = flash_updater_write_update_data (&fw->update, data, length);

			/* Mark the flash as dirty in case this gets called without a call to 'prepare'. */
			host_state_manager_save_inactive_dirty (fw->host_state, true);
		}
		else if (status == 1) {
			status = HOST_FIRMWARE_NO_FLASH_ACCESS;
		}
	}
	else {
		status = HOST_FIRMWARE_UPDATE_NOT_READY;
	}

	platform_mutex_unlock (&fw->manager->lock);

	return status;
}

static int omc_flash_manager_get_remaining_update_bytes (const struct host_firmware *firmware)
{
	struct omc_soc_firmware *fw = (struct omc_soc_firmware*) firmware;

	if (firmware == NULL) {
		return 0;
	}

	return flash_updater_get_remaining_bytes (&fw->update);
}

static int omc_flash_manager_get_port_id (const struct host_firmware *firmware)
{
	return 0;	// Only port ID 0 is supported for OMC
}

/**
 * Initialize a SoC firmware image handler.
 *
 * @param firmware The firmware image to initialized.
 * @param manager The flash manager containing the image.
 * @param flash The flash device for the image.
 * @param host_state The host state for the firmware image.
 * @param image_addr The base address for the main firmware image.
 * @param backup_addr The base address for the backup firmware image.
 * @param version_addr The base address for the main firmware version string.
 * @param max_length The maximum length of a single firmware image.
 * @param platform_id The PFM platform ID for the image.
 *
 * @return 0 if the firmware image was initialized successfully or an error code.
 */
static int omc_flash_manager_init_soc_firmware (struct omc_soc_firmware *firmware,
	struct omc_flash_manager *manager, struct flash *flash,
	const struct host_state_manager *host_state, uint32_t image_addr, uint32_t max_length,
	const char *platform_id)
{
	int status;

	status = flash_updater_init (&firmware->update, flash, image_addr, max_length);
	if (status != 0) {
		return status;
	}

	firmware->base.validate_flash = omc_flash_manager_validate_flash;
	firmware->base.recover_flash = omc_flash_manager_erase_active_flash_region;
	firmware->base.backup_flash = omc_flash_manager_backup_flash;
	firmware->base.prepare_image_update = omc_flash_manager_prepare_image_update;
	firmware->base.write_image_update = omc_flash_manager_write_image_update;
	firmware->base.get_remaining_update_bytes = omc_flash_manager_get_remaining_update_bytes;
	firmware->base.get_port_id = omc_flash_manager_get_port_id;

	firmware->manager = manager;
	firmware->host_state = host_state;
	firmware->image_addr = image_addr;
	firmware->max_length = max_length;
	firmware->platform_id = platform_id;

	return 0;
}

/**
 * Internal function to initialize management of the Overlake host flash device.
 *
 * @param manager The flash manager to initialize.
 * @param flash The host flash device.
 * @param control The interface for controlling access to host flash.
 * @param soc_control The interface for SoC and SoC flash reset control.  This can be NULL
 * when soc flash reset control is not required or not supported.
 * @param host_state_boot Manager for the boot image flash state.
 * @param boot_addr The base address for the boot fw image.
 * @param boot_max_length The maximum length of the boot fw image.
 * @param boot_platform_id The PFM platform ID for the boot fw image.
 * @param board_type Type of Overlake board.
 *
 * @return 0 if the manager was successfully initialized or an error code.
 */
int omc_flash_manager_init_internal (struct omc_flash_manager *manager, struct spi_flash *flash,
	const struct host_control *control, struct overlake_control *soc_control,
	const struct host_state_manager *host_state_boot, uint32_t boot_addr, uint32_t boot_max_length,
	const char *boot_platform_id, enum overlake_board_type board_type)
{
	int status;

	memset (manager, 0, sizeof (struct omc_flash_manager));

	status = platform_mutex_init (&manager->lock);
	if (status != 0) {
		return status;
	}

	manager->board_type = board_type;

	status = omc_flash_manager_init_soc_firmware (&manager->boot, manager, &flash->base,
		host_state_boot, boot_addr, boot_max_length, boot_platform_id);
	if (status != 0) {
		goto fail_boot;
	}

	manager->set_flash_for_rot_access = omc_flash_manager_set_flash_for_rot_access;
	manager->set_flash_for_soc_access = omc_flash_manager_set_flash_for_soc_access;
	manager->has_blocking_update = omc_flash_manager_has_blocking_update;
	manager->cancel_active_updates = omc_flash_manager_cancel_active_updates;
	manager->get_boot_image = omc_flash_manager_get_boot_image;
	manager->erase_soc_flash = omc_flash_manager_erase_soc_flash;
	manager->erase_image_partitions = omc_flash_manager_erase_image_partitions;

	manager->flash = flash;
	manager->control = control;
	manager->soc_control = soc_control;

	return 0;

fail_boot:
	platform_mutex_free (&manager->lock);

	return status;
}

int omc_flash_manager_init (struct omc_flash_manager *manager, struct spi_flash *flash,
	const struct host_control *control, struct overlake_control *soc_control,
	const struct host_state_manager *host_state_boot, enum overlake_board_id board_id)
{
	uint32_t boot_addr;
	uint32_t boot_max_length;
	const char *boot_platform_id = NULL;
	enum overlake_board_type board_type = overlake_get_board_type (board_id);
	int status;

	if ((manager == NULL) || (flash == NULL) || (control == NULL) || (host_state_boot == NULL) ||
		(soc_control == NULL) || (board_type != OVERLAKE_CASTLE_PEAK)) {
		return OMC_FLASH_MGR_INVALID_ARGUMENT;
	}

	boot_addr = OMC_CAP_FLASH_BOOT_ADDR;
	boot_max_length = OMC_CAP_FLASH_BOOT_MAX_LENGTH;
	boot_platform_id = overlake_platform_id;

	status = omc_flash_manager_init_internal (manager, flash, control, soc_control, host_state_boot,
		boot_addr, boot_max_length, boot_platform_id, board_type);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize management of the Overlake SoC flash device.  The interface to the flash may be
 * uninitialized, but an initialization manager is provided to ensure it is initialized prior to
 * use.
 *
 * @param manager The flash manager to initialize.
 * @param flash The SoC flash device.
 * @param control The interface for controlling access to host flash.
 * @param soc_control The interface for SoC and SoC flash reset control. This can be NULL
 * when soc flash reset control is not required or not supported.
 * @param host_state_boot Manager for the boot image flash state.
 * @param flash_init The flash initialization manager.
 * @param board_id The Overlake board ID.
 *
 * @return 0 if the manager was successfully initialized or an error code.
 */
int omc_flash_manager_init_with_managed_flash_initialization (
	struct omc_flash_manager *manager, struct spi_flash *flash, const struct host_control *control,
	struct overlake_control *soc_control, const struct host_state_manager *host_state_boot,
	const struct host_flash_initialization *flash_init, enum overlake_board_id board_id)
{
	int status;

	if (flash_init == NULL) {
		return OMC_FLASH_MGR_INVALID_ARGUMENT;
	}

	status = omc_flash_manager_init (manager, flash, control, soc_control, host_state_boot,
		board_id);
	if (status != 0) {
		return status;
	}

	manager->flash_init = flash_init;

	return 0;
}

/**
 * Release a manager the Overlake SoC flash.
 *
 * @param manager The manager to release.
 */
void omc_flash_manager_release (struct omc_flash_manager *manager)
{
	if (manager) {
		flash_updater_release (&manager->boot.update);
		platform_mutex_free (&manager->lock);
	}
}
