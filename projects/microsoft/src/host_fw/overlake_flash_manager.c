// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "flash/flash_util.h"
#include "host_fw/host_flash_manager.h"
#include "host_fw/host_logging.h"
#include "host_fw/overlake_flash_manager.h"
#include "host_fw/overlake_host_id.h"
#include "host_fw/overlake_logging.h"


/**
 * Overlake boot mode configuration structure.
 */
struct overlake_flash_boot_mode {
	uint32_t magic;		/**< Magic number for boot mode block. */
	uint8_t version;	/**< Boot mode structure version. */
	uint8_t boot_mode;	/**< Boot mode setting. */
} __attribute__((__packed__));

/**
 * Overlake SoC MAC address structure.
 */
struct overlake_flash_mac_id {
	uint32_t magic;		/**< Magic number for MAC id block. */
	uint8_t mac_id[6];	/**< SoC MAC address. */
} __attribute__((__packed__));

/**
* Overlake SoC debugging level structure
*/
struct overlake_flash_debug_level {
	uint32_t magic;		/**< Magic number for SoC debug level block. */
	uint32_t log_level;	/**< SoC debug level. */
} __attribute__((__packed__));

/**
 * Flash layout for Celestial Peak board.
 */
#define	OVERLAKE_FLASH_SHMOO_ADDR					0x720000
#define	OVERLAKE_FLASH_SHMOO_LENGTH					0x10000

#define	OVERLAKE_FLASH_BOOT_MODE_ADDR				0x730000
#define	OVERLAKE_FLASH_BOOT_MODE_MAGIC				0x4d424c4f
#define	OVERLAKE_FLASH_BOOT_MODE_VERSION			1

#define OVERLAKE_FLASH_MAC_ID_ADDR					0x740000
#define	OVERLAKE_FLASH_MAC_ID_MAGIC					0x43414d43

#define OVERLAKE_FLASH_CRMU_LOGS_ADDR				0x760000
#define OVERLAKE_FLASH_CRMU_LOGS_LENGTH				0x40000

#define OVERLAKE_FLASH_SOC_DEBUG_ADDR				0x750000
#define OVERLAKE_FLASH_SOC_DEBUG_MAGIC				0x43484f42

#define	OVERLAKE_FLASH_BOOT_ADDR					0x000000
#define OVERLAKE_FLASH_BOOT_VERSION_ADDR			0x1FFFEE
#define	OVERLAKE_FLASH_BOOT_MAX_LENGTH				0x200000
#define	OVERLAKE_FLASH_BOOT_BACKUP_ADDR				0x200000
#define OVERLAKE_FLASH_BOOT_BACKUP_VERSION_ADDR		0x3FFFEE

#define	OVERLAKE_FLASH_NITRO_ADDR					0x400000
#define OVERLAKE_FLASH_NITRO_VERSION_ADDR			0x57FFEE
#define	OVERLAKE_FLASH_NITRO_MAX_LENGTH				0x180000
#define	OVERLAKE_FLASH_NITRO_BACKUP_ADDR			0x580000
#define OVERLAKE_FLASH_NITRO_BACKUP_VERSION_ADDR	0x6FFFEE

/**
 * Flash layout for Pioneer/Bonanza/Glacier Peak board.
 */
#define	OVERLAKE_PP_FLASH_BOOT_MODE_ADDR			0x790000

#define OVERLAKE_PP_FLASH_MAC_ID_ADDR				0x7A0000

#define OVERLAKE_PP_FLASH_DEBUG_LOGS_ADDR			0x7C0000

#define OVERLAKE_PP_FLASH_SOC_DEBUG_ADDR			0x7B0000

#define	OVERLAKE_PP_FLASH_BOOT_ADDR					0x000000
#define OVERLAKE_PP_FLASH_BOOT_VERSION_ADDR			0x3BFFEB
#define	OVERLAKE_PP_FLASH_BOOT_MAX_LENGTH			0x3C0000
#define	OVERLAKE_PP_FLASH_BOOT_BACKUP_ADDR			0x3C0000

/**
 * Flash layout for Castle Peak board.
 */
#define OVERLAKE_CAP_FLASH_BOOT_ADDR				0x000000
#define OVERLAKE_CAP_FLASH_BOOT_MAX_LENGTH			0x4000000
#define OVERLAKE_CAP_FLASH_BOOT_BACKUP_ADDR			0x4000000

/**
 * Celestial Peak FIP Platform ID
 */
static const char overlake_boot_platform_id[] = "A2040-FIP";

/**
 * Celestial Peak Nitro Platform ID
 */
static const char overlake_nitro_platform_id[] = "A2040-NITRO";

/**
 * Glacier Peak FIP Platform ID
 */
static const char overlake_gp_boot_platform_id[] = "A2051-FIP";


/**
 * Enable RoT access to the SoC flash.
 *
 * @param manager The manager for the flash to access.
 *
 * @return 0 if flash access has been enabled or an error code.
 */
int overlake_flash_manager_enable_rot_flash_access (struct overlake_flash_manager *manager)
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

static int overlake_flash_manager_set_flash_for_rot_access (struct overlake_flash_manager *manager)
{
	int status;
	uint8_t boot_mode;
	struct overlake_flash_mac_id soc_mac_config;
	struct overlake_flash_boot_mode soc_boot_config;

	if (manager == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);
	status = overlake_flash_manager_enable_rot_flash_access (manager);

	if ((status == 0) && (manager->board_type != OVERLAKE_GLACIER_PEAK) &&
		(manager->board_type != OVERLAKE_CASTLE_PEAK)) {
		status = manager->flash->base.read (&manager->flash->base,
			(manager->board_type == OVERLAKE_CELESTIAL_PEAK) ?
				OVERLAKE_FLASH_MAC_ID_ADDR : OVERLAKE_PP_FLASH_MAC_ID_ADDR,
			(uint8_t*) &soc_mac_config, sizeof (struct overlake_flash_mac_id));
		if (status == 0) {
			memcpy (manager->soc_mac_id, soc_mac_config.mac_id, 6);
		}
	}

	if ((status == 0) && (manager->board_type != OVERLAKE_CASTLE_PEAK)) {
		status = manager->flash->base.read (&manager->flash->base,
			((manager->board_type == OVERLAKE_CELESTIAL_PEAK) ?
					OVERLAKE_FLASH_BOOT_MODE_ADDR : OVERLAKE_PP_FLASH_BOOT_MODE_ADDR),
			(uint8_t*) &soc_boot_config, sizeof (struct overlake_flash_boot_mode));

		boot_mode = (status == 0) ? soc_boot_config.boot_mode : OVERLAKE_SOC_BOOT_UNKNOWN;
		overlake_flash_manager_update_boot_mode_cache (manager, 16, boot_mode);
	}

	platform_mutex_unlock (&manager->lock);

	return status;
}

static int overlake_flash_manager_set_flash_for_soc_access (struct overlake_flash_manager *manager)
{
	int status;

	if (manager == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);

	if (!manager->boot.lock_flash && !manager->nitro.lock_flash) {
		status = manager->control->enable_processor_flash_access (manager->control, true);
		if (status == 0) {
			/* As soon as we give the processor access to the flash, we need to assume it is
			 * dirty. */
			host_state_manager_save_inactive_dirty (manager->boot.host_state, true);
			host_state_manager_save_inactive_dirty (manager->nitro.host_state, true);
		}
	}
	else {
		status = OVERLAKE_FLASH_MGR_UPDATE_IN_PROGRESS;
	}

	platform_mutex_unlock (&manager->lock);

	return status;
}

static int overlake_flash_manager_set_soc_boot_mode (struct overlake_flash_manager *manager,
	uint8_t mode)
{
	int status;
	uint8_t boot_mode;

	if (manager == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if (manager->board_type == OVERLAKE_CASTLE_PEAK) {
		return OVERLAKE_FLASH_MGR_UNSUPPORTED;
	}

	if (mode >= OVERLAKE_SOC_BOOT_INVALID) {
		return OVERLAKE_FLASH_MGR_UNKNOWN_BOOT_MODE;
	}

	platform_mutex_lock (&manager->lock);

	status = overlake_flash_manager_set_boot_mode (manager,
		(manager->board_type == OVERLAKE_CELESTIAL_PEAK) ?
			OVERLAKE_FLASH_BOOT_MODE_ADDR : OVERLAKE_PP_FLASH_BOOT_MODE_ADDR, mode, false);

	boot_mode = (status == 0) ? mode : OVERLAKE_SOC_BOOT_UNKNOWN;

	overlake_flash_manager_update_boot_mode_cache (manager, 16, boot_mode);

	platform_mutex_unlock (&manager->lock);

	return status;
}

static int overlake_flash_manager_get_soc_boot_mode (struct overlake_flash_manager *manager,
	uint8_t *mode)
{
	if ((manager == NULL) || (mode == NULL)) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if (manager->board_type == OVERLAKE_CASTLE_PEAK) {
		return OVERLAKE_FLASH_MGR_UNSUPPORTED;
	}

	platform_mutex_lock (&manager->lock);

	overlake_flash_manager_get_boot_mode_cache (manager, 16, mode);

	platform_mutex_unlock (&manager->lock);

	return 0;
}

static int overlake_flash_manager_has_blocking_update (struct overlake_flash_manager *manager)
{
	int status;

	if (manager == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);
	if (manager->boot.lock_flash ||
		((manager->boot.base.get_port_id (&manager->boot.base) != OVERLAKE_HOST_PORT_FPGA_C5) &&
		manager->nitro.lock_flash)) {
		status = OVERLAKE_FLASH_MGR_UPDATE_IN_PROGRESS;
	}
	else {
		status = 0;
	}
	platform_mutex_unlock (&manager->lock);

	return status;
}

static int overlake_flash_manager_cancel_active_updates (struct overlake_flash_manager *manager)
{
	if (manager == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&manager->lock);
	manager->boot.update_allowed = false;
	manager->boot.lock_flash = false;
	manager->nitro.update_allowed = false;
	manager->nitro.lock_flash = false;
	platform_mutex_unlock (&manager->lock);

	return 0;
}

static struct overlake_soc_firmware* overlake_flash_manager_get_boot_image (
	struct overlake_flash_manager *manager)
{
	if (manager != NULL) {
		return &manager->boot;
	}
	else {
		return NULL;
	}
}

static struct overlake_soc_firmware* overlake_flash_manager_get_nitro_image (
	struct overlake_flash_manager *manager)
{
	if ((manager != NULL) && (manager->board_type == OVERLAKE_CELESTIAL_PEAK)) {
		return &manager->nitro;
	}
	else {
		return NULL;
	}
}

static int overlake_flash_manager_get_soc_mac_address (struct overlake_flash_manager *manager,
	uint8_t *mac_id)
{
	if ((manager == NULL) || (mac_id == NULL)) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if ((manager->board_type == OVERLAKE_GLACIER_PEAK) ||
		(manager->board_type == OVERLAKE_CASTLE_PEAK)) {
		return OVERLAKE_FLASH_MGR_UNSUPPORTED;
	}

	platform_mutex_lock (&manager->lock);
	memcpy (mac_id, manager->soc_mac_id, 6);
	platform_mutex_unlock (&manager->lock);

	return 0;
}

static int overlake_flash_manager_get_soc_crmu_logs (struct overlake_flash_manager *manager,
	uint32_t offset, uint8_t *contents, size_t length)
{
	int status;
	int read_bytes = 0;

	if ((manager == NULL) || (contents == NULL)) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if (manager->board_type != OVERLAKE_CELESTIAL_PEAK) {
		return OVERLAKE_FLASH_MGR_UNSUPPORTED;
	}

	if (offset >= OVERLAKE_FLASH_CRMU_LOGS_LENGTH) {
		return read_bytes;
	}

	if (length > OVERLAKE_FLASH_CRMU_LOGS_LENGTH) {
		length = OVERLAKE_FLASH_CRMU_LOGS_LENGTH - offset;
	}

	platform_mutex_lock (&manager->lock);

	read_bytes = (((offset + length) > OVERLAKE_FLASH_CRMU_LOGS_LENGTH) ?
			(OVERLAKE_FLASH_CRMU_LOGS_LENGTH - offset) : length);
	status = manager->control->processor_has_flash_access (manager->control);
	if (status == 1) {
		status = overlake_flash_manager_enable_rot_flash_access (manager);
	}

	if (status == 0) {
		status = manager->flash->base.read (&manager->flash->base,
			(OVERLAKE_FLASH_CRMU_LOGS_ADDR + offset), contents, read_bytes);
	}

	platform_mutex_unlock (&manager->lock);

	return ((status == 0) ? read_bytes : status);
}

static int overlake_flash_manager_set_soc_debug_level (struct overlake_flash_manager *manager,
	uint32_t debug_level)
{
	struct overlake_flash_debug_level config;
	int status;

	if (manager == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if ((manager->boot.base.get_port_id (&manager->boot.base) == OVERLAKE_HOST_PORT_FPGA_C5) ||
		(manager->board_type == OVERLAKE_CASTLE_PEAK)) {
		return OVERLAKE_FLASH_MGR_UNSUPPORTED;
	}

	platform_mutex_lock (&manager->lock);

	status = manager->control->processor_has_flash_access (manager->control);
	if (status == 1) {
		status = overlake_flash_manager_enable_rot_flash_access (manager);
	}

	if (status == 0) {
		config.magic = OVERLAKE_FLASH_SOC_DEBUG_MAGIC;
		config.log_level = debug_level;

		status = flash_sector_program_and_verify (&manager->flash->base,
			(manager->board_type == OVERLAKE_CELESTIAL_PEAK) ?
				OVERLAKE_FLASH_SOC_DEBUG_ADDR : OVERLAKE_PP_FLASH_SOC_DEBUG_ADDR,
			(uint8_t*) &config, sizeof (config));
	}

	platform_mutex_unlock (&manager->lock);

	return status;
}

static int overlake_flash_manager_get_soc_debug_level (struct overlake_flash_manager *manager,
	uint32_t *debug_level)
{
	struct overlake_flash_debug_level config;
	int status;

	if ((manager == NULL) || (debug_level == NULL)) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if ((manager->boot.base.get_port_id (&manager->boot.base) == OVERLAKE_HOST_PORT_FPGA_C5) ||
		(manager->board_type == OVERLAKE_CASTLE_PEAK)) {
		return OVERLAKE_FLASH_MGR_UNSUPPORTED;
	}

	platform_mutex_lock (&manager->lock);

	status = manager->control->processor_has_flash_access (manager->control);
	if (status == 0) {
		status = manager->flash->base.read (&manager->flash->base,
			(manager->board_type == OVERLAKE_CELESTIAL_PEAK) ?
				OVERLAKE_FLASH_SOC_DEBUG_ADDR : OVERLAKE_PP_FLASH_SOC_DEBUG_ADDR,
			(uint8_t*) &config, sizeof (struct overlake_flash_debug_level));
		if (status == 0) {
			*debug_level = config.log_level;
		}
	}
	else if (status == 1) {
		status = OVERLAKE_FLASH_MGR_NO_FLASH_ACCESS;
	}

	platform_mutex_unlock (&manager->lock);

	return status;
}

int overlake_flash_manager_validate_flash (const struct host_firmware *firmware,
	const struct pfm *pfm, const struct pfm *good_pfm, const struct hash_engine *hash,
	const struct rsa_engine *rsa)
{
	int status;
	char *platform_id = NULL;
	struct overlake_soc_firmware *fw = (struct overlake_soc_firmware*) firmware;

	if ((fw == NULL) || (pfm == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	if ((fw->manager->board_type != OVERLAKE_CELESTIAL_PEAK) && (fw->max_length == 0)) {
		return 0;
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
				status = OVERLAKE_FLASH_MGR_PLATFORM_ID_MISMATCH;
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

static int overlake_flash_manager_recover_flash (const struct host_firmware *firmware,
	const struct pfm *pfm, const struct hash_engine *hash, const struct rsa_engine *rsa)
{
	int status;
	struct overlake_soc_firmware *fw = (struct overlake_soc_firmware*) firmware;

	if ((fw == NULL) || (pfm == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	if ((fw->manager->board_type != OVERLAKE_CELESTIAL_PEAK) && (fw->max_length == 0)) {
		return 0;
	}

	platform_mutex_lock (&fw->manager->lock);

	status = fw->manager->control->processor_has_flash_access (fw->manager->control);
	if (status == 0) {
		status = host_flash_manager_validate_offset_flash (pfm, hash, rsa, false,
			fw->manager->flash, fw->max_length, NULL);
		if (status != 0) {
			goto exit;
		}

		if (fw->erase_shmoo) {
			status = flash_erase_region_and_verify (&fw->manager->flash->base,
				OVERLAKE_FLASH_SHMOO_ADDR, OVERLAKE_FLASH_SHMOO_LENGTH);
			if (status != 0) {
				goto exit;
			}
		}

		status = flash_copy_and_verify (&fw->manager->flash->base, fw->image_addr, fw->backup_addr,
			fw->max_length);

		/* Set the dirty state of the flash based on the result of the flash copy:  dirty if the
		 * copy failed or not dirty if successful. */
		host_state_manager_save_inactive_dirty (fw->host_state, (status != 0));
		fw->update_allowed = false;
		fw->lock_flash = false;
	}
	else if (status == 1) {
		status = HOST_FIRMWARE_NO_FLASH_ACCESS;
	}

exit:
	platform_mutex_unlock (&fw->manager->lock);

	return status;
}

/**
 * Copy the main image to the backup region.
 *
 * @param firmware The firmware image to copy.
 *
 * @return 0 if the backup region contains a copy of the image or an error code.
 */
static int overlake_flash_manager_copy_image_to_backup (struct overlake_soc_firmware *firmware)
{
	int status;

	status = flash_verify_copy (&firmware->manager->flash->base, firmware->image_addr,
		firmware->backup_addr, firmware->max_length);
	if (status != FLASH_UTIL_DATA_MISMATCH) {
		return status;
	}

	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_HOST_FW,
		HOST_LOGGING_BACKUP_FIRMWARE_STARTED, firmware->base.get_port_id (&firmware->base), 0);
	status = flash_copy_and_verify (&firmware->manager->flash->base, firmware->backup_addr,
		firmware->image_addr, firmware->max_length);

	debug_log_create_entry ((status == 0) ? DEBUG_LOG_SEVERITY_INFO : DEBUG_LOG_SEVERITY_WARNING,
		DEBUG_LOG_COMPONENT_HOST_FW, HOST_LOGGING_BACKUP_FIRMWARE_COMPLETED,
		firmware->base.get_port_id (&firmware->base), status);

	return status;
}

static int overlake_flash_manager_backup_flash (const struct host_firmware *firmware)
{
	struct overlake_soc_firmware *fw = (struct overlake_soc_firmware*) firmware;
	int status;

	if (fw == NULL) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	if ((fw->manager->board_type != OVERLAKE_CELESTIAL_PEAK) && (fw->max_length == 0)) {
		return 0;
	}

	platform_mutex_lock (&fw->manager->lock);

	status = fw->manager->control->processor_has_flash_access (fw->manager->control);
	if (status == 0) {
		if (host_state_manager_is_inactive_dirty (fw->host_state)) {
			status = HOST_FIRMWARE_IMAGE_NOT_VALID;
			goto exit;
		}

		status = overlake_flash_manager_copy_image_to_backup (fw);
	}
	else if (status == 1) {
		status = HOST_FIRMWARE_NO_FLASH_ACCESS;
	}

exit:
	platform_mutex_unlock (&fw->manager->lock);

	return status;
}

static int overlake_flash_manager_prepare_image_update (const struct host_firmware *firmware,
	size_t total_length, bool lock_flash_access, uint8_t ctrl_flag)
{
	struct overlake_soc_firmware *fw = (struct overlake_soc_firmware*) firmware;
	int status;

	if (fw == NULL) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	if ((fw->manager->board_type != OVERLAKE_CELESTIAL_PEAK) && (fw->max_length == 0)) {
		return HOST_FIRMWARE_UNSUPPORTED;
	}

	platform_mutex_lock (&fw->manager->lock);

	status = fw->manager->control->processor_has_flash_access (fw->manager->control);
	if (status == 1) {
		status = overlake_flash_manager_enable_rot_flash_access (fw->manager);
	}

	if (status == 0) {
		if (!host_state_manager_is_inactive_dirty (fw->host_state)) {
			status = overlake_flash_manager_copy_image_to_backup (fw);
			if (status != 0) {
				goto exit;
			}
		}

		if (fw->erase_shmoo && (ctrl_flag == 0)) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_OVERLAKE,
				OVERLAKE_LOGGING_ERASE_SHMOO, fw->base.get_port_id (&fw->base), 0);

			status = flash_erase_region_and_verify (&fw->manager->flash->base,
				OVERLAKE_FLASH_SHMOO_ADDR, OVERLAKE_FLASH_SHMOO_LENGTH);
			if (status != 0) {
				goto exit;
			}
		}

		/* Always mark the flash as dirty.
		 * 1. We've already backed up the good flash,
		 * so we know we have a good copy there with no reason to back it up again.
		 * 2. If the flash preparation fails, we can't guarantee that the image is still good, so
		 * better to mark it as dirty. */
		host_state_manager_save_inactive_dirty (fw->host_state, true);

		status = flash_updater_prepare_for_update (&fw->update, total_length);
		if (status == 0) {
			fw->update_allowed = true;
			fw->lock_flash = lock_flash_access;
		}
	}

exit:
	platform_mutex_unlock (&fw->manager->lock);

	return status;
}

static int overlake_flash_manager_write_image_update (const struct host_firmware *firmware,
	const uint8_t *data, size_t length)
{
	struct overlake_soc_firmware *fw = (struct overlake_soc_firmware*) firmware;
	int status;

	if (fw == NULL) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	if ((fw->manager->board_type != OVERLAKE_CELESTIAL_PEAK) && (fw->max_length == 0)) {
		return HOST_FIRMWARE_UNSUPPORTED;
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

static int overlake_flash_manager_get_remaining_update_bytes (const struct host_firmware *firmware)
{
	struct overlake_soc_firmware *fw = (struct overlake_soc_firmware*) firmware;

	if (firmware == NULL) {
		return 0;
	}

	return flash_updater_get_remaining_bytes (&fw->update);
}

static int overlake_flash_manager_get_port_id (const struct host_firmware *firmware)
{
	struct overlake_soc_firmware *fw = (struct overlake_soc_firmware*) firmware;

	if (fw == NULL) {
		return -1;
	}

	if (fw->manager->board_type != OVERLAKE_CELESTIAL_PEAK) {
		return (fw->max_length == 0);
	}
	else {
		return (fw->image_addr == OVERLAKE_FLASH_NITRO_ADDR);
	}
}

static int overlake_flash_manager_get_fw_version (const struct overlake_soc_firmware *firmware,
	char *version, size_t length)
{
	int status;

	if ((firmware == NULL) || (version == NULL) || (length == 0)) {
		return HOST_FIRMWARE_INVALID_ARGUMENT;
	}

	if (length <= firmware->version_length) {
		return HOST_FIRMWARE_BUFFER_TOO_SMALL;
	}

	platform_mutex_lock (&firmware->manager->lock);

	status = firmware->manager->control->processor_has_flash_access (firmware->manager->control);
	if (status == 0) {
		status = firmware->manager->flash->base.read (&firmware->manager->flash->base,
			firmware->image_version_addr, (uint8_t*) version, firmware->version_length);
	}
	else if (status == 1) {
		status = HOST_FIRMWARE_NO_FLASH_ACCESS;
	}

	/* NULL terminate the version string. */
	version[firmware->version_length] = '\0';

	platform_mutex_unlock (&firmware->manager->lock);

	return status;
}

/**
 * Common helper function for updating the boot mode cache of the managed flash instance.  This
 * function is not thread-safe.
 *
 * @param manager The flash manager to initialize.
 * @param offset The byte offset location of the boot mode in the cache.
 * @param mode The boot mode value to write to the cache at the given offset.
 *
 */
void overlake_flash_manager_update_boot_mode_cache (struct overlake_flash_manager *manager,
	uint32_t offset, uint8_t mode)
{
	*(manager->boot_mode_cache) = ((*manager->boot_mode_cache) & ~(0xFFFFU << offset)) |
		(mode << offset);
}

/**
 * Common helper function for retrieving the cached boot mode of the managed flash instance.  This
 * function is not thread-safe.
 *
 * @param manager The flash manager to initialize.
 * @param offset The byte offset location of the boot mode in the cache.
 * @param mode The output boot mode value in the cache at the given offset.
 *
 */
void overlake_flash_manager_get_boot_mode_cache (struct overlake_flash_manager *manager,
	uint32_t offset, uint8_t *mode)
{
	*mode = ((*manager->boot_mode_cache >> offset) & 0xFFU);
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
static int overlake_flash_manager_init_soc_firmware (struct overlake_soc_firmware *firmware,
	struct overlake_flash_manager *manager, struct flash *flash,
	const struct host_state_manager *host_state, uint32_t image_addr, uint32_t backup_addr,
	uint32_t version_addr, uint32_t max_length, const char *platform_id)
{
	int status;

	status = flash_updater_init (&firmware->update, flash, image_addr, max_length);
	if (status != 0) {
		return status;
	}

	firmware->base.validate_flash = overlake_flash_manager_validate_flash;
	firmware->base.recover_flash = overlake_flash_manager_recover_flash;
	firmware->base.backup_flash = overlake_flash_manager_backup_flash;
	firmware->base.prepare_image_update = overlake_flash_manager_prepare_image_update;
	firmware->base.write_image_update = overlake_flash_manager_write_image_update;
	firmware->base.get_remaining_update_bytes = overlake_flash_manager_get_remaining_update_bytes;
	firmware->base.get_port_id = overlake_flash_manager_get_port_id;

	firmware->manager = manager;
	firmware->host_state = host_state;
	firmware->image_addr = image_addr;
	firmware->image_version_addr = version_addr;
	firmware->backup_addr = backup_addr;
	firmware->max_length = max_length;
	firmware->version_length = (image_addr + max_length) - version_addr;
	firmware->platform_id = platform_id;
	firmware->get_fw_version = overlake_flash_manager_get_fw_version;

	if (firmware->manager->board_type != OVERLAKE_CELESTIAL_PEAK) {
		firmware->erase_shmoo = false;
	}
	else {
		firmware->erase_shmoo = (image_addr == OVERLAKE_FLASH_BOOT_ADDR);
	}

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
 * @param host_state_nitro Manager for the Nitro image flash state, If applicable.
 * @param boot_addr The base address for the boot fw image.
 * @param boot_backup_addr The base address for the backup boot fw image.
 * @param boot_version_addr The base address for the boot fw version string.
 * @param boot_max_length The maximum length of the boot fw image.
 * @param boot_platform_id The PFM platform ID for the boot fw image.
 * @param nitro_addr The base address for the nitro fw image, if applicable.
 * @param nitro_backup_addr The base address for the backup nitro fw image, if applicable.
 * @param nitro_version_addr The base address for the nitro fw version string, if applicable.
 * @param nitro_max_length The maximum length for the nitro fw image, if applicable.
 * @param nitro_platform_id The PFM platform ID for the nitro fw image, if applicable.
 * @param board_type Type of Overlake board.
 *
 * @return 0 if the manager was successfully initialized or an error code.
 */
int overlake_flash_manager_init_internal (struct overlake_flash_manager *manager,
	struct spi_flash *flash, const struct host_control *control,
	struct overlake_control *soc_control, const struct host_state_manager *host_state_boot,
	const struct host_state_manager *host_state_nitro, uint32_t boot_addr,
	uint32_t boot_backup_addr, uint32_t boot_version_addr, uint32_t boot_max_length,
	const char *boot_platform_id, uint32_t nitro_addr, uint32_t nitro_backup_addr,
	uint32_t nitro_version_addr, uint32_t nitro_max_length, const char *nitro_platform_id,
	enum overlake_board_type board_type)
{
	int status;

	memset (manager, 0, sizeof (struct overlake_flash_manager));

	status = platform_mutex_init (&manager->lock);
	if (status != 0) {
		return status;
	}

	manager->board_type = board_type;

	status = overlake_flash_manager_init_soc_firmware (&manager->boot, manager, &flash->base,
		host_state_boot, boot_addr, boot_backup_addr, boot_version_addr, boot_max_length,
		boot_platform_id);
	if (status != 0) {
		goto fail_boot;
	}

	if (host_state_nitro != NULL) {
		status = overlake_flash_manager_init_soc_firmware (&manager->nitro, manager, &flash->base,
			host_state_nitro, nitro_addr, nitro_backup_addr, nitro_version_addr, nitro_max_length,
			nitro_platform_id);
		if (status != 0) {
			goto fail_nitro;
		}
	}

	manager->set_flash_for_rot_access = overlake_flash_manager_set_flash_for_rot_access;
	manager->set_flash_for_soc_access = overlake_flash_manager_set_flash_for_soc_access;
	manager->set_soc_boot_mode = overlake_flash_manager_set_soc_boot_mode;
	manager->get_soc_boot_mode = overlake_flash_manager_get_soc_boot_mode;
	manager->get_soc_mac_address = overlake_flash_manager_get_soc_mac_address;
	manager->has_blocking_update = overlake_flash_manager_has_blocking_update;
	manager->cancel_active_updates = overlake_flash_manager_cancel_active_updates;
	manager->get_boot_image = overlake_flash_manager_get_boot_image;
	manager->get_nitro_image = overlake_flash_manager_get_nitro_image;
	manager->get_soc_crmu_log = overlake_flash_manager_get_soc_crmu_logs;
	manager->set_soc_debug_level = overlake_flash_manager_set_soc_debug_level;
	manager->get_soc_debug_level = overlake_flash_manager_get_soc_debug_level;

	manager->flash = flash;
	manager->control = control;
	manager->soc_control = soc_control;

	return 0;

fail_nitro:
	flash_updater_release (&manager->boot.update);
fail_boot:
	platform_mutex_free (&manager->lock);

	return status;
}

/**
 * Common helper function for setting the boot mode of the managed flash instance.  This function is
 * not thread-safe.
 *
 * @param manager The flash manager to initialize.
 * @param offset The byte offset location of the boot mode.
 * @param mode The mode value to write to the flash location.
 * @param restore_soc_flash_access Indicates if flash access should be given back to the SoC
 *
 * @return 0 if the manager was successfully initialized or an error code.
 */
int overlake_flash_manager_set_boot_mode (struct overlake_flash_manager *manager, uint32_t offset,
	uint8_t mode, bool restore_soc_flash_access)
{
	struct overlake_flash_boot_mode config;
	int status;
	int restore_status = 0;

	if (manager == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	status = manager->control->processor_has_flash_access (manager->control);
	if (status == 1) {
		status = overlake_flash_manager_enable_rot_flash_access (manager);
	}

	if (status == 0) {
		config.magic = OVERLAKE_FLASH_BOOT_MODE_MAGIC;
		config.version = OVERLAKE_FLASH_BOOT_MODE_VERSION;
		config.boot_mode = mode;

		status = flash_sector_program_and_verify (&manager->flash->base, offset, (uint8_t*) &config,
			sizeof (config));
	}

	if (restore_soc_flash_access) {
		restore_status = manager->control->enable_processor_flash_access (manager->control, true);

		/* A failure writing the boot mode takes precendence over an error restoring flash access.
		 * Restoring flash access is generally just setting a register value, so less likely to
		 * fail anyway. */
		if (status == 0) {
			status = restore_status;
		}
	}

	return status;
}

/**
 * Common helper function for getting the boot mode of the managed flash instance.  This function
 * is not thread-safe.
 *
 * @param manager The flash manager to initialize.
 * @param offset The byte offset location of the boot mode.
 * @param mode An out parameter for storing the mode value from flash.
 *
 * @return 0 if the manager was successfully initialized or an error code.
 */
int overlake_flash_manager_get_boot_mode (struct overlake_flash_manager *manager, uint32_t offset,
	uint8_t *mode)
{
	struct overlake_flash_boot_mode config;
	int status;

	if ((manager == NULL) || (mode == NULL)) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	status = manager->control->processor_has_flash_access (manager->control);
	if (status == 0) {
		status = manager->flash->base.read (&manager->flash->base, offset, (uint8_t*) &config,
			sizeof (struct overlake_flash_boot_mode));
		if (status == 0) {
			*mode = config.boot_mode;
		}
	}
	else if (status == 1) {
		status = OVERLAKE_FLASH_MGR_NO_FLASH_ACCESS;
	}

	return status;
}

/**
 * Initialize management of the Overlake SoC flash device.
 *
 * @param manager The flash manager to initialize.
 * @param flash The SoC flash device.
 * @param control The interface for controlling access to host flash.
 * @param soc_control The interface for SoC and SoC flash reset control.  This can be NULL
 * when soc flash reset control is not required or not supported.
 * @param host_state_boot Manager for the boot image flash state.
 * @param host_state_nitro Manager for the Nitro image flash state.  This can be set to
 *  NULL for non-Celestial Peak Hardware.
 * @param board_id The Overlake board ID.
 * @param boot_mode_cache The cache address to hold Host boot mode.  If Cerberus doesn't have run-time
 * host flash access, this needs to persist across Cerberus reset.
 * @param is_por Boolean value indicating if the source of cerberus reset is power on reset.
 *
 * @return 0 if the manager was successfully initialized or an error code.
 */
int overlake_flash_manager_init (struct overlake_flash_manager *manager, struct spi_flash *flash,
	const struct host_control *control, struct overlake_control *soc_control,
	const struct host_state_manager *host_state_boot,
	const struct host_state_manager *host_state_nitro, enum overlake_board_id board_id,
	volatile uint32_t *boot_mode_cache, bool is_por)
{
	uint32_t boot_addr;
	uint32_t boot_backup_addr;
	uint32_t boot_version_addr = 0;
	uint32_t boot_max_length;
	const char *boot_platform_id = NULL;
	uint32_t nitro_addr = 0;
	uint32_t nitro_backup_addr = 0;
	uint32_t nitro_version_addr = 0;
	uint32_t nitro_max_length = 0;
	const char *nitro_platform_id = NULL;
	enum overlake_board_type board_type = overlake_get_board_type (board_id);
	int status;

	if ((manager == NULL) || (flash == NULL) || (control == NULL) || (host_state_boot == NULL) ||
		((board_id == OVERLAKE_BOARD_ID_CELESTIAL_PEAK) && (host_state_nitro == NULL))) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if ((board_type != OVERLAKE_CASTLE_PEAK) && (boot_mode_cache == NULL)) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if ((board_type == OVERLAKE_CASTLE_PEAK) && (soc_control == NULL)) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	if (board_id > OVERLAKE_BOARD_ID_CELESTIAL_PEAK) {
		return OVERLAKE_FLASH_MGR_UNKNOWN_BOARD_ID;
	}

	if (board_type == OVERLAKE_CASTLE_PEAK) {
		boot_addr = OVERLAKE_CAP_FLASH_BOOT_ADDR;
		boot_backup_addr = OVERLAKE_CAP_FLASH_BOOT_BACKUP_ADDR;
		boot_max_length = OVERLAKE_CAP_FLASH_BOOT_MAX_LENGTH;
		boot_platform_id = NULL;
	}
	else if (board_type == OVERLAKE_CELESTIAL_PEAK) {
		boot_addr = OVERLAKE_FLASH_BOOT_ADDR;
		boot_backup_addr = OVERLAKE_FLASH_BOOT_BACKUP_ADDR;
		boot_version_addr = OVERLAKE_FLASH_BOOT_VERSION_ADDR;
		boot_max_length = OVERLAKE_FLASH_BOOT_MAX_LENGTH;
		boot_platform_id = overlake_boot_platform_id;

		nitro_addr = OVERLAKE_FLASH_NITRO_ADDR;
		nitro_backup_addr = OVERLAKE_FLASH_NITRO_BACKUP_ADDR;
		nitro_version_addr = OVERLAKE_FLASH_NITRO_VERSION_ADDR;
		nitro_max_length = OVERLAKE_FLASH_NITRO_MAX_LENGTH;
		nitro_platform_id = overlake_nitro_platform_id;
	}
	else {
		boot_addr = OVERLAKE_PP_FLASH_BOOT_ADDR;
		boot_backup_addr = OVERLAKE_PP_FLASH_BOOT_BACKUP_ADDR;
		boot_version_addr = OVERLAKE_PP_FLASH_BOOT_VERSION_ADDR;
		boot_max_length = OVERLAKE_PP_FLASH_BOOT_MAX_LENGTH;
		if (board_type == OVERLAKE_GLACIER_PEAK) {
			boot_platform_id = overlake_gp_boot_platform_id;
		}
	}

	status = overlake_flash_manager_init_internal (manager, flash, control, soc_control,
		host_state_boot, host_state_nitro, boot_addr, boot_backup_addr, boot_version_addr,
		boot_max_length, boot_platform_id, nitro_addr, nitro_backup_addr, nitro_version_addr,
		nitro_max_length, nitro_platform_id, board_type);
	if (status != 0) {
		return status;
	}

	if (board_type != OVERLAKE_CASTLE_PEAK) {
		manager->boot_mode_cache = boot_mode_cache;

		if (is_por) {
			overlake_flash_manager_update_boot_mode_cache (manager, 16, OVERLAKE_SOC_BOOT_UNKNOWN);
		}
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
 * @param soc_control The interface for SoC and SoC flash reset control.  This can be NULL
 * when soc flash reset control is not required or not supported.
 * @param host_state_boot Manager for the boot image flash state.
 * @param host_state_nitro Manager for the Nitro image flash state.
 * @param flash_init The flash initialization manager.
 * @param board_id The Overlake board ID.
 * @param boot_mode_cache The cache address to hold Host boot mode.  If Cerberus doesn't have run-time
 * host flash access, this needs to persist across Cerberus soft-resets.
 * @param is_por Boolean value indicating if the source of cerberus reset is power on reset.
 *
 * @return 0 if the manager was successfully initialized or an error code.
 */
int overlake_flash_manager_init_with_managed_flash_initialization (
	struct overlake_flash_manager *manager, struct spi_flash *flash,
	const struct host_control *control, struct overlake_control *soc_control,
	const struct host_state_manager *host_state_boot,
	const struct host_state_manager *host_state_nitro,
	const struct host_flash_initialization *flash_init, enum overlake_board_id board_id,
	volatile uint32_t *boot_mode_cache, bool is_por)
{
	int status;

	if (flash_init == NULL) {
		return OVERLAKE_FLASH_MGR_INVALID_ARGUMENT;
	}

	status = overlake_flash_manager_init (manager, flash, control, soc_control, host_state_boot,
		host_state_nitro, board_id, boot_mode_cache, is_por);
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
void overlake_flash_manager_release (struct overlake_flash_manager *manager)
{
	if (manager) {
		flash_updater_release (&manager->boot.update);
		flash_updater_release (&manager->nitro.update);
		platform_mutex_free (&manager->lock);
	}
}
