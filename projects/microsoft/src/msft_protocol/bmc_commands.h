// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef BMC_COMMANDS_H_
#define BMC_COMMANDS_H_

#include "msft_mctp_protocol.h"
#include "cmd_interface/cmd_interface.h"
#include "crypto/hash.h"


/**
 * BMC command set commands
 */
enum {
	BMC_CMD_CHASSIS_INTRUSION_STORE_DATA = 0x11,		/**< Store battery backed data as part of chassis intrusion flow */
	BMC_CMD_CHASSIS_INTRUSION_CHALLENGE_DATA = 0x12,	/**< Challenge battery backed data as part of chassis intrusion flow */
	BMC_CMD_GET_SYSTEM_DEVICES = 0x13,					/**< Retrieve system devices for device disocovery flow */
	BMC_CMD_GET_DEVICE_STRING_IDENTIFIER = 0x14,		/**< Retrieve device string for device discovery flow */
	BMC_CMD_GET_DEVICE_EID = 0x15,						/**< Retrieve device eid for device discovery flow */
};

/**
 * The types of strings that can be requested from a device.
 */
enum bmc_device_string_type {
	BMC_DEVICE_STRING_TYPE_DEVICE_INSTANCE = 0x00,	/**< Device instance */
	BMC_DEVICE_STRING_TYPE_DEVICE_TYPE = 0x01,		/**< Device type */
};

/*
* The nonce length using during chassis intrusion operations
*/
#define BMC_CHASSIS_INTRUSION_NONCE_LEN					32

/**
 * Additional properties of the device in this system.
 */
#define BMC_DEVICE_PROPERTIES_NOT_SET						0x0		/**< Device properties are not set */
#define BMC_DEVICE_PROPERTIES_PA_ROT						0x1		/**< Bit 0, Platform Attestation Root of Trust */
#define BMC_DEVICE_PROPERTIES_ATTESTATION_SUPPORTED			0x2		/**< Bit 1, Attestation supported */
#define BMC_DEVICE_PROPERTIES_REMOTE_DEVICE					0x4		/**< Bit 2, The device is a remote device */
#define BMC_DEVICE_PROPERTIES_BUS_OWNER						0x8		/**< Bit 3, Top Most Bus Owner */
#define BMC_DEVICE_PROPERTIES_SUPPORT_MULTIPLE_EIDS			0x10	/**< Bit 4, Device supports multiple EIDs */

#pragma pack(push, 1)

/**
 * BMC command set chassis intrusion store data request format
 */
struct bmc_chassis_intrusion_store_data {
	struct msft_mctp_protocol_header header;	/**< Message header */
	uint8_t num_bytes;							/**< Number of bytes being stored */
};


/**
 * Get the total length for a BMC command set chassis intrusion store data request message
 *
 * @param len Length of the data to be stored
 */
#define	bmc_chassis_intrusion_store_data_length(len)    \
	(len + sizeof (struct bmc_chassis_intrusion_store_data))

/**
 * Get the buffer containing the data to be stored in the request message
 */
#define	bmc_chassis_intrusion_store_data_data(req)	(((uint8_t*) req) + sizeof (*req))

/**
 * BMC command set chassis intrusion store data response format
 */
struct bmc_chassis_intrusion_store_data_response {
	struct msft_mctp_protocol_response_header header;	/**< Message header */
};

/**
 * Hash algorithms used by chassis intrusion commands
 */
enum bmc_chassis_intrusion_hash_algos {
	BMC_CHASSIS_INTRUSION_HASH_SHA256 = 0x00,	/**< SHA256 */
	BMC_CHASSIS_INTRUSION_HASH_SHA384 = 0x01,	/**< SHA384 */
	BMC_CHASSIS_INTRUSION_HASH_SHA512 = 0x02,	/**< SHA512 */
};

/**
 * BMC command set chassis intrusion challenge data request format
 */
struct bmc_chassis_intrusion_challenge_data {
	struct msft_mctp_protocol_header header;		/**< Message header */
	uint8_t hash_algo;								/**< Hash algorithm to use in challenge response */
	uint8_t nonce[BMC_CHASSIS_INTRUSION_NONCE_LEN];	/**< Nonce */
};

/**
 * BMC command set chassis intrusion challenge data response format
 */
struct bmc_chassis_intrusion_challenge_data_response {
	struct msft_mctp_protocol_response_header header;	/**< Message header */
	uint8_t hash_len;									/**< Length of generated hash */
};


/**
 * Get the total length for a BMC command set chassis intrusion challenge data response message
 *
 * @param len Length of the generated hash included in the response
 */
#define	bmc_chassis_intrusion_challenge_data_response_length(len)   \
	(len + sizeof (struct bmc_chassis_intrusion_challenge_data_response))

/**
 * Get the buffer containing the generated hash in the response message
 */
#define	bmc_chassis_intrusion_challenge_data_hash(resp)	(((uint8_t*) resp) + sizeof (*resp))


/**
 * Version of the entry format.
 */
struct device_entry_version {
	uint8_t major_number:4;
	uint8_t minor_number:4;
};

/**
 * Device properties in the system
 */
struct bmc_system_device_properties {
	uint16_t is_pa_rot:1;				/**< Device is a PA-RoT */
	uint16_t attestation_support:1;		/**< Device supports attestation */
	uint16_t is_remote_device:1;		/**< Device is a remote device */
	uint16_t is_bus_owner:1;			/**< Device is the bus owner */
	uint16_t multiple_eid_support:1;	/**< Device supports multiple EIDs */
	uint16_t reverved:11;				/**< Reserved bits */
};

/**
 * Device entry format for a system
 */
struct bmc_system_device_entry {
	struct device_entry_version format_version;				/**< Version of entry format */
	uint16_t vendor_id;										/**< Vendor ID */
	uint16_t device_id;										/**< Device ID */
	uint16_t subsystem_vendor_id;							/**< Subsystem Vendor ID */
	uint16_t subsystem_id;									/**< Subsystem ID */
	uint8_t instance_id;									/**< Instance ID */
	struct bmc_system_device_properties device_properties;	/**< Device properties */
};


/**
 * Request payload to get system devices
 */
struct bmc_system_devices_get_data {
	struct msft_mctp_protocol_header header;	/**< Message header */
	uint16_t start_index;						/**< Start index of system device */
	uint16_t entry_count;						/**< Entry count of system device */
	uint16_t filter_device_properties;			/**< Device properties */
};

/**
 * Response payload to get system devices
 */
struct bmc_system_devices_get_data_response {
	struct msft_mctp_protocol_response_header header;	/**< Response message header */
	uint16_t start_index;								/**< Start index of system device */
	uint16_t entry_count;								/**< Entry count of system device */
	uint16_t remaining_entries;							/**< Remaining entries */
};


/* Get the buffer containing the system devices data in the response message
 *
 * @param rsp Pointer to the response message
 */
#define bmc_system_devices_get_data_response_data(rsp) \
	((uint8_t*) rsp + sizeof (struct bmc_system_devices_get_data_response))

/**
 * Get the total length for a BMC command set get system devices response message
 *
 * @param count Count of get system devices response entries
 */
#define bmc_system_devices_get_data_response_length(count) \
	((count * sizeof (struct bmc_system_device_entry)) + \
	sizeof (struct bmc_system_devices_get_data_response))

/**
 * Request payload to get device string
 */
struct bmc_system_device_string_identifier_get_data {
	struct msft_mctp_protocol_header header;	/**< Message header */
	uint16_t vendor_id;							/**< Vendor ID */
	uint16_t device_id;							/**< Device ID */
	uint16_t subsystem_vendor_id;				/**< Subsystem Vendor ID */
	uint16_t subsystem_id;						/**< Subsystem ID */
	uint8_t instance_id;						/**< Instance ID */
	uint8_t string_type;						/**< Device string type */
};

/**
 * Response payload to get device string
 */
struct bmc_system_device_string_identifier_get_data_response {
	struct msft_mctp_protocol_response_header header;	/**< Response message header */
	uint16_t string_length;								/**< Device string length */
};


/**
 * Get the buffer containing get device string response message
 *
 * @param rsp Pointer to the response message
 */
#define bmc_system_device_string_identifier_get_data_response_data(rsp) \
	((uint8_t*) rsp + sizeof (struct bmc_system_device_string_identifier_get_data_response))

/**
 * Get the total length for a BMC command set get device string response message
 *
 * @param len Length of the device string included in the response
 */
#define bmc_system_device_string_identifier_get_data_response_length(len) \
	(len + sizeof (struct bmc_system_device_string_identifier_get_data_response))

/**
 * EID entry for a device
 */
struct bmc_device_eid_entry {
	uint8_t eid;				/**< EID assigned to the device */
	uint8_t binding_identifier;	/**< Physical transport Binding Identifier */
};

/**
 * Request payload to get device eid
 */
struct bmc_system_device_eid_get_data {
	struct msft_mctp_protocol_header header;	/**< Message header */
	uint16_t vendor_id;							/**< Vendor ID */
	uint16_t device_id;							/**< Device ID */
	uint16_t subsystem_vendor_id;				/**< Subsystem Vendor ID */
	uint16_t subsystem_id;						/**< Subsystem ID */
	uint8_t instance_id;						/**< Instance ID */
};

/**
 * Response payload to get device eid
 */
struct bmc_system_device_eid_get_data_response {
	struct msft_mctp_protocol_response_header header;	/**< Response message header */
	uint8_t eid_count;									/**< EID count */
};


/* Get the buffer containing the device eid data in the response message
 *
 * @param rsp Pointer to the response message
 */
#define  bmc_system_device_eid_get_data_response_data(rsp) \
	((uint8_t*) rsp + sizeof (struct bmc_system_device_eid_get_data_response))

/**
 * Get the buffer containing the device eid data in the response message
 *
 * @param rsp Pointer to the response message
 */
#define  bmc_system_device_eid_get_data_response_length(count) \
	((count * sizeof (uint16_t)) + sizeof (struct bmc_system_device_eid_get_data_response))

#pragma pack(pop)


int bmc_chassis_intrusion_generate_store_data_request (const uint8_t *data, uint8_t num_bytes,
	uint8_t *buf, size_t buf_len);
int bmc_chassis_intrusion_process_store_data_response (struct cmd_interface_msg *response);

int bmc_chassis_intrusion_generate_challenge_data_request (enum hash_type hash_algo, uint8_t *nonce,
	uint8_t *buf, size_t buf_len);
int bmc_chassis_intrusion_process_challenge_data_response (struct cmd_interface_msg *response);

int bmc_system_devices_generate_get_data_request (uint16_t start_index, uint16_t entry_count,
	uint16_t filter_properties, uint8_t *buf, size_t buf_len);
int bmc_system_devices_process_get_data_response (struct cmd_interface_msg *response);

int bmc_system_device_generate_string_identifier_get_data_request (uint16_t vendor_id,
	uint16_t device_id,	uint16_t subsystem_vendor_id, uint16_t subsystem_id, uint8_t instance_id,
	uint8_t string_type, uint8_t *buf, size_t buf_len);
int bmc_system_device_process_string_identifier_get_data_response (
	struct cmd_interface_msg *response);

int bmc_system_device_generate_eid_get_data_request (uint16_t vendor_id, uint16_t device_id,
	uint16_t subsystem_vendor_id, uint16_t subsystem_id, uint8_t instance_id, uint8_t *buf,
	size_t buf_len);
int bmc_system_device_process_eid_get_data_response (struct cmd_interface_msg *response);


#endif	/* BMC_COMMANDS_H_ */
