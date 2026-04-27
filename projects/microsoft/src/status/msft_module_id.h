// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MSFT_MODULE_ID_H_
#define MSFT_MODULE_ID_H_

#include "status/rot_status.h"


/**
 * The IDs for Microsoft modules that can generate errors.
 *
 * Commented module IDs have been deprecated or promoted to a core module.  These IDs should not be
 * reused.
 */
enum {
	MSFT_MODULE_OVERLAKE_FLASH_MGR = 0x003f,				/**< Flash management for the Overlake SoC. */
	MSFT_MODULE_OVERLAKE_CONTROL = 0x0043,					/**< Driver interface for controlling the Overlake SoC. */
	MSFT_MODULE_HOST_FW_CMD = 0x0045,						/**< Handler for Overlake SoC firmware operations. */

	/* Earlier module IDs were in the 'core' numbering space.  Future modules should be defined
	 * starting at 0x1000 to provide isolation from 'core' modules. */
	MSFT_MODULE_CMD_HANDLER_MSFT = 0x1000,					/**< Handler for received MSFT protocol commands. */
	MSFT_MODULE_CMD_INTERFACE_MSFT_BMC_OBSERVER = 0x1001,	/**< MSFT BMC command interface observer. */
	MSFT_MODULE_JTAG_HANDLER = 0x1002,						/**< Handler for HSP JTAG messages. */
	MSFT_MODULE_FUSE_CONTROLLER = 0x1003,					/**< Driver for HSP fuses. */
	MSFT_MODULE_CCS_KSU = 0x1004,							/**< Driver for HSP secure key management. */
	MSFT_MODULE_JTAG_MAILBOX = 0x1005,						/**< Driver for the HSP JTAG mailbox. */
	MSFT_MODULE_DEVICE_KEYS = 0x1006,						/**< Device-unique key generation. */
	MSFT_MODULE_IDENTITY_RENEWAL = 0x1007,					/**< Driver for handling renewal of device identity. */
	// MSFT_MODULE_PKA = 0x1008,							/**< Driver for HSP public key hardware accelerator. */
	MSFT_MODULE_HS_SHA = 0x1009,							/**< Driver for the HSP hashing hardware accelerator. */
	MSFT_MODULE_HW_ROT = 0x100a,							/**< Interface for HW-backed RoT state. */
	MSFT_MODULE_HSP_FW_1SP = 0x100b,						/**< Handler for the 1SP firmware image. */
	// MSFT_MODULE_FIRMWARE_LOADER = 0x100c,				/**< Handler to load firmware images into memory. */
	MSFT_MODULE_HSP_FW_UTIL = 0x100d,						/**< Utilities for handling HSP firmware images. */
	MSFT_MODULE_LOAD_IMAGE_1SP = 0x100e,					/**< 1SP image loading. */
	MSFT_MODULE_BOOT_MEASUREMENTS = 0x100f,					/**< HSP ROM boot measurements. */
	MSFT_MODULE_HSP_AES = 0x1010,							/**< Driver for the HSP AES engine. */
	MSFT_MODULE_CHECKPOINT = 0x1011,						/**< Driver for the HSP checkpoint hardware. */
	MSFT_MODULE_HSP_RNG_HW = 0x1012,						/**< Driver for the HSP RNG hardware. */
	MSFT_MODULE_HSP_DMB = 0x1013,							/**< Driver for the HSP DMB hardware. */
	MSFT_MODULE_IRQ = 0x1014,								/**< Driver for traps and interrupts. */
	MSFT_MODULE_HSP_GPIO = 0x1015,							/**< Driver for the HSP GPIOs. */
	MSFT_MODULE_TEMP_SENSOR = 0x1016,						/**< Driver for temperature sensor. */
	MSFT_MODULE_I2C_DW_APB = 0x1017,						/**< Driver for DesignWare I2C device. */
	MSFT_MODULE_FORCED_RECOVERY_CPLD = 0x1018,				/**< Handler for host forced recovery. */
	MSFT_MODULE_CMC_PDI_LOADER = 0x1019,					/**< Handler to load PDIs using IPI. */
	MSFT_MODULE_HSP_AEB = 0x101a,							/**< Driver For the HSP AEBs. */
	MSFT_MODULE_MEMORY_PROTECTION = 0x101b,					/**< Manager for memory protection configuration. */
	MSFT_MODULE_CMC_FW_STORE = 0x101c,						/**< Handler for CMC firmware store operations. */
	MSFT_MODULE_TEMP_SENSOR_CLUSTER = 0x101d,				/**< Handler for a cluster of temperature sensors. */
	MSFT_MODULE_HSP_SRAM = 0x101e,							/**< Utilities for HSP SRAM operations. */
	MSFT_MODULE_CMC_IPI = 0x101f,							/**< Handler for CMC IPI interactions. */
	MSFT_MODULE_FUSE_SYNC_DATA = 0x1020,					/**< Module to enable fuse sync data preparation. */
	MSFT_MODULE_HSP_MAILBOX = 0x1021,						/**< Driver for HSP mailbox operation. */
	MSFT_MODULE_HSP_MAILBOX_PROTOCOL = 0x1022,				/**< Protocol layer to handle all mailbox communications. */
	MSFT_MODULE_HSP_MAILBOX_MSG_HANDLER = 0x1023,			/**< Handler to receive requests, process and send responses. */
	MSFT_MODULE_HSP_DUAL_DIE_JTAG_HANDLER = 0x1024,			/**< JTAG handler module for multi-die synchronization.  */
	MSFT_MODULE_CMC_RESET_HANDLER = 0x1025,					/**< Handler for CMC reset handling. */
	MSFT_MODULE_EXT_POWER = 0x1026,							/**< Driver for HSP Ext power operations. */
	MSFT_MODULE_CMC_HOST_UPDATE_CONTEXT = 0x1027,			/**< Handler for CMC host update processing. */
	MSFT_MODULE_PUF_INFRA = 0x1028,							/**< PUF infrastructure module. */
	MSFT_MODULE_HSP_WATCHDOG = 0x1029,						/**< Hardware watchdog timer for HSP. */
	MSFT_MODULE_CPLD_MEASUMERENT = 0x102a,					/**< Handler for measuring CPLD flash. */
	MSFT_MODULE_CMC_HOST_UPDATE_NOTIFY = 0x102b,			/**< Notifications for host updates */
	MSFT_MODULE_HSP_CRASHDUMP_HANDLER = 0x102c,				/**< Handler for HSP crashdump. */
	MSFT_MODULE_OVERLAKE_PERIODIC_RESET = 0x102d,			/**< Overlake Cerberus periodic reset module */
	MSFT_MODULE_ZLIB_INFRA = 0x102e,						/**< Zlib decompression infra */
	MSFT_MODULE_CMVP_TESTING = 0x102f,						/**< CMVP test case handling. */
	MSFT_MODULE_HSP_HANDLER = 0x1030,						/**< Handler for HW error processing. */
	MSFT_MODULE_HSP_HANDLER_ACCESS_ERR = 0x1031,			/**< Handler for processing access error. */
	MSFT_MODULE_HSP_HANDLER_BUS_ERR = 0x1032,				/**< Handler for processing bus error. */
	MSFT_MODULE_HSP_HANDLER_CHK_ERR = 0x1033,				/**< Handler for processing HW check point error. */
	MSFT_MODULE_HSP_HANDLER_DMB_ERR = 0x1034,				/**< Handler for processing DMB error. */
	MSFT_MODULE_HSP_HANDLER_MEM_ERR = 0x1035,				/**< Handler for processing memory error. */
	MSFT_MODULE_HSP_HANDLER_MPU_ERR = 0x1036,				/**< Handler for processing MPU error. */
	MSFT_MODULE_HSP_HANDLER_RNG_ERR = 0x1037,				/**< Handler for processing RNG error. */
	MSFT_MODULE_HSP_HANDLER_WATCHDOG_TIMEOUT = 0x1038,		/**< Handler for processing watchdog timeout. */
	MSFT_MODULE_HSP_HANDLER_WDT_ERR = 0x1039,				/**< Handler for processing WDT error. */
	MSFT_MODULE_FIPS_SELF_TEST = 0x103a,					/**< FIPS self-test handling. */
	MSFT_MODULE_CMC_HEARTBEAT_MONITOR = 0x103b,				/**< Handler for CMC heartbeat monitoring. */
	MSFT_MODULE_OMC_FLASH_MGR = 0x103c,						/**< OVL3 SOC flash manager. */
};


#endif	/* MSFT_MODULE_ID_H_ */
