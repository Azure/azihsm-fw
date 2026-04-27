// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_PROTOCOL_COMMANDS_COMMON_H_
#define OVERLAKE_PROTOCOL_COMMANDS_COMMON_H_

#include <stdint.h>
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/overlake_protocol.h"


/* TODO: These should be removed when command structures are defined. */
#define CERBERUS_PROTOCOL_CMD(name, type,\
		req)			  type name = (type) &(req)->data[CERBERUS_PROTOCOL_MIN_MSG_LEN]
#define CERBERUS_PROTOCOL_CMD_LEN(\
	type)					  (CERBERUS_PROTOCOL_MIN_MSG_LEN + sizeof (type))


/**
 * Algorithms supported for decryption.
 */
enum {
	OVERLAKE_DECRYPTION_ALGORITHM_RSA_OAEP_SHA1 = 0,	/**< RSA-OAEP using SHA1 padding. */
	OVERLAKE_DECRYPTION_ALGORITHM_RSA_OAEP_SHA256 = 1,	/**< RSA-OAEP using SHA256 padding. */
	OVERLAKE_DECRYPTION_ALGORITHM_ECDH = 2,				/**< ECDH with provided public key. */
	OVERLAKE_DECRYPTION_ALGORITHM_ECDH_SHA256 = 3,		/**< ECDH with SHA256 hash of result. */
};

#pragma pack(push, 1)
/**
 * Overlake protocol get debug log request format
 */
struct overlake_protocol_get_debug_log {
	struct cerberus_protocol_header header;	/**< Message header */
	uint32_t offset;						/**< Offset in the log to start reading */
	uint32_t length;						/**< Maximum length of data to read */
};

/**
 * Overlake protocol get debug log response format
 */
struct overlake_protocol_get_debug_log_response {
	struct cerberus_protocol_header header;	/**< Message header */
};

/**
 * Get the buffer containing the log data
 */
#define	overlake_protocol_debug_log(resp)	(((uint8_t*) resp) + sizeof (*resp))

/**
 * Get the total message length for a debug log response message.
 *
 * @param len Length of the log data.
 */
#define	overlake_protocol_debug_log_length(len) \
	(len + sizeof (struct overlake_protocol_get_debug_log_response))

/**
 * Maximum amount of log data that can be returned in the request
 *
 * @param req The command request structure containing the message.
 */
#define	OVERLAKE_PROTOCOL_MAX_DEBUG_LOG_DATA(req)   \
	(req->max_response - sizeof (struct overlake_protocol_get_debug_log_response))

/**
 * Overlake protocol prepare firmware update request format
 */
struct overlake_protocol_prepare_fw_update_request {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t port_id;						/**< Port ID */
	uint32_t size;							/**< Update size */
	uint8_t ctrl_flag;						/**< Implementation specific flag that can be used to trigger special workflows during update preparation. */
};

/**
 * Overlake protocol get SoC firmware update request format
 */
struct overlake_protocol_fw_update_request {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t port_id;						/**< Port ID */
	uint8_t payload;						/**< First byte of the variable data */
};


/**
 * Get the amount of payload data in a SoC fw update message.
 *
 * @param req The command request structure containing the message.
 */
#define	OVERLAKE_PROTOCOL_SOC_FW_UPDATE_LENGTH(req) \
	((req->length - sizeof (struct overlake_protocol_fw_update_request)) + sizeof (uint8_t))

/**
 * Overlake protocol get SoC firmware update status request format
 */
struct overlake_protocol_get_fw_update_status {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t port_id;						/**< Port ID */
};

/**
 * Overlake protocol get SoC firmware update status response format
 */
struct overlake_protocol_get_fw_update_status_response {
	struct cerberus_protocol_header header;	/**< Message header */
	uint32_t update_status;					/**< FW update status */
	uint32_t remaining_len;					/**< Number of bytes expected to still be sent */
};

/**
 * Overlake protocol decrypt payload request format
 */
struct overlake_protocol_decrypt_payload {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t algorithm;						/**< Which algorithm to use for decryption */
	uint16_t label_len;						/**< Length of the optional label for decryption */
	uint16_t decrypt_len;					/**< Length of the encrypted payload */
};

/**
 * Overlake protocol decrypt payload response format
 */
struct overlake_protocol_decrypt_payload_response {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t data;							/**< First byte of variable length decrypted data. */
};

/**
 * Get the buffer containing the label to use during decryption
 */
#define	overlake_protocol_decrypt_label(req)	(((uint8_t*) req) + sizeof (*req))

/**
 * Get the buffer containing the payload to decrypt
 */
#define	overlake_protocol_decrypt_data(req) \
	(overlake_protocol_decrypt_label (req) + req->label_len)

/**
 * Get the total length of the decrypt payload request
 */
#define	overlake_protocol_decrypt_payload_total_length(req) \
	(sizeof (struct overlake_protocol_decrypt_payload) + req->label_len + req->decrypt_len)

/**
 * Get the total message length for a debug log response message.
 *
 * @param len Length of the decrypted data
 */
#define	overlake_protocol_decrypted_response_length(len)    \
	((len + sizeof (struct overlake_protocol_decrypt_payload_response)) - sizeof (uint8_t))

/**
 * Maximum amount of log data that can be returned in the request
 *
 * @param req The command request structure containing the message.
 */
#define	OVERLAKE_PROTOCOL_MAX_DECRYPTED_DATA(req)   \
	((req->max_response - sizeof (struct overlake_protocol_decrypt_payload_response)) + sizeof (uint8_t))

/**
 * Overlake protocol sign data request format
 */
struct overlake_protocol_sign_data {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t payload;						/**< First byte of the variable data */
};

/**
 * Overlake protocol sign data response format
 */
struct overlake_protocol_sign_data_response {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t signature;						/**< First byte of the variable data */
};


/**
 * Get the amount of payload data for a sign data request.
 *
 * @param req The command request structure containing the message.
 */
#define overlake_protocol_sign_data_length(req) \
	((req->length - sizeof (struct overlake_protocol_sign_data)) + sizeof (uint8_t))

/**
 * Get the total message length for a sign data request.
 *
 * @param len The length of the signature.
 */
#define overlake_protocol_sign_data_response_length(len)    \
	(len + sizeof (struct overlake_protocol_sign_data_response) - sizeof (uint8_t))

/**
 * Maximum amount of signature data that can be returned in the request.
 *
 * @param req The command request structure containing the message.
 */
#define	OVERLAKE_PROTOCOL_MAX_SIGNATURE_DATA(req)   \
	((req->max_response - sizeof (struct overlake_protocol_sign_data_response)) + sizeof (uint8_t))

/**
 * Overlake protocol store data request format
 */
struct overlake_protocol_store_data {
	struct cerberus_protocol_header header;	/**< Message header */
	uint16_t id;							/**< Block ID */
	uint8_t payload;						/**< First byte of the variable data */
};


/**
 * Get the amount of payload data for a store data request.
 *
 * @param req The command request structure containing the message.
 */
#define overlake_protocol_store_data_length(req)    \
	((req->length - sizeof (struct overlake_protocol_store_data)) + sizeof (uint8_t))

/**
 * Overlake protocol clear data request format
 */
struct overlake_protocol_clear_data {
	struct cerberus_protocol_header header;	/**< Message header */
	uint16_t id;							/**< Block ID */
};

/**
 * Overlake protocol read data request format
 */
struct overlake_protocol_read_data {
	struct cerberus_protocol_header header;	/**< Message header */
	uint16_t id;							/**< Block ID */
};

/**
 * Overlake protocol read data response format
 */
struct overlake_protocol_read_data_response {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t payload;						/**< First byte of the variable data */
};


/**
 * Get the total message length for a read data request.
 *
 * @param len The length of the data read from storage.
 */
#define overlake_protocol_read_data_response_length(len)    \
	(len + sizeof (struct overlake_protocol_read_data_response) - sizeof (uint8_t))

/**
 * Maximum amount of stored data that can be returned in the request.
 *
 * @param req The command request structure containing the message.
 */
#define	OVERLAKE_PROTOCOL_MAX_READ_DATA(req)    \
	((req->max_response - sizeof (struct overlake_protocol_read_data_response)) + sizeof (uint8_t))

/**
 * Overlake protocol SoC/FPGA reset request format
 */
struct overlake_protocol_soc_reset {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t id;								/**< SoC/FPGA id */
};

/**
 * Overlake set SoC or FPGA boot mode request format
 */
struct overlake_protocol_boot_mode_payload {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t mode;							/**< Boot mode */
};

/**
 * Overlake protocol SoC FW version request format
 */
struct overlake_protocol_soc_fw_version {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t port_id;						/**< Port ID */
};

/**
 * Overlake protocol SoC FW version response format
 */
struct overlake_protocol_soc_fw_version_response {
	struct cerberus_protocol_header header;	/**< Message header */
	uint8_t version;						/**< First byte of the ASCII version string */
};


/**
 * Get the total message length for SoC FW version request.
 *
 * @param version_len The length of the version string read from storage.
 */
#define overlake_protocol_soc_fw_version_response_length(version_len)   \
	(version_len + sizeof (struct overlake_protocol_soc_fw_version_response) - sizeof (uint8_t))

/**
 * Maximum length of SoC FW version string that can be returned in the request.
 *
 * @param req The command request structure containing the message.
 */
#define	OVERLAKE_PROTOCOL_MAX_SOC_FW_VERSION_LENGTH(req)    \
	((req->max_response - sizeof (struct overlake_protocol_soc_fw_version_response)) + sizeof (uint8_t))

/**
 * Overlake protocol SoC debug level request/response format, used for socsetdebuglevel as request
 * and for socgetdebuglevel as response
 */
struct overlake_protocol_soc_debug_level {
	struct cerberus_protocol_header header;	/**< Message header */
	uint32_t debug_level;					/**< SoC debug level */
};

/**
 * Overlake protocol SOC request structure for "socgetdebuglevel" command
 */
struct overlake_protocol_soc_get_debug_level_request {
	struct cerberus_protocol_header header;	/**< Message header */
};

/* TODO: Define command formats for all Overlake commands. Move definitions from
 * overlake_protocol.h and redefined to mirror organization of base commands. */
#pragma pack(pop)


#endif	/* OVERLAKE_PROTOCOL_COMMANDS_COMMON_H_ */
