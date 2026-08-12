// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_LOGGING_H_
#define INIT_LOGGING_H_

#include "logging/debug_log.h"


/**
 * Error messages that can be logged during initialization.
 *
 * Note: Commented message have been deprecated.
 */
enum {
	INIT_LOGGING_BOOT_SOURCE = 0,				/**< Indicate the flash device used for booting. */
	INIT_LOGGING_HW_SHA_ERROR,					/**< Error initializing SHA hardware block. */
	INIT_LOGGING_HW_AES_ERROR,					/**< Error initializing AES hardware block. */
	INIT_LOGGING_INIT_HASH,						/**< Error allocating init task hash engine. */
	INIT_LOGGING_INIT_RSA,						/**< Error initializing init task RSA engine. */
	INIT_LOGGING_RUNNING_IMG,					/**< Error with the running image. */
	INIT_LOGGING_FW_UPDATER,					/**< Error with the firmware updater. */
	INIT_LOGGING_DEVICE_PROVISIONING,			/**< Error provisioning device OTP. */
	INIT_LOGGING_RIOT_MANAGER,					/**< Error initializing RIoT keys. */
	INIT_LOGGING_AUX_ATTESTATION,				/**< Error initializing auxiliary attestation. */
	INIT_LOGGING_SYSTEM_STATE,					/**< Error initializing system state. */
	INIT_LOGGING_CFM_MANAGEMENT,				/**< Error initializing CFMs. */
	INIT_LOGGING_GPIO_IRQ,						/**< Error initializing GPIO IRQ handlers. */
	INIT_LOGGING_PROGRAM_CPLD,					/**< Error programming the CPLD SPI filter. */
	INIT_LOGGING_CPLD_NOT_DONE,					/**< CPLD DONE was not set at start of programming. */
	INIT_LOGGING_INIT_CPLD,						/**< Error initializing CPLD communication. */
	INIT_LOGGING_HOST_FLASH,					/**< Error initializing access to host flash. */
	INIT_LOGGING_HOST_FW,						/**< Error initializing management of host firmware. */
	INIT_LOGGING_CPLD_IRQ,						/**< Error initializing CPLD IRQs. */
	INIT_LOGGING_HOST_SPI_FREQ,					/**< Error setting the host SPI frequency. */
	INIT_LOGGING_START_HOST,					/**< Error validating and starting the host. */
	INIT_LOGGING_FW_UPDATE_TASK,				/**< Error starting the firmware updater. */
	INIT_LOGGING_PFM_TASK,						/**< Error starting PFM management. */
	INIT_LOGGING_CFM_TASK,						/**< Error starting CFM management. */
	INIT_LOGGING_COMMAND_HANDLER,				/**< Error intializing the command handler. */
//	INIT_LOGGING_AUX_KEY,						/**< Done generating auxiliary attestation key. */
	INIT_LOGGING_FILTER_CONFIG = 26,			/**< The SPI filter configuration. */
	INIT_LOGGING_PORT0_PFMS_BEFORE_POR,			/**< Initial state of PFMs for port 0. */
	INIT_LOGGING_PORT0_PFMS_AFTER_POR,			/**< State of port 0 PFMs after POR processing. */
	INIT_LOGGING_PORT1_PFMS_BEFORE_POR,			/**< Initial state of PFMs for port 1. */
	INIT_LOGGING_PORT1_PFMS_AFTER_POR,			/**< State of port 1 PFMs after POR processing. */
	INIT_LOGGING_CPLD_VERSION,					/**< The CPLD version executing. */
	INIT_LOGGING_LOG_TASK,						/**< Error starting the task to flush the log. */
	INIT_LOGGING_TCG_LOG,						/**< Error initializing TCG log module. */
	INIT_LOGGING_PCR_STORE,						/**< Error initializing PCR store. */
	INIT_LOGGING_PCR_STORE_UPDATE_DIGEST,		/**< Error adding digest to PCR. */
	INIT_LOGGING_PCR_STORE_UPDATE_BUFFER,		/**< Error adding buffer to PCR. */
	INIT_LOGGING_HEARTBEAT,						/**< Error starting the RoT heartbeat. */
	INIT_LOGGING_CPLD_FLASH_PROGRAM,			/**< Error programming the CPLD flash. */
	INIT_LOGGING_RESTORE_CONTEXT,				/**< Error restoring context after reset. */
	INIT_LOGGING_SAVE_CONTEXT,					/**< Error saving the context during POR. */
	INIT_LOGGING_HOST_TASK,						/**< Error creating handler task for host firmware. */
	INIT_LOGGING_PROGRAM_CPLD_FLASH,			/**< CPLD flash programming has started. */
	INIT_LOGGING_FORCED_CPLD_FLASH,				/**< CPLD flash programming was forced to start. */
//	INIT_LOGGING_ENTER_RESET,					/**< Detected host reset. */
//	INIT_LOGGING_EXIT_RESET,					/**< Detected host out of reset. */
	INIT_LOGGING_PCR_VERIFY = 46,				/**< Error verifying shared PCR values. */
	INIT_LOGGING_INIT_SEQUENCING,				/**< Error initializing reset sequence control. */
	INIT_LOGGING_FW_VERSION,					/**< Running version of firmware. */
	INIT_LOGGING_INIT_X509,						/**< Error initialize init task X.509 engine. */
	INIT_LOGGING_HOST_UPDATE_TASK,				/**< Error starting host FW update management. */
	INIT_LOGGING_CONFIG_MGMT,					/**< Error initializing configuration management. */
	INIT_LOGGING_OVERLAKE_COMMAND_HANDLER,		/**< Error starting the Overlake command handler. */
	INIT_LOGGING_PCD_MANAGEMENT,				/**< Error initializing PCDs. */
	INIT_LOGGING_PCD_TASK,						/**< Error starting PCD management. */
	INIT_LOGGING_STATE_PERSISTENCE,				/**< Error initializing state persistence. */
	INIT_LOGGING_TPM,							/**< Error initializing TPM. */
	INIT_LOGGING_RIOT_KEY_TOO_BIG,				/**< A RIoT key was too large for the reserved buffer. */
	INIT_LOGGING_INIT_CONFIG_CMD_TASK,			/**< Error initializing command task context. */
	INIT_LOGGING_CONFIG_CMD_TASK,				/**< Error starting command task context. */
	INIT_LOGGING_PERSISTENCE_TASK,				/**< Error starting task to flush persistent data to flash. */
	INIT_LOGGING_PORT0_RECOVERY_IMAGE,			/**< State of port 0 recovery image. */
	INIT_LOGGING_PORT1_RECOVERY_IMAGE,			/**< State of port 1 recovery image. */
	INIT_LOGGING_HOST_PCR_UPDATER,				/**< Failed to initialize host PCR updating. */
	INIT_LOGGING_SB_RECOVERY_IMAGE,				/**< State of the SB recovery image on SPI flash. */
	INIT_LOGGING_FLASH_DEVICE,					/**< Information about a flash device in the system. */
	INIT_LOGGING_HOST_FLASH_INIT,				/**< Error initializing host flash devices. */
//	INIT_LOGGING_GENERATE_AUX_KEY,				/**< Generating auxiliary attestation key. */
	INIT_LOGGING_SYSTEM_CRYPTO = 68,			/**< Error initializing system crypto engines. */
	INIT_LOGGING_SESSION_MANAGEMENT,			/**< Error initializing session manager. */
	INIT_LOGGING_TPM_DATA_COPY,					/**< Error migrating TPM storage data. */
	INIT_LOGGING_RECOVERY_IMAGE,				/**< Error validating the recovery image. */
	INIT_LOGGING_RESTORE_CPLD_MEASUREMENTS,		/**< Failed to restore CPLD measurement data. */
	INIT_LOGGING_RESET_PROCESSING_TIME,			/**< Time to execute reset processing. */
	INIT_LOGGING_BOARD_ID,						/**< Board ID. */
	INIT_LOGGING_NO_RIOT_KEY_DATA_AVAILABLE,	/**< A RIoT key data was not available. */
	INIT_LOGGING_HEAP_USAGE,					/**< Current heap usage information. */
	INIT_LOGGING_PORT_CONFIG,					/**< Error getting host port configuration. */
	INIT_LOGGING_ROT_CONFIG,					/**< Error getting the RoT configuration. */
	INIT_LOGGING_SECURE_STORAGE,				/**< Error initializing secure storage. */
	INIT_LOGGING_INIT_INTRUSION,				/**< Error initializing chassis intrusion. */
	INIT_LOGGING_HOST_FW_CMD_TASK,				/**< Error Initializing host firmware command task. */
	INIT_LOGGING_PORT2_PFMS_BEFORE_POR,			/**< Initial state of PFMs for port 2. */
	INIT_LOGGING_PORT2_PFMS_AFTER_POR,			/**< State of port 2 PFMs after POR processing. */
	INIT_LOGGING_PORT_RESET_WDT_ERROR,			/**< Error while initializing a host boot watchdog timer. */
	INIT_LOGGING_RESTORE_EID_FAIL,				/**< Error while restoring saved EID. */
	INIT_LOGGING_PCD_COMPONENT_ERROR,			/**< Error getting components from PCD. */
	INIT_LOGGING_PCD_COMPONENT,					/**< PCD component added to attestable devices list. */
	INIT_LOGGING_START_INTRUSION,				/**< Error starting chassis intrusion task. */
	INIT_LOGGING_COMMAND_HANDLER_START,			/**< Error starting the command handler. */
	INIT_LOGGING_ENABLE_FORCED_RECOVERY,		/**< Enabled CPLD based forced host recovery. */
	INIT_LOGGING_FORCED_RECOVERY_ERROR,			/**< Failed to force host recovery. */
	INIT_LOGGING_HW_CRYPTO,						/**< Error initializing HW crypto blocks. */
	INIT_LOGGING_INIT_SPDM_RESPONDER,			/**< Error initializing SPDM responder */
	INIT_LOGGING_INIT_IPC,						/**< Error initializing IPC communication */
	INIT_LOGGING_CMC_LOADER,					/**< Error while loading partial PDIs */
	INIT_LOGGING_IPC_SYNC,						/**< Error synchronizing boot between multiple cores. */
	INIT_LOGGING_TDISP,							/**< Error while initializing PCIE modules (IDE/TDISP) */
	INIT_LOGGING_INIT_EPHEMERAL_KEY_MANAGER,	/**< Error initializing ephemeral key manager components */
	INIT_LOGGING_START_EPHEMERAL_KEY_MANAGER,	/**< Error while starting ephemeral key manager task */
	INIT_LOGGING_DME_PUBLIC_KEY,				/**< Failed to prepare the DME public key for export. */
	INIT_LOGGING_INIT_ACVP,						/**< Error initializing ACVP. */
	INIT_LOGGING_PERIODIC_RESET_TIMER_ERROR,	/**< Failed to initialize periodic reset timer. */
	INIT_LOGGING_CRYPTO_KAT,					/**< Error while running crypto self-tests. */
	INIT_LOGGING_INIT_ERROR_STATE_TASK,			/**< Error while initializing the FIPS error state handler. */
	INIT_LOGGING_START_ERROR_STATE_TASK,		/**< Error starting the FIPS error state task. */
	INIT_LOGGING_INIT_SOC,						/**< Error initializing the SoC for normal operation. */
	INIT_LOGGING_MPU_PROTECTION,				/**< Error while applying MPU configuration. */
	INIT_LOGGING_SOC_TELEMETRY,					/**< Error initializing telemetry flush handlers. */
	INIT_LOGGING_INIT_CRASHDUMP_HANDLER,		/**< Error initializing crashdump handler. */
	INIT_LOGGING_INIT_EPHEMERAL_KEY_MONITOR,	/**< Error initializing the ephemeral key monitor. */
	INIT_LOGGING_START_EPHEMERAL_KEY_MONITOR,	/**< Error starting the ephemeral key monitor task. */
};

/**
 * Chip reset sources.
 */
enum {
	RESET_POR,			/**< The chip has received power for the first time. */
	RESET_EXTERNAL,		/**< Reset was triggered using the external reset pin. */
	RESET_WATCHDOG,		/**< The watchdog expired and reset the chip. */
	RESET_BROWN_OUT,	/**< Brown-out was detected. */
	RESET_SOFT,			/**< An internal software reset was triggered. */
	RESET_UNKNOWN,		/**< No reset source was indicated. */
	RESET_SOFTWARE,		/**< Software caused the last reset. */
};

/**
 * IDs for logging errors passing RIoT keys and certificates.
 */
enum {
	INIT_RIOT_KEY_DEVICE_ID = 0,	/**< Device ID certificate. */
	INIT_RIOT_KEY_DEVICE_ID_CSR,	/**< Device ID CSR. */
	INIT_RIOT_KEY_ALIAS_KEY,		/**< Alias key pair. */
	INIT_RIOT_KEY_ALIAS_CERT,		/**< Alias certificate. */
	INIT_RIOT_KEY_ATTESTATION_CERT,	/**< Host attestation certificate. */
};

/**
 * IDs for the different flash devices being reported.
 */
enum {
	INIT_FLASH_CERBERUS_MAIN = 0,	/**< Cerberus flash with the main firmware image. */
	INIT_FLASH_CERBERUS_RECOVERY,	/**< Cerberus flash with the recovery firmware image. */
	INIT_FLASH_PORT0_CS0,			/**< Host flash on filter port 0, chip select 0. */
	INIT_FLASH_PORT0_CS1,			/**< Host flash on filter port 0, chip select 1. */
	INIT_FLASH_PORT1_CS0,			/**< Host flash on filter port 1, chip select 0. */
	INIT_FLASH_PORT1_CS1,			/**< Host flash on filter port 1, chip select 1. */
	INIT_FLASH_PORT2_CS0,			/**< Host flash on filter port 2, chip select 0. */
};

/**
 * IDs for different reset processing contexts.
 */
enum {
	CERBERUS_RESET_PROCESSING = 0,	/**< Processing during a Cerberus reset. */
	WARM_RESET_PROCESSING,			/**< Processing during a host warm reset. */
	HOST_UP_PROCESSING,				/**< Processing during a host up notification. */
	BMC_CS1_PROCESSING,				/**< Processing when receiving a BMC CS1 notification. */
};


#endif	/* INIT_LOGGING_H_ */
