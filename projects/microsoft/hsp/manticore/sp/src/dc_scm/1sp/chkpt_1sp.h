// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CHKPT_1SP_H_
#define CHKPT_1SP_H_

// *INDENT-OFF*
/* Include the parts from the auto-generated header files.  Include ordering is important. */
#include "checkpoint/precomp.h"
#include "checkpoint/chkpt_1sp_init.h"
#include "checkpoint/chkpt_1sp_load_external.h"
#include "checkpoint/chkpt_1sp_load_internal.h"
#include "checkpoint/chkpt_1sp_sprt_execute.h"
#include "status/module_id.h"
// *INDENT-ON*


/**
 * The value to use for a specific checkpoint step.  This uses the INIT module ID to provide
 * uniqueness.
 */
#define	CHKPT_1SP_VALUE(x)		((ROT_MODULE_INIT << 8) | (x))


/**
 * Values that are fed into the checkpoint engine during 1SP execution.
 */
enum {
	CHKPT_1SP_RNG_INIT = CHKPT_1SP_VALUE (0x01),					/**< RNG initialization has completed. */
	CHKPT_1SP_BOOT_MESSAGES = CHKPT_1SP_VALUE (0x02),				/**< UART boot messages generated. */
	CHKPT_1SP_RESET_CNTR_INIT = CHKPT_1SP_VALUE (0x03),				/**< Reset counters have been initialized. */
	CHKPT_1SP_AEB_INIT = CHKPT_1SP_VALUE (0x04),					/**< AEB driver is initialized. */
	CHKPT_1SP_SILICON_ERRATA = CHKPT_1SP_VALUE (0x05),				/**< Silicon errata workarounds applied. */
	CHKPT_1SP_FLASH_INIT = CHKPT_1SP_VALUE (0x06),					/**< Flash access is initialized. */
	CHKPT_1SP_LOG_INIT = CHKPT_1SP_VALUE (0x07),					/**< Debug log is initialized. */
	CHKPT_1SP_DICE_VALID = CHKPT_1SP_VALUE (0x08),					/**< The DICE key is determined to be valid. */
	CHKPT_1SP_CRYPTO_INIT_START = CHKPT_1SP_VALUE (0x09),			/**< Start initialization of crypto drivers. */
	CHKPT_1SP_CRYPTO_INIT_END = CHKPT_1SP_VALUE (0x0a),				/**< End initialization of crypto drivers. */
	CHKPT_1SP_CRYPTO_INIT_DONE = CHKPT_1SP_VALUE (0x0b),			/**< Crypto drivers are initialized. */
	CHKPT_1SP_FIPS_INTEGRITY_START = CHKPT_1SP_VALUE (0x0c),		/**< Run 1SP integrity check. */
	CHKPT_1SP_FIPS_CAST_ECDSA_1SP_VERIFY = CHKPT_1SP_VALUE (0x0d),	/**< The ECDSA verify self-test for 1SP was run. */
	CHKPT_1SP_FIPS_INTEGRITY_END = CHKPT_1SP_VALUE (0x0e),			/**< Finished 1SP integrity check. */
	CHKPT_1SP_FIPS_INTEGRITY_DONE = CHKPT_1SP_VALUE (0x0f),			/**< Done running 1SP integrity check. */
	CHKPT_1SP_FIPS_CAST_START = CHKPT_1SP_VALUE (0x10),				/**< Start FIPS crypto self-tests. */
	CHKPT_1SP_FIPS_CAST_DRBG = CHKPT_1SP_VALUE (0x11),				/**< The DRBG self-test was run. */
	CHKPT_1SP_FIPS_CAST_KDF = CHKPT_1SP_VALUE (0x12),				/**< The KDF self-test was run. */
	CHKPT_1SP_FIPS_CAST_ECDSA_PKA_SIGN = CHKPT_1SP_VALUE (0x13),	/**< The ECDSA sign self-test using PKA was run. */
	CHKPT_1SP_FIPS_CAST_ECDSA_PKA_VERIFY = CHKPT_1SP_VALUE (0x14),	/**< The ECDSA verify self-test using PKA was run. */
	CHKPT_1SP_FIPS_CAST_ECDSA_ECC_VERIFY = CHKPT_1SP_VALUE (0x15),	/**< The ECDSA verify self-test for SPRT was run. */
	CHKPT_1SP_FIPS_CAST_DONE = CHKPT_1SP_VALUE (0x16),				/**< Done running FIPS crypto self-tests. */
	CHKPT_1SP_STACK_GUARD_SET = CHKPT_1SP_VALUE (0x17),				/**< Configure the stack guard cookie. */
	CHKPT_1SP_MPU_CONFIGURED = CHKPT_1SP_VALUE (0x18),				/**< The MPU has been configured for 1SP. */
	CHKPT_1SP_DICE_INIT_START = CHKPT_1SP_VALUE (0x19),				/**< Start DICE initialization. */
	CHKPT_1SP_FIPS_KEYS_DONE = CHKPT_1SP_VALUE (0x1a),				/**< Coverted to FIPS approved keys. */
	CHKPT_1SP_DICE_INIT_END = CHKPT_1SP_VALUE (0x1b),				/**< End DICE initialization. */
	CHKPT_1SP_DICE_INIT_DONE = CHKPT_1SP_VALUE (0x1c),				/**< DICE is initialized. */
	CHKPT_1SP_SEC_POLICY_INIT_START = CHKPT_1SP_VALUE (0x1d),		/**< Start security policy initialization. */
	CHKPT_1SP_SEC_POLICY_LOADED = CHKPT_1SP_VALUE (0x1e),			/**< Current security policy has been loaded. */
	CHKPT_1SP_UNLOCKED_KEYS = CHKPT_1SP_VALUE (0x1f),				/**< Management of device keys with unlock policies. */
	CHKPT_1SP_SEC_POLICY_INIT_END = CHKPT_1SP_VALUE (0x20),			/**< End security policy initialization. */
	CHKPT_1SP_SEC_POLICY_INIT_DONE = CHKPT_1SP_VALUE (0x21),		/**< Security policy is initialized. */
	CHKPT_1SP_BOOTLOADER_INIT_START = CHKPT_1SP_VALUE (0x22),		/**< Start bootloader initialization. */
	CHKPT_1SP_BOOTLOADER_INTERNAL = CHKPT_1SP_VALUE (0x23),			/**< Initialize bootloader for internal flash. */
	CHKPT_1SP_BOOTLOADER_EXTERNAL = CHKPT_1SP_VALUE (0x24),			/**< Initialize bootloader for external flash. */
	CHKPT_1SP_BOOTLOADER_INIT_END = CHKPT_1SP_VALUE (0x25),			/**< End bootloader initialization. */
	CHKPT_1SP_BOOTLOADER_INIT_DONE = CHKPT_1SP_VALUE (0x26),		/**< Bootloader is initialized. */
	CHKPT_1SP_FW_LOAD_START = CHKPT_1SP_VALUE (0x27),				/**< Start run-time firmware loading. */
	CHKPT_1SP_FW_LOAD_INTERNAL = CHKPT_1SP_VALUE (0x28),			/**< Load firmware from internal flash. */
	CHKPT_1SP_FW_LOAD_EXTERNAL = CHKPT_1SP_VALUE (0x29),			/**< Load firmware from external flash. */
	CHKPT_1SP_FW_LOADED = CHKPT_1SP_VALUE (0x2a),					/**< Run-time firmware has been loaded. */
	CHKPT_1SP_FW_LOAD_END = CHKPT_1SP_VALUE (0x2b),					/**< End run-time firmware loading. */
	CHKPT_1SP_FW_LOAD_DONE = CHKPT_1SP_VALUE (0x2c),				/**< Run-time firmware loading is done. */
	CHKPT_1SP_MEASURE_INTERNAL = CHKPT_1SP_VALUE (0x2d),			/**< Measure firmware from internal flash. */
	CHKPT_1SP_MEASURE_EXTERNAL = CHKPT_1SP_VALUE (0x2e),			/**< Measure firmware from external flash. */
	CHKPT_1SP_MEASURE_END = CHKPT_1SP_VALUE (0x2f),					/**< End firmware measurements. */
	CHKPT_1SP_MEASURE_DONE = CHKPT_1SP_VALUE (0x30),				/**< Loaded firmware has been measured. */
	CHKPT_1SP_SPRT_DICE_START = CHKPT_1SP_VALUE (0x31),				/**< Start SPRT DICE execution. */
	CHKPT_1SP_SPRT_DICE_END = CHKPT_1SP_VALUE (0x32),				/**< End SPRT DICE execution. */
	CHKPT_1SP_SPRT_DICE_DONE = CHKPT_1SP_VALUE (0x33),				/**< SPRT DICE keys are generated. */
	CHKPT_1SP_CP_DICE_START = CHKPT_1SP_VALUE (0x34),				/**< Start HSM DICE execution. */
	CHKPT_1SP_CP_DICE_END = CHKPT_1SP_VALUE (0x35),					/**< End HSM DICE execution. */
	CHKPT_1SP_CP_DICE_DONE = CHKPT_1SP_VALUE (0x36),				/**< HSM DICE keys are generated. */
	CHKPT_1SP_SPRT_DICE_RELEASE = CHKPT_1SP_VALUE (0x37),			/**< SPRT DICE keys have been zeroized. */
	CHKPT_1SP_CP_DICE_RELEASE = CHKPT_1SP_VALUE (0x38),				/**< CP DICE keys have been zeroized. */
	CHKPT_1SP_APPLY_SECURITY_CFG = CHKPT_1SP_VALUE (0x39),			/**< Security policy configuration has been applied. */
	CHKPT_1SP_MEASURE_AEB_START = CHKPT_1SP_VALUE (0x3a),			/**< Start measuring AEB state. */
	CHKPT_1SP_MEASURE_AEB_END = CHKPT_1SP_VALUE (0x3b),				/**< End measuring AEB state. */
	CHKPT_1SP_MEASURE_AEB_DONE = CHKPT_1SP_VALUE (0x3c),			/**< AEB state has been measured. */
	CHKPT_1SP_SPRT_SHARED_DATA_READY = CHKPT_1SP_VALUE (0x3d),		/**< SPRT shared data is ready. */
	CHKPT_1SP_CP_SHARED_DATA_READY = CHKPT_1SP_VALUE (0x3e),		/**< CP shared data is ready. */
	CHKPT_1SP_SPRT_READY = CHKPT_1SP_VALUE (0x3f),					/**< Ready to execute main firmware. */
};


#endif	/* CHKPT_1SP_H_ */
