// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MSFT_BASE_COMMANDS_H_
#define MSFT_BASE_COMMANDS_H_

#include <stddef.h>
#include <stdint.h>
#include "msft_mctp_protocol.h"
#include "temperature_sensor_cluster.h"
#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/device_manager.h"
#include "mctp/mctp_notifier_interface.h"
#include "msft_protocol/cmd_interface_msft.h"


/**
 * Version of the base protocol provided by this implementation.
 */
#define	MSFT_BASE_PROTOCOL_VERSION			0


/**
 * Common completion codes that are valid across all command sets.
 */
enum {
	MSFT_BASE_CC_SUCCESS = 0x00,				/**< No error occurred during processing. */
	MSFT_BASE_CC_FAILURE = 0xff,				/**< Failure during command processing. */
	MSFT_BASE_CC_INVALID_CMD = 0xfe,			/**< The received command is unknown. */
	MSFT_BASE_CC_UNSUPPORTED_CMD = 0xfd,		/**< The command is known but not supported. */
	MSFT_BASE_CC_MALFORMED_CMD = 0xfc,			/**< The command is not structured correctly. */
	MSFT_BASE_CC_UNSUPPORTED_PARAM = 0xfb,		/**< An argument provided is not valid. */
	MSFT_BASE_CC_RESOURCE_UNAVAILABLE = 0xfa,	/**< A requested resource is currently not available. */
	MSFT_BASE_CC_UNSUPPORTED_VERSION = 0xf9,	/**< The message protocol version is not supported. */
};

/**
 * Command codes for the base MSFT command set.
 */
enum {
	MSFT_BASE_CMD_STATUS = 0x00,					/**< Generic status response message. */
	MSFT_BASE_CMD_CMD_SET_SUPPORT = 0x01,			/**< Get the command sets supported by the device. */
	MSFT_BASE_CMD_CAPABILITIES_NEGOTIATION = 0x02,	/**< Negotiate device capabilites for future requests. */
	MSFT_BASE_CMD_GET_TEMPERATURE = 0x03,			/**< Read the value of temperature sensors. */
	MSFT_BASE_CMD_HEARTBEAT_CTRL = 0x04,			/**< Start/stop Heartbeat messages from the device. */
	MSFT_BASE_CMD_HEARTBEAT = 0x05,					/**< Report current device health. */
};

/**
 * The feature bits in the capabilities negotiation response.
 */
enum msft_base_features {
	/* Byte 0 */
	MSFT_BASE_FEATURES_0_TEMP_SENSOR = (1U << 0),	/**< Platform has temperature sensors available. */
	MSFT_BASE_FEATURES_0_HEARTBEAT = (1U << 1),		/**< Platform supports sending heartbeat messages. */
};


#pragma pack(push, 1)

/**
 * Format for a generic status response.  This is common across all command sets.
 */
struct msft_base_status_response {
	struct msft_mctp_protocol_response_header header;	/**< Response header on the status information. */
	uint32_t error_code;								/**< Detailed error code for a failed request. */
	uint16_t data_length;								/**< Length of any additional error details in the response. */
};

/**
 * Get the extra error data present in the status response.
 *
 * @param resp The response message containing the error data.
 *
 * @return A pointer to the extra error data.
 */
#define	msft_base_status_response_error_data(resp)		(((uint8_t*) resp) + sizeof (*resp))

/**
 * Get the total length for a status response including additional error data.
 *
 * @param len Length of the additional error data added to the response.
 *
 * @return Total length of the response.
 */
#define	msft_base_status_response_length(len)	(sizeof (struct msft_base_status_response) + len)

/**
 * A request to obtain the supported MSFT command sets on the platform.
 */
struct msft_base_cmd_set_support_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint8_t list_entry_start;					/**< The index from within the command set list to begin receiving. */
};

/**
 * A single supported command set reported by the platform.
 */
struct msft_base_cmd_set_support_entry {
	uint8_t command_set;	/**< Identifier for the supported command set. */
	uint8_t version_count;	/**< Number of protocol versions supported in the command set. */
	uint16_t version;		/**< List of supported protocol versions. */
};

/**
 * The response containing the supported MSFT command sets on the platform.
 */
struct msft_base_cmd_set_support_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint8_t next_list_entry;							/**< If a message cannot contain all command
															sets, this will be the index needed for
															another request. 0xFF means there are no
															more command sets. */
	uint8_t entry_count;								/**< The number of command set entries in
															the response. */
	struct msft_base_cmd_set_support_entry entry;		/**< List of supported command sets. */
};

/**
 * List entry identifier when all supported commands have been provided.
 */
#define	MSFT_BASE_CMD_SET_SUPPORT_NO_MORE_ENTRIES		0xff

/**
 * Get the length of a single supported command set entry.
 *
 * @param count The number of supported protocol versions for the command set.
 *
 * @return The total length of the entry.
 */
#define	msft_base_cmd_set_support_get_entry_length(count)   \
	(sizeof (struct msft_base_cmd_set_support_entry) + (((count) - 1) * sizeof (uint16_t)))

/**
 * Get the next support command set entry in the list of supported command sets.
 *
 * There is no checking to ensure that there is another valid entry in the list.
 *
 * @param entry A pointer to the current command set entry.
 *
 * @return A pointer to the next command set entry in the list.
 */
#define	msft_base_cmd_set_support_get_next_entry(entry)         \
	(struct msft_base_cmd_set_support_entry*) (((uint8_t*) (entry)) + \
		msft_base_cmd_set_support_get_entry_length ((entry)->version_count))

/**
 * Get the total length for a command set support response.
 *
 * @param len Length of the command set entries list in the response.
 *
 * @return Total length of the response.
 */
#define	msft_base_cmd_set_support_response_length(len)          \
	((sizeof (struct msft_base_cmd_set_support_response) - \
		sizeof (struct msft_base_cmd_set_support_entry)) + len)

/**
 * A request to negotiate communication parameters between the requester and the responder.
 */
struct msft_base_caps_negotiation_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint16_t max_msg_size;						/**< The max supported message size by the requester. */
	uint16_t max_pkt_size;						/**< The max supported packet size by the requester. */
};

/**
 * The negotiation response for agreeing on communication parameters.
 */
struct msft_base_caps_negotiation_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint16_t max_msg_size;								/**< The max supported message size by the responder. */
	uint16_t max_pkt_size;								/**< The max supported packet size by the responder. */
	uint16_t msg_timeout;								/**< The communication timeout expectation,
															for the responder in 10 ms intervals. */
	uint8_t feature_flags;								/**< Bit mask of supported features. */
};

/**
 * A request to read temperature sensors.
 */
struct msft_base_get_temperature_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint8_t sensor_id;							/**< A platform defined ID of a sensor to begin reading from. */
	uint8_t read_single;						/**< A flag that specifies if only a single sensor should be read. */
};

/**
 * A single temperature reading from a temperature sensor.
 */
struct msft_base_temperature_reading {
	uint8_t sensor_id;		/**< The sensor ID for the temperature reading. */
	int16_t temperature;	/**< The temperature value of the sensor in 1/100th degrees C. */
};

/**
 * The response to return a list of temperature sensor readings.
 */
struct msft_base_get_temperature_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint8_t total_sensors;								/**< The total number of temperature sensors on the platform. */
	uint8_t sensors_reporting;							/**< The number of temperature readings. */
	struct msft_base_temperature_reading sensor;		/**< List of temperature readings. */
};


/**
 * Get the total length for a get temperature response.
 *
 * @param resp A pointer to the temperature response.
 *
 * @return Total length of the response.
 */
#define	msft_base_get_temperature_response_length(resp)         \
	(sizeof (struct msft_base_get_temperature_response) + \
		(sizeof (struct msft_base_temperature_reading) * ((resp)->sensors_reporting - 1)))

/**
 * A request to start or stop heartbeat message generation for all devices on the platform.
 */
struct msft_base_heartbeat_ctrl_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint8_t enable:1;							/**< Enables (1) or disables (0) heartbeat generation. */
	uint8_t force:1;							/**< Flag to force heartbeat registration. */
	uint8_t rsvd:6;								/**< Reserved. */
};

/**
 * The result of the heartbeat device control command.
 */
struct msft_base_heartbeat_ctrl_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
};

/**
 * The health status for a single CPU core in the device.
 */
struct msft_base_heartbeat_cpu {
	uint16_t core_id;		/**< A platform defined ID for this entry. */
	uint16_t health_status;	/**< A platform defined health status value. */
};

/**
 * A notification request to update the heartbeat state.  The source of the heartbeat is determined
 * by the message source EID.
 */
struct msft_base_heartbeat_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint16_t timeout_secs;						/**< The timeout in seconds expected for the next heartbeat update. */
	uint8_t cpu_count;							/**< The number of additional CPUs being reported. */
	struct msft_base_heartbeat_cpu cpu;			/**< List of CPUs reporting health status. */
};

/**
 * The response to acknowledge the heartbeat.
 */
struct msft_base_heartbeat_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
};


/**
 * Get the total length for a heartbeat request.
 *
 * @param req A pointer to the heartbeat request.
 *
 * @return Total length of the request.
 */
#define	msft_base_heartbeat_request_length(req)		(sizeof (struct msft_base_heartbeat_request) + \
	(sizeof (struct msft_base_heartbeat_cpu) * (req)->cpu_count))

#pragma pack(pop)

/**
 * Descriptor for a single command set supported by the device.
 */
struct msft_base_supported_command_set {
	uint8_t set_id;				/**< The command set identifier. */
	const uint16_t *versions;	/**< List of supported protocol versions for the command set. */
	size_t version_count;		/**< Number of protocol versions in the list. */
};


void msft_base_build_error_response (struct cmd_interface_msg *message, uint8_t completion_code,
	uint32_t error_code);
void msft_base_build_error_response_with_data (struct cmd_interface_msg *message,
	uint8_t completion_code, uint32_t error_code, const uint8_t *error_data, size_t length);

int msft_base_supported_command_set_init (struct msft_base_supported_command_set *entry,
	uint8_t set_id, const uint16_t *protocol_versions, size_t version_count);
int msft_base_command_set_support (const struct msft_base_supported_command_set *cmd_sets,
	size_t count, struct cmd_interface_msg *request);

int msft_base_capabilities_negotiation (struct device_manager *device_mgr, uint8_t feature_flags,
	struct cmd_interface_msg *request);

int msft_base_get_temperature (const struct temperature_sensor_cluster *cluster,
	struct cmd_interface_msg *request);

int msft_base_heartbeat_control (const struct mctp_notifier_interface *notifier,
	struct cmd_interface_msg *request);

int msft_base_build_heartbeat_request (uint16_t timeout_secs, uint16_t core_id,
	uint16_t health_status, uint8_t *payload, size_t payload_len);


#endif	/* MSFT_BASE_COMMANDS_H_ */
