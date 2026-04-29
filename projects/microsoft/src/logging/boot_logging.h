// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef BOOT_LOGGING_H_
#define BOOT_LOGGING_H_

#include "logging/debug_log.h"


/**
 * Error messages that can be logged during boot.
 */
enum {
	BOOT_LOGGING_HW_SHA_ERROR = 0,			/**< Error initializing SHA hardware block. */
	BOOT_LOGGING_INIT_HASH,					/**< Error initializing hash engine. */
	BOOT_LOGGING_INIT_RSA,					/**< Error initializing RSA engine. */
	BOOT_LOGGING_INIT_ECC,					/**< Error initializing ECC engine. */
	BOOT_LOGGING_INIT_X509,					/**< Error initializing X.509 engine. */
	BOOT_LOGGING_INIT_BASE64,				/**< Error initializing base64 engine. */
	BOOT_LOGGING_HW_AES_ERROR,				/**< Error initializing AES hardware block. */
	BOOT_LOGGING_INIT_RIOT_CORE,			/**< Error initializing RIoT Core. */
	BOOT_LOGGING_INIT_PUF,					/**< Error initializing PUF keystore. */
	BOOT_LOGGING_APP_LOAD,					/**< Error loading the main application .*/
	BOOT_LOGGING_ATTESTATION_KEY,			/**< Error loading the attestation key. */
	BOOT_LOGGING_RIOT_KEYS,					/**< Error saving RIoT keys. */
	BOOT_LOGGING_SVN_ERROR,					/**< Error determining TCB SVN information. */
	BOOT_LOGGING_RECOVERY_LOAD,				/**< Loading main image from recovery flash. */
	BOOT_LOGGING_IMAGE_RECOVERY_START,		/**< Main image recovery has started. */
	BOOT_LOGGING_IMAGE_RECOVERY_COMPLETED,	/**< Main image recovery completed successfully. */
	BOOT_LOGGING_ATTESTATION_CERT,			/**< Error generating the attestation certificate. */
	BOOT_LOGGING_RECOVERY_APP_LOAD,			/**< Load main application from the recovery image. */
	BOOT_LOGGING_FORCE_EXT_RECOVERY,		/**< Force the chip to enter external recovery mode. */
	BOOT_LOGGING_INIT_RNG,					/**< Error initializing RNG engine. */
	BOOT_LOGGING_ROM_LOG_ERROR,				/**< Error persisting ROM log messages. */
	BOOT_LOGGING_INIT_HW_CRYPTO,			/**< Error initializing HW crypto blocks. */
	BOOT_LOGGING_INIT_SECURITY_POLICY,		/**< Error initializing the security policy. */
	BOOT_LOGGING_APPLY_SECURITY_CONFIG,		/**< Error applying the security config. */
	BOOT_LOGGING_INCORRECT_DICE_KEY,		/**< The DICE key was generated incorrectly. */
	BOOT_LOGGING_INITIALIZE_PCRS,			/**< Error initializing run-time PCR state. */
	BOOT_LOGGING_UPDATE_PCRS,				/**< Error updating run-time PCR state. */
	BOOT_LOGGING_CRYPTO_KAT,				/**< Error while running crypto self-tests. */
	BOOT_LOGGING_UNEXPECTED_PCR_STATE,		/**< The device PCRs are not in the expected state. */
	BOOT_LOGGING_UNEXPECTED_MEASUREMENT,	/**< A cached measurement was not the expected value. */
	BOOT_LOGGING_MPU_PROTECTION,			/**< Error while applying MPU protection */
	BOOT_LOGGING_PREPARE_MAIN_EXECUTION,	/**< Failed to prepare the device for main firmware. */
	BOOT_LOGGING_INTEGRITY_CHECK_FAIL,		/**< Integrity check of the boot image failed. */
	BOOT_LOGGING_UNEXPECTED_EXECUTION,		/**< A code path integrity failure was detected. */
};


#endif	/* BOOT_LOGGING_H_ */
