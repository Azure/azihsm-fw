// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_FLASH_MANAGER_H_
#define OVERLAKE_FLASH_MANAGER_H_

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


/**
 * Boot mode settings for the Overlake SoC.
 */
enum overlake_soc_boot_mode {
	OVERLAKE_SOC_BOOT_EMMC,				/**< SoC will boot from eMMC. */
	OVERLAKE_SOC_BOOT_USB,				/**< SoC will boot from USB. */
	OVERLAKE_SOC_BOOT_PXE,				/**< SoC will boot using PXE. */
	OVERLAKE_SOC_BOOT_NETWORK_UNLOCK,	/**< SoC will boot in network unlock mode. */
	OVERLAKE_SOC_BOOT_PXE_IPV6,			/**< Soc will boot using PXE with IPV6. */
	OVERLAKE_SOC_BOOT_INVALID,			/**< Invalid boot mode. */
	OVERLAKE_SOC_BOOT_UNKNOWN = 0xff,	/**< Unknown boot mode. */
};

struct overlake_flash_manager;

/**
 * Handler for operations on a single firmware image stored on the protected flash device.
 */
struct overlake_soc_firmware {
	struct host_firmware base;	/**< Base SoC firmware instance. */

	/**
	 * Get the SoC FW Version string.
	 *
	 * @param firmware The firmware image to query.
	 * @param version The pointer to the output buffer for the version string.  The buffer cannot be
	 * NULL and MUST BE large enough to hold the version string.
	 * @param length The length of the output buffer to filled with version string.
	 *
	 * @return 0 if the fwversion was successfully read or an error code.
	 */
	int (*get_fw_version) (const struct overlake_soc_firmware *firmware, char *version,
		size_t length);

	struct overlake_flash_manager *manager;			/**< Manager for the firmware image on flash. */
	const struct host_state_manager *host_state;	/**< Manager for host state. */
	uint32_t image_addr;							/**< Base address for the main image. */
	uint32_t image_version_addr;					/**< Base address for the main image version string.*/
	uint32_t backup_addr;							/**< Base address for the backup image. */
	uint32_t max_length;							/**< Maximum image length. */
	size_t version_length;							/**< Image version string length. */
	struct flash_updater update;					/**< Update management for the image flash. */
	bool update_allowed;							/**< Flag indicating if updates are permitted. */
	bool lock_flash;								/**< Flag indicating if SoC flash access is permitted. */
	bool erase_shmoo;								/**< Flag indicating if shmoo flash should be erased. */
	const char *platform_id;						/**< PFM platform ID for the image. */
};

/**
 * Management of the protected flash device for the Overlake SoC.
 */
struct overlake_flash_manager {
	/**
	 * Configure the system for to allow RoT access to the protected flash device.
	 *
	 * @param manager The manager for the flash device to access from the RoT.
	 *
	 * @return 0 if the flash was successfully configured for RoT access or an error code.
	 */
	int (*set_flash_for_rot_access) (struct overlake_flash_manager *manager);

	/**
	 * Configure the system for to allow SoC access to the protected flash device.
	 *
	 * @param manager The manager for the flash device to access from the SoC.
	 *
	 * @return 0 if the flash was successfully configured for SoC access or an error code.
	 */
	int (*set_flash_for_soc_access) (struct overlake_flash_manager *manager);

	/**
	 * Configure the boot mode for the SoC.
	 *
	 * @param manager The flash manager to use for updating the boot mode.
	 * @param mode The SoC boot mode to configure.
	 *
	 * @return 0 if the boot mode was successfully configured or an error code.
	 */
	int (*set_soc_boot_mode) (struct overlake_flash_manager *manager, uint8_t mode);

	/**
	 * Get the current boot mode configuration of the SoC.
	 *
	 * @param manager The flash manager to use for reading the boot mode.
	 * @param mode To be populated with the current SoC boot mode.
	 *
	 * @return 0 if the boot mode was successfully read or an error code.
	 */
	int (*get_soc_boot_mode) (struct overlake_flash_manager *manager, uint8_t *mode);

	/**
	 * Get the MAC address of the SoC.
	 *
	 * @param manager The flash manager to use for reading the MAC address.
	 * @param mac_addr To be populated with the SoC MAC address.  This buffer will always be
	 * filled with 6 bytes of data.
	 *
	 * @return 0 if the MAC address was successfully read or an error code.
	 */
	int (*get_soc_mac_address) (struct overlake_flash_manager *manager, uint8_t *mac_addr);

	/**
	 * Get CRMU logs from the SoC.
	 *
	 * @param manager The flash manager to use for reading the logs.
	 * @param offset The offset within the log to start
	 * @param contents Output buffer to be populated with contents of CRMU logs.
	 * @param length The maximum length of the contents to read.
	 *
	 * @return The number of bytes read from the log or an error code.  Use ROT_IS_ERROR to check
	 * the return value.
	 */
	int (*get_soc_crmu_log) (struct overlake_flash_manager *manager, uint32_t offset,
		uint8_t *contents, size_t length);

	/**
	 * Configure the debug level for the SoC.
	 *
	 * @param manager The flash manager to use for updating the debug level.
	 * @param debug_level The SoC debug level to configure.
	 *
	 * @return 0 if the debug level was successfully configured or an error code.
	 */
	int (*set_soc_debug_level) (struct overlake_flash_manager *manager, uint32_t debug_level);

	/**
	 * Get the SoC debug level.
	 *
	 * @param manager The flash manager to use for reading the debug level.
	 * @param debug_level To be populated with the SoC debug level.
	 *
	 * @return 0 if the debug level was successfully read or an error code.
	 */
	int (*get_soc_debug_level) (struct overlake_flash_manager *manager, uint32_t *debug_level);

	/**
	 * Check if there is a SoC firmware update currently in progress that would block SoC access
	 * to the flash.
	 *
	 * @param manager The flash manager to query.
	 *
	 * @return 0 if no update is active or an error code.
	 */
	int (*has_blocking_update) (struct overlake_flash_manager *manager);

	/**
	 * Cancel SoC firmware updates that are currently in progress.  This will allow the SoC to
	 * regain access to the flash.
	 *
	 * @param manger The flash manager to update.
	 *
	 * @return 0 if the updates were cancelled or an error code.
	 */
	int (*cancel_active_updates) (struct overlake_flash_manager *manager);

	/**
	 * Get the interface for working with the Overlake SoC boot image.
	 *
	 * @param manager The flash manager to query.
	 *
	 * @return The boot image instance or null if the manager is not valid.
	 */
	struct overlake_soc_firmware* (*get_boot_image) (struct overlake_flash_manager *manager);

	/**
	 * Get the interface for working with the Overlake SoC Nitro image.
	 *
	 * @param manager The flash manager to query.
	 *
	 * @return The Nitro image instance or null if the manager is not valid.
	 */
	struct overlake_soc_firmware* (*get_nitro_image) (struct overlake_flash_manager *manager);

	struct spi_flash *flash;							/**< The protected flash device. */
	const struct host_control *control;					/**< Control interface for host flash. */
	struct overlake_control *soc_control;				/**< Control interface for SoC and SoC flash. */
	const struct host_flash_initialization *flash_init;	/**< Initialization manager for SPI flash. */
	platform_mutex lock;								/**< Synchronization for flash operations. */
	struct overlake_soc_firmware boot;					/**< Boot firmware image. */
	struct overlake_soc_firmware nitro;					/**< Nitro firmware image. */
	uint8_t soc_mac_id[6];								/**< SoC MAC ID. */
	volatile uint32_t *boot_mode_cache;					/**< Host boot mode cache.  Lower half is used for SoC and Upper half is used for Cyclone FPGA */
	enum overlake_board_type board_type;				/**< Type of Overlake board. */
};


int overlake_flash_manager_init (struct overlake_flash_manager *manager, struct spi_flash *flash,
	const struct host_control *control, struct overlake_control *soc_control,
	const struct host_state_manager *host_state_boot,
	const struct host_state_manager *host_state_nitro, enum overlake_board_id board_id,
	volatile uint32_t *boot_mode_cache_reg, bool is_por);

int overlake_flash_manager_init_with_managed_flash_initialization (
	struct overlake_flash_manager *manager, struct spi_flash *flash,
	const struct host_control *control, struct overlake_control *soc_control,
	const struct host_state_manager *host_state_boot,
	const struct host_state_manager *host_state_nitro,
	const struct host_flash_initialization *flash_init, enum overlake_board_id board_id,
	volatile uint32_t *boot_mode_cache_reg, bool is_por);
void overlake_flash_manager_release (struct overlake_flash_manager *manager);

/* Internal function used by derived types */
int overlake_flash_manager_init_internal (struct overlake_flash_manager *manager,
	struct spi_flash *flash, const struct host_control *control,
	struct overlake_control *soc_control, const struct host_state_manager *host_state_boot,
	const struct host_state_manager *host_state_nitro, uint32_t boot_addr,
	uint32_t boot_backup_addr, uint32_t boot_version_addr, uint32_t boot_max_length,
	const char *boot_platform_id, uint32_t nitro_addr, uint32_t nitro_backup_addr,
	uint32_t nitro_version_addr, uint32_t nitro_max_length, const char *nitro_platform_id,
	enum overlake_board_type board_type);

int overlake_flash_manager_enable_rot_flash_access (struct overlake_flash_manager *manager);
int overlake_flash_manager_set_boot_mode (struct overlake_flash_manager *manager, uint32_t offset,
	uint8_t mode, bool restore_soc_flash_access);
int overlake_flash_manager_get_boot_mode (struct overlake_flash_manager *manager, uint32_t offset,
	uint8_t *mode);
void overlake_flash_manager_update_boot_mode_cache (struct overlake_flash_manager *manager,
	uint32_t offset, uint8_t mode);
void overlake_flash_manager_get_boot_mode_cache (struct overlake_flash_manager *manager,
	uint32_t offset, uint8_t *mode);
int overlake_flash_manager_validate_flash (const struct host_firmware *firmware,
	const struct pfm *pfm, const struct pfm *good_pfm, const struct hash_engine *hash,
	const struct rsa_engine *rsa);


#define	OVERLAKE_FLASH_MGR_ERROR(code)		ROT_ERROR (MSFT_MODULE_OVERLAKE_FLASH_MGR, code)

/**
 * Error codes that can be generated by a manager of Overlake flash.
 */
enum {
	OVERLAKE_FLASH_MGR_INVALID_ARGUMENT = OVERLAKE_FLASH_MGR_ERROR (0x00),			/**< Input parameter is null or not valid. */
	OVERLAKE_FLASH_MGR_NO_MEMORY = OVERLAKE_FLASH_MGR_ERROR (0x01),					/**< Memory allocation failed. */
	//OVERLAKE_FLASH_MGR_VALIDATE_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x02),				/**< An error unrelated to validation caused verification to fail. */
	//OVERLAKE_FLASH_MGR_RECOVER_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x03),				/**< An error unrelated to validation caused recovery to fail. */
	OVERLAKE_FLASH_MGR_ROT_ACCESS_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x04),			/**< Flash is not accessible by the RoT. */
	OVERLAKE_FLASH_MGR_SOC_ACCESS_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x05),			/**< Flash is not accessible by the SoC. */
	OVERLAKE_FLASH_MGR_BOOT_MODE_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x06),			/**< The SoC boot mode was not configured. */
	//OVERLAKE_FLASH_MGR_PREPARE_UPDATE_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x07),		/**< SoC flash is not ready to receive an update. */
	//OVERLAKE_FLASH_MGR_WRITE_UPDATE_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x08),			/**< SoC image data was not written to flash. */
	OVERLAKE_FLASH_MGR_UNKNOWN_BOOT_MODE = OVERLAKE_FLASH_MGR_ERROR (0x09),			/**< Attempt to set an unknown boot mode. */
	OVERLAKE_FLASH_MGR_NO_FLASH_ACCESS = OVERLAKE_FLASH_MGR_ERROR (0x0a),			/**< The host currently has access to the flash. */
	//OVERLAKE_FLASH_MGR_UPDATE_NOT_READY = OVERLAKE_FLASH_MGR_ERROR (0x0b),			/**< The system has not been prepared to receive an update. */
	OVERLAKE_FLASH_MGR_NO_TASK = OVERLAKE_FLASH_MGR_ERROR (0x0c),					/**< No manager command task is running. */
	OVERLAKE_FLASH_MGR_TASK_BUSY = OVERLAKE_FLASH_MGR_ERROR (0x0d),					/**< The command task is busy performing an operation. */
	OVERLAKE_FLASH_MGR_CANCEL_UPDATES_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x0e),		/**< Failed to cancel active updates. */
	OVERLAKE_FLASH_MGR_UPDATE_IN_PROGRESS = OVERLAKE_FLASH_MGR_ERROR (0x0f),		/**< SoC firmware update is running. */
	OVERLAKE_FLASH_MGR_SOC_MAC_ADDR_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x10),		/**< The SoC MAC address was not configured. */
	OVERLAKE_FLASH_MGR_SOC_DEBUG_LEVEL_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x11),	/**< The SoC debug level was not configured. */
	//OVERLAKE_FLASH_MGR_BACKUP_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x12),				/**< A backup image could not be created. */
	//OVERLAKE_FLASH_MGR_IMAGE_NOT_VALID = OVERLAKE_FLASH_MGR_ERROR (0x13),				/**< The active image is not known to be valid. */
	OVERLAKE_FLASH_MGR_UNKNOWN_BOARD_ID = OVERLAKE_FLASH_MGR_ERROR (0x14),			/**< Unknown SoC board ID. */
	OVERLAKE_FLASH_MGR_UNSUPPORTED = OVERLAKE_FLASH_MGR_ERROR (0x15),				/**< Operation not supported by flash manager */
	OVERLAKE_FLASH_MGR_PLATFORM_ID_MISMATCH = OVERLAKE_FLASH_MGR_ERROR (0x16),		/**< Platform ID of PFM used to validate Flash does not match with port's assigned platform ID. */
	OVERLAKE_FLASH_MGR_SOC_FW_VERSION_FAILED = OVERLAKE_FLASH_MGR_ERROR (0x17),		/**< SoC FW Version is not configured. */
	OVERLAKE_FLASH_MGR_UNKNOWN_IMAGE_SRC = OVERLAKE_FLASH_MGR_ERROR (0x18),			/**< Image source is not known. */
};


#endif	/* OVERLAKE_FLASH_MANAGER_H_ */
