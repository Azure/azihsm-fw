// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OMC_FLASH_MANAGER_H_
#define OMC_FLASH_MANAGER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "platform_api.h"
#include "crypto/hash.h"
#include "crypto/rsa.h"
#include "flash/flash_updater.h"
#include "flash/spi_flash.h"
#include "host_fw/host_control.h"
#include "host_fw/host_firmware.h"
#include "host_fw/host_flash_initialization.h"
#include "host_fw/host_state_manager.h"
#include "host_fw/overlake_board_id.h"
#include "host_fw/overlake_control.h"
#include "manifest/pfm/pfm.h"
#include "status/msft_module_id.h"
#include "status/rot_status.h"

struct omc_flash_manager;

/**
 * Handler for operations on a single firmware image stored on the protected flash device.
 */
struct omc_soc_firmware {
	struct host_firmware base;						/**< Base SoC firmware instance. */
	struct omc_flash_manager *manager;				/**< Manager for the firmware image on flash. */
	const struct host_state_manager *host_state;	/**< Manager for host state. */
	uint32_t image_addr;							/**< Base address for the main image. */
	uint32_t max_length;							/**< Maximum image length. */
	struct flash_updater update;					/**< Update management for the image flash. */
	bool update_allowed;							/**< Flag indicating if updates are permitted. */
	bool lock_flash;								/**< Flag indicating if SoC flash access is permitted. */
	const char *platform_id;						/**< PFM platform ID for the image. */
};

/**
 * Management of the protected flash device for the OMC SoC.
 */
struct omc_flash_manager {
	/**
	 * Configure the system for to allow RoT access to the protected flash device.
	 *
	 * @param manager The manager for the flash device to access from the RoT.
	 *
	 * @return 0 if the flash was successfully configured for RoT access or an error code.
	 */
	int (*set_flash_for_rot_access) (struct omc_flash_manager *manager);

	/**
	 * Configure the system for to allow SoC access to the protected flash device.
	 *
	 * @param manager The manager for the flash device to access from the SoC.
	 *
	 * @return 0 if the flash was successfully configured for SoC access or an error code.
	 */
	int (*set_flash_for_soc_access) (struct omc_flash_manager *manager);

	/**
	 * Check if there is a SoC firmware update currently in progress that would block SoC access
	 * to the flash.
	 *
	 * @param manager The flash manager to query.
	 *
	 * @return 0 if no update is active or an error code.
	 */
	int (*has_blocking_update) (struct omc_flash_manager *manager);

	/**
	 * Cancel SoC firmware updates that are currently in progress.  This will allow the SoC to
	 * regain access to the flash.
	 *
	 * @param manger The flash manager to update.
	 *
	 * @return 0 if the updates were cancelled or an error code.
	 */
	int (*cancel_active_updates) (struct omc_flash_manager *manager);

	/**
	 * Erase the SoC flash memory.
	 *
	 * This function will erase the entire SoC flash memory.
	 *
	 * @param manager The flash manager to use for the operation.
	 *
	 * @return 0 if the erase was successful or an error code.
	 */
	int (*erase_soc_flash) (struct omc_flash_manager *manager);

	/**
	 * Erase the SoC flash image partitions (active and backup firmware images).
	 *
	 * This function will erase the SoC flash memory from address 0x0 to 0xF43FFFF,
	 * preserving other important flash storage beyond the partition address.
	 *
	 * @param manager The flash manager to use for the operation.
	 *
	 * @return 0 if the erase was successful or an error code.
	 */
	int (*erase_image_partitions) (struct omc_flash_manager *manager);

	/**
	 * Get the interface for working with the OMC SoC boot image.
	 *
	 * @param manager The flash manager to query.
	 *
	 * @return The boot image instance or null if the manager is not valid.
	 */
	struct omc_soc_firmware* (*get_boot_image) (struct omc_flash_manager *manager);

	struct spi_flash *flash;							/**< The protected flash device. */
	const struct host_control *control;					/**< Control interface for host flash. */
	struct overlake_control *soc_control;				/**< Control interface for SoC and SoC flash. */
	const struct host_flash_initialization *flash_init;	/**< Initialization manager for SPI flash. */
	platform_mutex lock;								/**< Synchronization for flash operations. */
	struct omc_soc_firmware boot;						/**< Boot firmware image. */
	enum overlake_board_type board_type;				/**< Type of Overlake board. */
};


int omc_flash_manager_init (struct omc_flash_manager *manager, struct spi_flash *flash,
	const struct host_control *control, struct overlake_control *soc_control,
	const struct host_state_manager *host_state_boot, enum overlake_board_id board_id);

int omc_flash_manager_init_with_managed_flash_initialization (
	struct omc_flash_manager *manager, struct spi_flash *flash, const struct host_control *control,
	struct overlake_control *soc_control, const struct host_state_manager *host_state_boot,
	const struct host_flash_initialization *flash_init, enum overlake_board_id board_id);
void omc_flash_manager_release (struct omc_flash_manager *manager);

/* Internal function used by derived types */
int omc_flash_manager_init_internal (struct omc_flash_manager *manager, struct spi_flash *flash,
	const struct host_control *control, struct overlake_control *soc_control,
	const struct host_state_manager *host_state_boot, uint32_t boot_addr, uint32_t boot_max_length,
	const char *boot_platform_id, enum overlake_board_type board_type);

int omc_flash_manager_enable_rot_flash_access (struct omc_flash_manager *manager);

int omc_flash_manager_validate_flash (const struct host_firmware *firmware, const struct pfm *pfm,
	const struct pfm *good_pfm, const struct hash_engine *hash, const struct rsa_engine *rsa);


#define	OMC_FLASH_MGR_ERROR(code)		ROT_ERROR (MSFT_MODULE_OMC_FLASH_MGR, code)

/**
 * Error codes that can be generated by a manager of OMC flash.
 */
enum {
	OMC_FLASH_MGR_INVALID_ARGUMENT = OMC_FLASH_MGR_ERROR (0x00),		/**< Input parameter is null or not valid. */
	OMC_FLASH_MGR_UPDATE_IN_PROGRESS = OMC_FLASH_MGR_ERROR (0x01),		/**< SoC firmware update is running. */
	OMC_FLASH_MGR_PLATFORM_ID_MISMATCH = OMC_FLASH_MGR_ERROR (0x02),	/**< Platform ID of PFM used to validate flash does not match assigned platform ID. */
	OMC_FLASH_MGR_SOC_ACCESS_FAILED = OMC_FLASH_MGR_ERROR (0x03),		/**< Failed to set SoC flash access. */
	OMC_FLASH_MGR_ROT_ACCESS_FAILED = OMC_FLASH_MGR_ERROR (0x04),		/**< Failed to set RoT flash access. */
};


#endif	/* OMC_FLASH_MANAGER_H_ */
