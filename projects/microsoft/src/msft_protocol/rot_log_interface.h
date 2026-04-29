// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef ROT_LOG_INTERFACE_H_
#define ROT_LOG_INTERFACE_H_

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

/**
 * The types of hashes supported by RoT log interface.
 */
enum {
	ROT_LOG_INTERFACE_HASH_TYPE_SHA256 = 0,	/**< SHA-256 hash */
	ROT_LOG_INTERFACE_HASH_TYPE_SHA384 = 1,	/**< SHA-384 hash */
	ROT_LOG_INTERFACE_HASH_TYPE_SHA512 = 2,	/**< SHA-512 hash */
};

/**
 * RoT log entry info for the Send Log command.
 */
struct rot_log_interface_entry_info {
	uint16_t log_id;		/**< Identifier for the log that’s being requested. */
	uint32_t total_length;	/**< The total length of the log. */
	uint8_t hash_type;		/**< Hash type. */
};

#pragma pack(pop)

/**
 * RoT Log interface used for the RoT MCTP command.
 */
struct rot_log_interface {
	/**
	 * Retrieve the log data and length based on the log type and log ID
	 *
	 * @param log Log interface.
	 * @param log_type Log type. Using log type 0 will get log info and other values get log entries.
	 * @param log_id Log id.
	 * @param offset Offset within the log to start reading data.
	 * @param data Output buffer for the log data.
	 * @param length Maximum number of bytes to read from the log.
	 * @param more_entry_data flag. This boolean indicates if there is more log data
	 * that was not returned due to buffer length constraints or if the entire entry is present.
	 *
	 * @return Length of the log data or an error code.  Use
	 * ROT_IS_ERROR to check the return value.
	 *
	 */
	int (*get_log) (const struct rot_log_interface *log, uint8_t log_type, uint16_t log_id,
		uint32_t offset, uint8_t *data,	size_t length, bool *more_entry_data);

	/**
	 * Retrieve the log entry info for a single log entry.
	 *
	 * @param log Log interface.
	 * @param log_type Log type. Get log entry info based on the log type.
	 * @param info Log entry info.
	 * @param digest Digest for the log data.
	 * @param digest_length The length of the digest buffer. The length of the digest
	 * will be determined by the implementation based on which hash type it uses.
	 *
	 * @return Length of the log entry info or an error code.  Use
	 * ROT_IS_ERROR to check the return value.
	 *
	 */
	int (*get_entry_read_info) (const struct rot_log_interface *log, uint8_t log_type,
		struct rot_log_interface_entry_info *info, uint8_t *digest, size_t digest_length);
};


#endif	/* ROT_LOG_INTERFACE_H_ */
