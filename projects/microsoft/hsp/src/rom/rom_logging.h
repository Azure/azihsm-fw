// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef ROM_LOGGING_H_
#define ROM_LOGGING_H_

#include "logging/msft_debug_log.h"
#include "recovery/ocp_recovery_device.h"


/**
 * The component ID for HSP ROM.
 */
#define	DEBUG_LOG_COMPONENT_HSP_ROM		MSFT_LOGGING_COMPONENT_HSP_ROM

/**
 * Logging messages for HSP ROM.
 */
enum {
	ROM_LOGGING_FAIL_INIT = 0x00,							/**< General ROM initialization failure. */
	ROM_LOGGING_FAIL_KAT = 0x01,							/**< Crypto HW KAT failure. */
	ROM_LOGGING_FAIL_JTAG_HANDLER = 0x02,					/**< Failure processing a JTAG mailbox message. */
	ROM_LOGGING_FAIL_TRANSITION_TO_TEST = 0x03,				/**< Failed to transition security state to Test. */
	ROM_LOGGING_FAIL_TRANSITION_TO_PROD = 0x04,				/**< Failed to transition security state to Production. */
	ROM_LOGGING_FAIL_TRANSITION_TO_SECURE = 0x05,			/**< Failed to transition security state to Secure. */
	ROM_LOGGING_FAIL_DEVICE_KEYS = 0x06,					/**< Device key derivation failure. */
	ROM_LOGGING_FAIL_SLOT_A = 0x07,							/**< Failed to load an image from boot slot A. */
	ROM_LOGGING_FAIL_SLOT_B = 0x08,							/**< Failed to load an image from boot slot B. */
	ROM_LOGGING_FAIL_EXT_FLASH = 0x09,						/**< Failed to load an image from external flash. */
	ROM_LOGGING_FAIL_SVN_UPDATE = 0x0a,						/**< Failed to update the fused SVN value. */
	ROM_LOGGING_FAIL_OWNER_TRANSFER = 0x0b,					/**< Failed to update the owner root key. */
	ROM_LOGGING_FAIL_TENANT_TRANSFER = 0x0c,				/**< Failed to execute a tenancy grant or revocation. */
	ROM_LOGGING_FAIL_MEASUREMENTS = 0x0d,					/**< Update measurement logs and PCRs failure. */
	ROM_LOGGING_FAIL_DICE = 0x0e,							/**< DICE identity derivation failure. */
	ROM_LOGGING_FAIL_DME = 0x0f,							/**< DME endorsement for DICE key failure. */
	ROM_LOGGING_FAIL_RECOVERY = 0x10,						/**< Failed to load a recovery image from memory. */
	ROM_LOGGING_EXCEPTION = 0x11,							/**< An exception occurred during the previous ROM boot. */
	ROM_LOGGING_STACK_CORRUPTION = 0x12,					/**< Stack corruption was detected during the previous ROM boot. */
	ROM_LOGGING_UNHANDLED_SYSCALL = 0x13,					/**< An unhandled syscall was executed during the previous ROM boot. */
	ROM_LOGGING_FATAL_HW_ERROR = 0x14,						/**< A fatal HW error occurred during the previous ROM boot. */
	ROM_LOGGING_FAIL_RSVD1_CACHE = 0x15,					/**< Failed to read RSVD1 fuses. */
	ROM_LOGGING_FAIL_FUSE_SYNC = 0x16,						/**< Failed to read fuses for multi-die sync. */
	ROM_LOGGING_FAIL_DMB_MAPPING = 0x17,					/**< Failed to map DMB. */
	ROM_LOGGING_FAIL_HSP_MAILBOX_MSG_HANDLER = 0x18,		/**< Failed in mailbox message handler. */
	ROM_LOGGING_FAIL_HSP_MAILBOX_MSG_SEND_FAILED = 0x19,	/**< Failed to send mailbox message. */
	ROM_LOGGING_FAIL_HSP_MAILBOX_MSG_RECV_FAILED = 0x1A,	/**< Failed to receive mailbox message. */
	ROM_LOGGING_FAIL_EXT_POWER = 0x1B,						/**< External power failure. */
	ROM_LOGGING_FAIL_FUSE_CMD = 0x1C,						/**< Failed to perform a fuse cmd. */
};

/**
 * Message codes for tracing ROM execution.
 */
enum {
	ROM_LOGGING_TRACE_START = 0x80000000,							/**< Start ROM execution. */
	ROM_LOGGING_TRACE_HW_INIT = 0x80000001,							/**< Initialize hardware and internal structures. */
	ROM_LOGGING_TRACE_INITIALIZE_KEYS = 0x80000002,					/**< Initialize the device keys. */
	ROM_LOGGING_TRACE_JTAG_MESSAGE = 0x80000003,					/**< Handle a message present in the JTAG mailbox. */
	ROM_LOGGING_TRACE_START_FLASH_LOAD = 0x80000004,				/**< Start to load a 1SP image from flash. */
	ROM_LOGGING_TRACE_LOAD_IMAGE_SLOT_A = 0x80000005,				/**< Check internal flash slot A for an image. */
	ROM_LOGGING_TRACE_LOAD_IMAGE_SLOT_B = 0x80000006,				/**< Check internal flash slot B for an image. */
	ROM_LOGGING_TRACE_LOAD_IMAGE_EXT = 0x80000007,					/**< Check external flash for an image. */
	ROM_LOGGING_TRACE_ENTER_RECOVERY = 0x80000008,					/**< Enter I2C recovery mode. */
	ROM_LOGGING_TRACE_LOAD_IMAGE_REC = 0x80000009,					/**< Check recovery memory for an image. */
	ROM_LOGGING_TRACE_VERIFY_MANIFEST = 0x8000000a,					/**< Authenticate the manifests on the image. */
	ROM_LOGGING_TRACE_REVOCATION_CHECK = 0x8000000b,				/**< Check for manifest revocation. */
	ROM_LOGGING_TRACE_VERIFY_1SP_HEADER = 0x8000000c,				/**< Authenticate the 1SP image header. */
	ROM_LOGGING_TRACE_READ_1SP = 0x8000000d,						/**< Copy the 1SP image to memory and validate. */
	ROM_LOGGING_TRACE_NO_OWNER_KEY = 0x8000000e,					/**< The image is not using any owner key for validation.
																		If available, a manufacturing key is used. */
	ROM_LOGGING_TRACE_OWNER_KEY_0 = 0x8000000f,						/**< The image is using owner key 0 for validation. */
	ROM_LOGGING_TRACE_OWNER_KEY_1 = 0x80000010,						/**< The image is using owner key 1 for validation. */
	ROM_LOGGING_TRACE_OWNER_KEY_2 = 0x80000011,						/**< The image is using owner key 2 for validation. */
	ROM_LOGGING_TRACE_OWNER_KEY_3 = 0x80000012,						/**< The image is using owner key 3 for validation. */
	ROM_LOGGING_TRACE_OWNER_KEY_4 = 0x80000013,						/**< The image is using owner key 4 for validation. */
	ROM_LOGGING_TRACE_OWNER_KEY_5 = 0x80000014,						/**< The image is using owner key 5 for validation. */
	ROM_LOGGING_TRACE_OWNER_KEY_6 = 0x80000015,						/**< The image is using owner key 6 for validation. */
	ROM_LOGGING_TRACE_TENANCY_GRANT = 0x80000016,					/**< The image requires a tenancy grant check. */
	ROM_LOGGING_TRACE_UPDATE_REVOCATION = 0x80000017,				/**< The revocation state is being updated. */
	ROM_LOGGING_TRACE_OWNERSHIP_TRANSFER = 0x80000018,				/**< A new owner key is being programmed. */
	ROM_LOGGING_TRACE_TENANCY_TRANSFER = 0x80000019,				/**< A tenancy transfer is being executed. */
	ROM_LOGGING_TRACE_DICE = 0x8000001a,							/**< Derive and endorse the DICE identity key. */
	ROM_LOGGING_TRACE_JUMP_TO_1SP = 0x8000001b,						/**< Execute the loaded 1SP image. */
	ROM_LOGGING_TRACE_HALT = 0x8000001c,							/**< ROM execution has halted, awaiting reset. */
	ROM_LOGGING_TRACE_ENTER_RECOVERY_STATUS = 0x8000001d,			/**< Enter status-only recovery mode due to init failures. */
	ROM_LOGGING_TRACE_FLASH_IMAGE_BAD = 0x8000001e,					/**< An image on flash did not pass verification. */
	ROM_LOGGING_TRACE_RECOVERY_IMAGE_BAD = 0x8000001f,				/**< A recovery image did not pass verification. */
	ROM_LOGGING_TRACE_RNG_TIMEOUT = 0x80000020,						/**< Timeout waiting for RNG initialization. */
	ROM_LOGGING_TRACE_DERIVE_FW_KEYS = 0x80000021,					/**< Derive firmware keys dependent on firmware state. */
	ROM_LOGGING_TRACE_CLEAR_FW_KEYS = 0x80000022,					/**< Clear firmware keys. */
	ROM_LOGGING_TRACE_SECONDARY_DIE_WAIT_FOR_MBX7_MSG = 0x80000023,	/**< Wait for mbox7 message on secondary die. */
	ROM_LOGGING_TRACE_SECONDARY_DIE_MBX7_MSG_RCVD = 0x80000024,		/**< Wait for mbox7 message on secondary die. */
	ROM_LOGGING_TRACE_FUSE_SYNC_DONE_PRIMARY = 0x80000025,			/**< Completed fuse sync prepare on primary die. */
	ROM_LOGGING_TRACE_FUSE_SYNC_DONE_SECONDARY = 0x80000026,		/**< Completed fuse sync update on secondary die. */
};


void rom_logging_error (uint8_t fail_id, uint32_t error_code);
void rom_logging_print_error (uint8_t fail_id, uint32_t error_code);
void rom_logging_set_last_error (uint8_t fail_id, uint32_t error_code);
void rom_logging_get_last_error (struct ocp_recovery_device_status_vendor *last);


#endif	/* ROM_LOGGING_H_ */
