// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef ROT_COMMANDS_H_
#define ROT_COMMANDS_H_

#include "msft_mctp_protocol.h"
#include "rot_log_interface.h"
#include "cmd_interface/cmd_background.h"
#include "intrusion/intrusion_state.h"
#include "mctp/mctp_notifier_interface.h"
#include "system/real_time_clock.h"
#include "system/secure_device_unlock.h"


/**
 * Version of the RoT protocol provided by this implementation.
 */
#define	ROT_PROTOCOL_VERSION			0

/**
 * The size of the feature_flags array in the Get RoT Capabilities response.
 */
#define ROT_CAPABILITIES_SIZE		2


/**
 * Completion codes specific to the RoT command set
 */
enum {
	ROT_CC_INVALID_DEVICE_STATE = 0x01,	/**< The device state does not allow the requested command. */
};

/**
 * Command codes for the RoT MSFT command set.
 */
enum {
	ROT_CMD_GET_ROT_CAPABILITIES = 0x00,	/**< Determine the capabilities of a RoT implementation. */
	ROT_CMD_RESET_ROT = 0x01,				/**< Trigger a RoT-only reset. */
	ROT_CMD_GET_TENANCY_GRANT_TOKEN = 0x02,	/**< Get a tenancy grant token for a tenant FW signing key. */
	ROT_CMD_GET_UNLOCK_TOKEN = 0x03,		/**< Get a device authorization token to unlock features. */
	ROT_CMD_APPLY_UNLOCK_POLICY = 0x04,		/**< Provide an authorized unlock policy to the device. */
	ROT_CMD_CLEAR_UNLOCK_POLICY = 0x05,		/**< Erase any active or pending unlock policy in the device. */
	ROT_CMD_GET_CRASH_DUMP_COUNT = 0x06,	/**< Determine the number of available crash dumps for a core. */
	ROT_CMD_GET_CRASH_DUMP_INFO = 0x07,		/**< Retrieve information about the available crash dump. */
	ROT_CMD_READ_CRASH_DUMP = 0x08,			/**< Read the current crash dump information. */
	ROT_CMD_CLEAR_CRASH_DUMP = 0x09,		/**< Erase a crash dump. */
	ROT_CMD_GET_TIME = 0x0a,				/**< Get the current time from the RoT. */
	ROT_CMD_SET_TIME = 0x0b,				/**< Set the current time in the RoT. */
	ROT_CMD_GET_INTRUSION_DETECTION = 0x0c,	/**< Get the current state of the intrusion detection signal. */
	ROT_CMD_INTRUSION_EVENT = 0x0d,			/**< Report a run-time intrusion event. */
	ROT_CMD_GET_INTRUSION_COUNT = 0x0e,		/**< Get the number of detected intrusion events. */
	ROT_CMD_WARM_RESET_EVENT_CTRL = 0x0f,	/**< Start/stop warm reset event messages for internal cores. */
	ROT_CMD_PREPARE_WARM_RESET = 0x10,		/**< Notification that an internal core is about to be reset. */
	ROT_CMD_WARM_RESET_COMPLETE = 0x11,		/**< Notification that an internal core has completed warm reset. */
	ROT_CMD_SEND_LOG_COMMAND = 0x12,		/**< Trigger to send log data. */
	ROT_CMD_READ_LOG_COMMAND = 0x13,		/**< Read log data from RoT. */
};

/**
 * The RoT feature bits from a capabilities request that describe the supported commands.
 */
enum rot_feature {
	ROT_FEATURE_ROT_RESET = 0,				/**< RoT reset commands supported. */
	ROT_FEATURE_TENANCY_TRANSFER = 1,		/**< Tenancy transfer commands supported. */
	ROT_FEATURE_DEBUG_UNLOCK = 2,			/**< Debug unlock commands supported. */
	ROT_FEATURE_RUNTIME_UNLOCK = 3,			/**< Runtime unlock commands supported. */
	ROT_FEATURE_CRASH_DUMP = 4,				/**< Crash dump commands supported. */
	ROT_FEATURE_TIME = 5,					/**< Time commands supported. */
	ROT_FEATURE_INTRUSION_DETECTION = 6,	/**< Intrusion detection commands supported. */
	ROT_FEATURE_WARM_RESET_EVENTS = 7,		/**< Warm reset commands supported. */
	ROT_FEATURE_LOG = 8,					/**< Log commands supported */
	ROT_FEATURE_COUNT,						/**< The total count of supported feature bits. */
};


/**
 * Available options for when to change the device unlock policy.  The same options apply when
 * enabling or disabling an active unlock policy.
 */
enum {
	ROT_UNLOCK_POLICY_OPTION_NEXT_RESET = 0x00,		/**< Change the unlock policy on the next reset. */
	ROT_UNLOCK_POLICY_OPTION_FORCE_RESET = 0x01,	/**< Reset the RoT to change the unlock policy. */
	ROT_UNLOCK_POLICY_OPTION_IMMEDIATELY = 0x02,	/**< Immediately change the unlock policy. */
};

/**
 * The intrusion event codes.
 */
enum {
	ROT_NO_INTRUSION_DETECTED = 0x00,	/**< There is no active intrusion detected */
	ROT_INTRUSION_DETECTED = 0x01,		/**< There is an active intrusion detected */
	ROT_INTRUSION_STATE_RESET = 0x02,	/**< Secure intrusion state has been reset */
};

/**
 * The RoT log type codes.
 */
enum {
	ROT_LOG_TYPE_LOG_ENTRY_INFO = 0,	/**< Log Entry Info. */
};

/**
 * The RoT Send Log request flags
 */
enum {
	ROT_SEND_LOG_FLAG_READ_REQUEST = 1 << 0,	/**< Log Read Request. */
};

#pragma pack(push, 1)

/**
 * A request to get the RoT capabilities and features supported by the device.
 */
struct rot_get_rot_capabilities_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
};

/**
 * The response containing capabilities and features supported by the device.
 */
struct rot_get_rot_capabilities_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint8_t feature_flags[ROT_CAPABILITIES_SIZE];		/**< Bit mask of supported features. */
};

/**
 * A request to execute a warm reset of the RoT subsystem.
 */
struct rot_reset_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
};

/**
 * The RoT reset response.
 */
struct rot_reset_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
};

/**
 * A request to get an authorization token for unlocking protected features of a device.
 */
struct rot_get_unlock_token_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
};

/**
 * The unlock token response containing the authorization token to use with unlock handling.
 */
struct rot_get_unlock_token_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint8_t token;										/**< First byte of the authorization token. */
};


/**
 * Get the length of the unlock token contained in the response.
 *
 * @param len Total length of the unlock token response message.
 *
 * @return Length of the token data within the message.
 */
#define	rot_get_unlock_token_get_token_length(len)      \
	(len - (sizeof (struct rot_get_unlock_token_response) - sizeof (uint8_t)))

/**
 * Get the total length for an unlock token response.
 *
 * @param len Length of the authorization token in the response.
 *
 * @return Total length of the response.
 */
#define	rot_get_unlock_token_response_length(len)       \
	((sizeof (struct rot_get_unlock_token_response) - sizeof (uint8_t)) + len)

/**
 * Get the maximum length allowed for the unlock token data.
 *
 * @param max The maximum response length allowed by the message.
 *
 * @return Maximum token data length that will fit in the response.
 */
#define	rot_get_unlock_token_max_token_length(max)	rot_get_unlock_token_get_token_length (max)

/**
 * A request to apply an authorized unlock policy to the device.
 */
struct rot_apply_unlock_policy_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint8_t option;								/**< The option for when the policy will get applied. */
	uint8_t unlock_policy;						/**< First byte of the authorized policy data. */
};

/**
 * The apply unlock policy response message.
 */
struct rot_apply_unlock_policy_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
};


/**
 * Get the length of the authorized unlock policy contained in the request.
 *
 * @param len Total length of the apply unlock policy request message.
 *
 * @return Length of the unlock policy data within the message.
 */
#define	rot_apply_unlock_policy_get_policy_length(len)      \
	(len - (sizeof (struct rot_apply_unlock_policy_request) - sizeof (uint8_t)))

/**
 * A request to clear an active unlock policy from the device, disabling any unlocked features.
 */
struct rot_clear_unlock_policy_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint8_t option;								/**< The option for when the active policy will be disabled. */
};

/**
 * The clear unlock policy response message.
 */
struct rot_clear_unlock_policy_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
};

/**
 * A request to obtain the current value, in seconds, of the system clock.
 */
struct rot_get_time_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
};

/**
 * The get time response message.  It is up to the requestor to interpret the returned time.
 */
struct rot_get_time_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint32_t secs;										/**< The value, in seconds, of the system clock. */
};

/**
 * A request to set the system clock.
 */
struct rot_set_time_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint32_t secs;								/**< The value, in seconds, to assign to the system clock. */
};

/**
 * The set time response message.
 */
struct rot_set_time_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
};

/**
 * A request to obtain the active intrusion detection of the system.
 */
struct rot_get_intrusion_detection_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	union {
		uint8_t value_1byte;					/**< Value for 1 byte measured data type */
		struct {
			uint8_t event_register : 1;			/**< Set (1) to start/stop receiving Intrusion Events
													Clear (0), other control bits are ignored and there
													will be no change to Intrusion Event registration */
			uint8_t start_event_reporting : 1;	/**< Set (1) to start reporting Intrusion Events to requester
													Clear (0) to stop event reporting */
			uint8_t force_send_event : 1;		/**< Set (1) to force the device to send Intrusion Events to
													requester */
			uint8_t reserved : 5;				/**< Reserved config bits */
		} control;
	} intrusion_event_control;					/**< intrusion event control config */
};

/**
 * The get intrusion detection response message.
 */
struct rot_get_intrusion_detection_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint8_t intrusion_detection;						/**< Clear (0) : no active intrusion
																Set (1) : active intrusion detected */
};

/**
 * A request to obtain the intrusion count of the system.
 */
struct rot_get_intrusion_count_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
};

/**
 * The get intrusion count response message.
 */
struct rot_get_intrusion_count_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint32_t intrusion_count;							/**< Current value of intrusion count */
};

/**
 * A intrusion event request message.
 */
struct rot_intrusion_event_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol request header. */
	uint8_t intrusion_event;					/**< Intrusion event, 0 means no active intrusion detected; otherwise, active intrusion detected. */
};

/**
 * A intrusion event response message.
 */
struct rot_intrusion_event_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol response header. */
};

/**
 * A request to read log.
 */
struct rot_read_log_command_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint8_t type;								/**< Log type of data. */
	uint16_t log_id;							/**< Identifier for the log that’s being requested. */
	uint32_t offset;							/**< Offset within the log data to start at when reading data. */
};

/**
 * The read log command response.
 */
struct rot_read_log_command_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint8_t type;										/**< Log type of data. */
	uint16_t log_id;									/**< Identifier for the log that’s being requested. */
	uint32_t offset;									/**< Offset within the log data. */
	uint8_t hash_type;									/**< Hash type. */
	uint8_t sha256[SHA256_HASH_LENGTH];					/**< The SHA256 hash of the data. */
};

/**
 * A send log command request message.
 */
struct rot_send_log_command_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol request header. */
	uint8_t type;								/**< Log type of data. */
	uint8_t flag;								/**< Bit 0: Log Read Request.
														If set, the content data contains the log read information rather than actual log data. */
};

/**
 * A send log command response message.
 */
struct rot_send_log_command_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol response header. */
};


/**
 * Get the buffer containing the digest for the log entry info of a single Send Log Request message.
 */
#define	rot_log_entry_info_digest_data(req)  \
	(((uint8_t*) req) + sizeof (*req) + sizeof (struct rot_log_interface_entry_info))

/**
 * Get the buffer containing the log data for read log and send log request messages
 */
#define	rot_log_request_data(req) (((uint8_t*) req) + sizeof (*req))

/**
 * Get the buffer containing the log data for read log and send log response messages
 */
#define	rot_log_response_data(resp) (((uint8_t*) resp) + sizeof (*resp))

/**
 * Get the total message length for a send log request message.
 *
 * @param log_len Length of the log data.
 */
#define	rot_send_log_request_log_length(log_len)  \
	(log_len + sizeof (struct rot_send_log_command_request))

/**
 * Get the total message length for a read log response message.
 *
 * @param log_len Length of the log data.
 */
#define	rot_read_log_response_log_length(log_len)  \
	(log_len + sizeof (struct rot_read_log_command_response))

/**
 * Maximum amount of log data that can be returned in a single request
 *
 * @param req The command request structure containing the message.
 */
#define	ROT_READ_LOG_MAX_LOG_DATA(req)  \
	(cmd_interface_msg_get_max_response (req) - sizeof (struct rot_read_log_command_response))

#pragma pack(pop)


int rot_set_rot_capabilities_feature (uint8_t *feature_flags, size_t features_len,
	enum rot_feature feature);
int rot_get_rot_capabilities (const uint8_t *caps, size_t caps_len,
	struct cmd_interface_msg *request);

int rot_reset (const struct cmd_background *background, struct cmd_interface_msg *request);

int rot_get_unlock_token (const struct secure_device_unlock *unlock,
	struct cmd_interface_msg *request);
int rot_apply_unlock_policy (const struct secure_device_unlock *unlock,
	const struct cmd_background *background, struct cmd_interface_msg *request);
int rot_clear_unlock_policy (const struct secure_device_unlock *unlock,
	const struct cmd_background *background, struct cmd_interface_msg *request);

int rot_get_time (const struct real_time_clock *rtc, struct cmd_interface_msg *request);
int rot_set_time (const struct real_time_clock *rtc, struct cmd_interface_msg *request);

int rot_get_intrusion_detection (const struct intrusion_state *intrusion,
	const struct mctp_notifier_interface *notifier, struct cmd_interface_msg *request);
int rot_get_intrusion_count (const struct intrusion_state *intrusion,
	struct cmd_interface_msg *request);

int rot_build_intrusion_event_request (uint8_t event, uint8_t *payload, size_t payload_len);

int rot_read_log_command (const struct rot_log_interface *log, const struct hash_engine *hash,
	struct cmd_interface_msg *request);
int rot_build_send_log_command_request (const struct rot_log_interface *log, uint8_t log_type,
	uint8_t *payload, size_t payload_len);


#endif	/* ROT_COMMANDS_H_ */
