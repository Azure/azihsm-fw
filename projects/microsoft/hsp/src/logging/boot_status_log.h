// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef BOOT_STATUS_LOG_H_
#define BOOT_STATUS_LOG_H_

#include <stdint.h>
#include "logging/logging.h"


/**
 * Boot status logging fail_id for non failure paths.
 */
#define BOOT_STATUS_NO_FAILURE	0xFF

/**
 * Boot status unmapped post code - not logged.
 */
#define BOOT_STATUS_UNMAPPED_POSTCODE	0x0

/**
 * HSP boot status source.
 */
#define BOOT_STATUS_SRC_HSP		0x0

/**
 * Makes 8bit GPIO boot status.
 */
#define MAKE_GPIO_BOOT_STATUS(src, code) \
	((uint8_t)(((uint8_t)(src & 0x3) << 6) | ((uint32_t)(code & 0x3F))))

/**
 * Possible values for the boot status.
 */
enum boot_status_codes {
	BOOT_STATUS_HSP_ROM_START = 0x10,					/**< Boot status for rom start */
	BOOT_STATUS_HSP_ROM_JTAG_MSG = 0x11,				/**< Boot status for jtag message */
	BOOT_STATUS_HSP_ROM_VERIFY_ROOT_MANIFEST = 0x12,	/**< Boot status for verifying root manifest */
	BOOT_STATUS_HSP_ROM_FETCH_SP1_SLOTA	= 0x13,			/**< Boot status for fetching sp1 from slot a */
	BOOT_STATUS_HSP_ROM_FETCH_SP1_SLOTB	= 0x14,			/**< Boot status for fetching sp1 from slot b */
	BOOT_STATUS_HSP_ROM_VERIFY_SP1 = 0x15,				/**< Boot status for verifying sp1 */
	BOOT_STATUS_HSP_ROM_JUMP_TO_SP1 = 0x16,				/**< Boot status for jump to sp1 */
	BOOT_STATUS_HSP_ROM_SP1_INVALID_FIRMWARE_ID = 0x39,	/**< Boot status for invalid sp1 fw id */
	BOOT_STATUS_HSP_ROM_FETCH_SP1_FAILED = 0x3A,		/**< Boot status for fetching failure of sp1 */
	BOOT_STATUS_HSP_ROM_1SP_REVOCATION_FAILED = 0x3B,	/**< Boot status for revocation failure */
	BOOT_STATUS_HSP_ROM_RECOVERY = 0x3C,				/**< Boot status for rom recovery */
	BOOT_STATUS_HSP_ROM_HALT = 0x3D,					/**< Boot status for rom halt */
	BOOT_STATUS_HSP_ROM_UNKNOWN_ERROR = 0x3E,			/**< Boot status for unknown error */
	BOOT_STATUS_HSP_BOOT_COMPLETE = 0x3F,				/**< Boot status final code */
};

/**
 * Global singleton for the boot status log.
 */
#ifndef LOGGING_BOOT_STATUS_LOG_CONST_INSTANCE
extern const struct logging *boot_status_log;
extern const struct boot_status_gpio_interface *boot_status_gpio;
#else
extern const struct logging *const boot_status_log;
extern const struct boot_status_gpio_interface *const boot_status_gpio;
#endif

/**
 * Defines the API for accessing boot status gpio.
 */
struct boot_status_gpio_interface {
	/**
	 * Writes boot status gpio.
	 *
	 * @param boot_status_gpio Boot status gpio interface instance.
	 * @param boot_status Boot status code to be written on gpio.
	 *
	 * @return 0 if successful, else an error
	 */
	int (*write) (const struct boot_status_gpio_interface *boot_status_gpio, uint8_t boot_status);
};

#pragma pack(push, 1)

/**
 * Format for an entry in the boot status log.
 */
struct boot_status_log_entry_info {
	uint8_t boot_status;	/**< boot status Entry */
};

/**
 * Format of the boot status log entry as stored in the log.
 */
struct boot_status_log_entry {
	struct logging_entry_header header;			/**< Standard logging header. */
	struct boot_status_log_entry_info entry;	/**< Information for the log entry. */
};

#pragma pack(pop)


int boot_status_log_create_entry (uint8_t fail_id, uint32_t logging_code);
#ifndef LOGGING_DISABLE_FLUSH
int boot_status_log_flush (void);
#endif
int boot_status_log_clear (void);
int boot_status_log_get_size (void);
int boot_status_log_read_contents (uint32_t offset, uint8_t *contents, size_t length);


#endif	/* BOOT_STATUS_LOG_H_ */
