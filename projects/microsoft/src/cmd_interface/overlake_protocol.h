// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_PROTOCOL_H_
#define OVERLAKE_PROTOCOL_H_

#include "tpm/tpm.h"


/**
 * Overlake protocol commands
 */
enum {
	OVERLAKE_PROTOCOL_GET_STORAGE = 0xA1,			/**< Get blob storage */
	OVERLAKE_PROTOCOL_SET_STORAGE = 0xA2,			/**< Set blob storage */
	OVERLAKE_PROTOCOL_GET_COUNTER = 0xA3,			/**< Get tamper counter */
	OVERLAKE_PROTOCOL_INCREMENT_COUNTER = 0xA4,		/**< Increment tamper counter */
	OVERLAKE_PROTOCOL_READ_DATA = 0xA5,				/**< Read data from internal storage */
	OVERLAKE_PROTOCOL_CLEAR_DATA = 0xA6,			/**< Clear internal storage data */
	OVERLAKE_PROTOCOL_STORE_DATA = 0xA7,			/**< Store data to internal storage */
	OVERLAKE_PROTOCOL_SIGN_DATA = 0xA8,				/**< Sign object data */
	OVERLAKE_PROTOCOL_GET_FPGA_BOOT_MODE = 0xB0,	/**< Get cached FPGA boot mode */
	OVERLAKE_PROTOCOL_SET_FPGA_BOOT_MODE = 0xB1,	/**< Set FPGA boot mode in flash and update cache */
	OVERLAKE_PROTOCOL_GET_SOC_FW_HEADER = 0xB3,		/**< Get the extended header of the port. */
	OVERLAKE_PROTOCOL_SOC_RESET = 0xE0,				/**< Reset SoC */
	OVERLAKE_PROTOCOL_SOC_INIT_FW_UPDATE = 0xE1,	/**< Init SoC FW update process */
	OVERLAKE_PROTOCOL_SOC_UPDATE_FW = 0xE2,			/**< Send SoC FW update data */
	OVERLAKE_PROTOCOL_TRIGGER_NMI = 0xE3,			/**< Trigger SoC NMI */
	OVERLAKE_PROTOCOL_GET_BOOT_DEVICE = 0xE4,		/**< Get SoC boot device */
	OVERLAKE_PROTOCOL_SET_BOOT_DEVICE = 0xE5,		/**< Change SoC boot device */
	OVERLAKE_PROTOCOL_GET_MAC_ADDRESS = 0xE6,		/**< Get SoC MAC Address */
	OVERLAKE_PROTOCOL_GET_DEBUG_LOG_INFO = 0xE7,	/**< Get SoC firmware debug Log info */
	OVERLAKE_PROTOCOL_GET_DEBUG_LOG = 0xE8,			/**< Get SoC debug log data */
	OVERLAKE_PROTOCOL_GET_SOC_UPDATE_STATUS = 0xE9,	/**< Get SoC update status */
	OVERLAKE_PROTOCOL_TPM_CLEAR = 0xEA,				/**< TPM clear */
	OVERLAKE_PROTOCOL_GET_PUBLIC_KEY = 0xEB,		/**< Get public key */
	OVERLAKE_PROTOCOL_DECRYPT_PAYLOAD = 0xEC,		/**< Decrypt payload */
	OVERLAKE_PROTOCOL_SET_DEBUG_VERBOSITY = 0xED,	/**< Set SoC debug verbose level */
	OVERLAKE_PROTOCOL_GET_DEBUG_VERBOSITY = 0xEE,	/**< Get SoC debug verbose level */
	OVERLAKE_PROTOCOL_GET_SOC_FWVERSION = 0xEF,		/**< Get the SoC FW Version */
};


#pragma pack(push, 1)
/**
 * Overlake protocol get storage request packet format
 */
struct overlake_protocol_get_storage_request_packet {
	uint8_t index;	/**< Storage segment index */
};

/**
 * Overlake protocol get storage response packet format
 */
struct overlake_protocol_get_storage_response_packet {
	uint8_t index;			/**< Storage segment index */
	uint8_t segment_data;	/**< First byte of the variable data */
};

/**
 * Overlake protocol set storage request packet format
 */
struct overlake_protocol_set_storage_request_packet {
	uint8_t index;			/**< Storage segment index */
	uint8_t segment_data;	/**< First byte of the variable data */
};

/**
 * Overlake protocol get counter request packet format
 */
struct overlake_protocol_get_counter_request_packet {
	uint32_t reserved;	/**< Reserved */
};

/**
 * Overlake protocol get counter response packet format
 */
struct overlake_protocol_get_counter_response_packet {
	uint64_t counter;	/**< Counter value */
};

/**
 * Overlake protocol increment counter request packet format
 */
struct overlake_protocol_increment_counter_request_packet {
	uint32_t reserved;	/**< Reserved */
};

/**
 * Overlake protocol clear key slot request packet format
 */
struct overlake_protocol_clear_key_slot_request_packet {
	uint8_t slot_num;	/**< Key slot number */
};

/**
 * Overlake protocol get entropy request packet format
 */
struct overlake_protocol_get_entropy_request_packet {
	uint16_t entropy_len;	/**< Entropy length */
};

/**
 * Overlake protocol get public key request packet format
 */
struct overlake_protocol_get_public_key_request_packet {
	uint8_t slot_num;	/**< Slot number of target chain */
	uint8_t cert_num;	/**< Certificate number in chain */
	uint16_t offset;	/**< Offset in bytes from start of public key */
	uint16_t length;	/**< Number of bytes to read back, 0 for max payload length */
};

/**
 * Overlake protocol get public key response header format
 */
struct overlake_protocol_get_public_key_response_header {
	uint8_t slot_num;	/**< Slot number of target chain */
	uint8_t cert_num;	/**< Certificate number in chain */
};

/**
 * Overlake protocol TPM clear request packet format
 */
struct overlake_protocol_tpm_clear_request_packet {
	uint32_t reserved;	/**< Reserved */
};

#pragma pack(pop)


#endif	/* OVERLAKE_PROTOCOL_H_ */
