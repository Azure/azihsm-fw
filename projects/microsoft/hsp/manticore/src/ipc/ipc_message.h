// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef IPC_MESSAGE_H_
#define IPC_MESSAGE_H_

#include <stdint.h>
#include <string.h>
#include "cmd_interface/cmd_interface.h"


/**
 * Enum for IPC message opcodes that handled by HSP.
 */
enum ipc_message_opcode {
	IPC_MESSAGE_OPCODE_STATUS_CHANGE = 0x00,		/**< Opcode for Status Change request */
	IPC_MESSAGE_OPCODE_DOE = 0x40,					/**< Opcode of DOE Message request */
	IPC_MESSAGE_OPCODE_HEALTH_MONITOR = 0x41,		/**< Opcode of IPC health monitor request */
	IPC_MESSAGE_OPCODE_SHUTDOWN_REQUEST = 0x42,		/**< Opcode of IPC Shutdown request */
	IPC_MESSAGE_OPCODE_STOP_INTERFACE = 0x43,		/**< Opcode of IPC Stop Interface request */
	IPC_MESSAGE_OPCODE_RSA_KEY_GEN = 0x44,			/**< Opcode of RSA Key Generation request, Deprecated */
	IPC_MESSAGE_OPCODE_GET_CERT_CHAIN_LEN = 0x45,	/**< Opcode of Get certificate chain length request */
	IPC_MESSAGE_OPCODE_GET_CERT = 0x46,				/**< Opcode of Get certificate request */
	IPC_MESSAGE_OPCODE_TDISP_INTERRUPT = 0x49,		/**< Opcode of TDISP Interrupt request */
};

/**
 * Maximum IPC Message data length in bytes.
 */
#define IPC_MESSAGE_MAX_PAYLOAD_LEN_IN_DWORDS			(15)

/**
 * Maximum IPC message Tag Value.
 */
#define IPC_MESSAGE_MAX_TAG_VALUE						(256)

/**
 * Increment IPC Tag value.
 */
#define IPC_MESSAGE_TAG_INC(tag)						((tag + 1) % IPC_MESSAGE_MAX_TAG_VALUE)

/**
 * IPC Message Request bit.
 */
#define IPC_MESSAGE_REQUEST_BIT							(0)

/**
 * IPC Message Response bit.
 */
#define IPC_MESSAGE_RESPONSE_BIT						(1)

/**
 * Maximum possible response value for ipc message
 */
#define IPC_MESSAGE_MAX_RESPONSE_STATUS_VALUE			(15)

/**
 * Default Failed response value if the response code is higher than the
 * `IPC_MESSAGE_MAX_RESPONSE_STATUS_VALUE`
 */
#define IPC_MESSAGE_FAILED_RESPONSE_STATUS				(15)

/**
 * Function identifier for a virtual function.
 */
#define	IPC_MESSAGE_FUNCTION_ID_VF(vf_num)				(vf_num)

/**
 * Function identifier for the physical function.
 */
#define	IPC_MESSAGE_FUNCTION_ID_PF						(64)


/**
 * Max number of certificate length entries that IPC payload can hold.
 */
#define	IPC_MESSAGE_GET_CERT_CHAIN_LEN_MAX_CERTS		(13)


#pragma pack(push, 1)
/**
 * Define ipc_message_header structure and bit field information of each components.
 */
struct ipc_message_header {
	uint32_t opcode:7;		/**< Message operation code to identify the type of message */
	uint32_t response:1;	/**< Identify the message direction, 0: request, 1: response */
	uint32_t tag:8;			/**< Message tag identifier */
	uint32_t status:4;		/**< Response status code, 0: success, 0x01 - 0x0F: Message specific error codes */
	uint32_t reserved:4;	/**< Reserved */
	uint32_t data_length:8;	/**< Variable length of data payload in bytes */
};

/**
 * Define fixed length IPCMessage data.
 */
struct ipc_message {
	/**
	 * Common header for IPC messages.
	 */
	struct ipc_message_header header;

	/**
	 * 60-bytes of IPC message data payload, as an array of 32-bit unsigned integer
	 */
	uint32_t data[IPC_MESSAGE_MAX_PAYLOAD_LEN_IN_DWORDS];
};

/**
 * Generic IPC message status codes.
 *
 * Note:
 * - 0x00 to 0x06 and 0x0F are reserved for generic IPC message status codes.
 * - 0x07 to 0x0E can be used for request-specific error codes.
 */
enum ipc_message_status_code {
	IPC_MESSAGE_STATUS_CODE_SUCCESS = 0,				/**< Status success */
	IPC_MESSAGE_STATUS_CODE_MESSAGE_NOT_SUPPORTED = 1,	/**< Unsupported value in the message opcode field */
	IPC_MESSAGE_STATUS_CODE_INVALID_FIELD = 2,			/**< Invalid field contained in message */
	IPC_MESSAGE_STATUS_CODE_FUNCTION_NOT_ENABLED = 3,	/**< Function is not enabled */
	IPC_MESSAGE_STATUS_CODE_OPERATION_TIMEOUT = 4,		/**< Operation timed out */
	IPC_MESSAGE_STATUS_CODE_OPERATION_FAILED = 5,		/**< Operation failed */
	IPC_MESSAGE_STATUS_CODE_UNKNOWN_STATUS = 0xF,		/**< Unknown status code */
};


/**
 * Define enum for error response code for DoE message in IPC message request
 */
enum ipc_message_doe_response_status {
	IPC_MESSAGE_DOE_RESPONSE_STATUS_NO_RESPONSE_BUFFER = 1,	/** < DOE Message error response code */
};

/**
 * Define payload structure for the DoE IPC message.
 */
struct ipc_message_doe_payload {
	uint32_t buffer_address;	/* Buffer Address to copy data in GSRAM */
};

/**
 * Define the IPC message structure for a DoE message.
 *
 * HSP <--> Admin
 */
struct ipc_message_doe {
	struct ipc_message_header header;		/**< Common header for IPC messages. */
	struct ipc_message_doe_payload payload;	/**< Payload for the DOE message. */
};


_Static_assert ((sizeof (struct ipc_message_doe) <= sizeof (struct ipc_message)),
	"Size of ipc_message_doe is greater than size of ipc_message.");

/**
 * Supported codes for requesting a status change.
 */
enum {
	IPC_MESSAGE_STATUS_CHANGE_PREPARE_RELEASE = 0x0a,	/**< Prepare to run by restoring states from GSRAM. */
	IPC_MESSAGE_STATUS_CHANGE_RELEASE = 0x0b,			/**< Resume UCD and enter normal execution mode. */
};

/**
 * Error status codes for the Status Change message.
 */
enum {
	IPC_MESSAGE_STATUS_CHANGE_ERROR = 1,	/**< Error changing to the requested status. */
};

/**
 * Define payload structure for the Status change IPC message.
 */
struct ipc_message_status_change_payload {
	uint8_t requested_status;	/**< Status change being requested. */
};

/**
 * Define the IPC message structure for a Status Change message.
 *
 * HSP <--> Admin
 */
struct ipc_message_status_change {
	struct ipc_message_header header;					/**< Common header for IPC messages. */
	struct ipc_message_status_change_payload payload;	/**< Payload for the Status Change message. */
};


_Static_assert ((sizeof (struct ipc_message_status_change) <= sizeof (struct ipc_message)),
	"Size of ipc_message_status_change is greater than size of ipc_message.");


/**
 * Error status codes for the Shutdown Request message.
 */
enum {
	IPC_MESSAGE_SHUTDOWN_REQUEST_ERROR = 1,		/**< Error shutting down cores. */
	IPC_MESSAGE_SHUTDOWN_REQUEST_TIMEOUT = 4,	/**< Shutdown request timed out to at least one core. */
	IPC_MESSAGE_SHUTDOWN_REQUEST_FAILURE = 5,	/**< At least one cored failed to shutdown for reset. */
};

/**
 * Define payload structure for the Shutdown Request message.
 */
struct ipc_message_shutdown_request_payload {
	uint32_t drain_time;	/**< The time, in milliseconds, allowed to be used for draining queues. */
};

/**
 * Define the IPC message structure for a Shutdown Request message.
 *
 * HSP <--> Admin
 */
struct ipc_message_shutdown_request {
	struct ipc_message_header header;						/**< Common header for IPC messages. */
	struct ipc_message_shutdown_request_payload payload;	/**< Payload for the Shutdown Request message. */
};


_Static_assert ((sizeof (struct ipc_message_shutdown_request) <= sizeof (struct ipc_message)),
	"Size of ipc_message_shutdown_request is greater than size of ipc_message.");


/**
 * Error status codes for the Stop Interface message.
 */
enum {
	IPC_MESSAGE_STOP_INTERFACE_ERROR = 1,	/**< Error handling a Stop Interface message. */
};

/**
 * Define payload structure for the Stop Interface message.
 */
struct ipc_message_stop_interface_payload {
	uint64_t vf_mask;	/**< Mask indicating which virtual functions are impacted by the source event. */
	uint32_t pf_mask;	/**< Mask indicating whether the Physical Function is impacted by the source event. */
};

/**
 * Define the IPC message structure for a Stop Interface message.
 *
 * HSP -> Admin
 */
struct ipc_message_stop_interface {
	struct ipc_message_header header;					/**< Common header for IPC messages. */
	struct ipc_message_stop_interface_payload payload;	/**< Payload for the Stop Interface message. */
};


_Static_assert ((sizeof (struct ipc_message_stop_interface) <= sizeof (struct ipc_message)),
	"Size of ipc_message_stop_interface is greater than size of ipc_message.");


/**
 * Define payload structure for a Get Certificate Chain Length message.
 */
struct ipc_message_get_cert_chain_len_payload {
	uint8_t digest[SHA256_HASH_LENGTH];						/**< SHA-256 hash of the raw certificate data. */
	uint8_t num_certs;										/**< Number of certificate lengths being reported. */
	uint16_t len[IPC_MESSAGE_GET_CERT_CHAIN_LEN_MAX_CERTS];	/**< List of certificate lengths. */
};

/**
 * Define the IPC message structure for Get Certificate Chain Length message.
 *
 * HSP <--> HSM
 */
struct ipc_message_get_cert_chain_len {
	struct ipc_message_header header;						/**< Common header for IPC messages. */
	struct ipc_message_get_cert_chain_len_payload payload;	/**< Payload for the Get Certificate Chain Length message. */
};


/**
 * Get the data length for a response containing certificate lengths.
 */
#define	ipc_message_get_cert_chain_len_data_length(num_certs)   \
	(SHA256_HASH_LENGTH + sizeof (uint8_t) + (sizeof (uint16_t) * num_certs))


_Static_assert ((sizeof (struct ipc_message_get_cert_chain_len) <= sizeof (struct ipc_message)),
	"Size of ipc_message_get_cert_chain_len is greater than size of ipc_message.");

/**
 * Define payload structure for a Get Certificate message.
 */
struct ipc_message_get_cert_payload {
	uint8_t cert_num;	/* In: certificate number */
	uint16_t cert_size;	/* Out: certificate size */
	uint64_t buf_addr;	/* In: start address of dTCM buffer to store certificate */
	uint16_t buf_size;	/* In: dTCM buffer size */
};

/**
 * Define the IPC message structure for Get Certificate message.
 *
 * HSP <--> HSM
 */
struct ipc_message_get_cert {
	struct ipc_message_header header;				/**< Common header for IPC messages. */
	struct ipc_message_get_cert_payload payload;	/**< Payload for the Get Certificate message. */
};


_Static_assert ((sizeof (struct ipc_message_get_cert) <= sizeof (struct ipc_message)),
	"Size of ipc_message_get_cert is greater than size of ipc_message.");


#define IPC_MESSAGE_TDISP_INTERRUPT_NUM_REGISTERS	(5)	/**< Number of registers in TDISP interrupt message payload */

/**
 * Error status codes for the TDISP interrupt message.
 */
enum {
	IPC_MESSAGE_TDISP_INTERRUPT_ERROR = 1,	/**< Error handling a TDISP interrupt message. */
};

/**
 * Source values for the TDISP interrupt message.
 */
enum {
	IPC_MESSAGE_TDISP_INTERRUPT_SOURCE_TDISP = 0x0,			/**< TDISP source event. */
	IPC_MESSAGE_TDISP_INTERRUPT_SOURCE_IDE = 0x1,			/**< IDE source event. */
	IPC_MESSAGE_TDISP_INTERRUPT_SOURCE_FLR = 0x2,			/**< FLR (Function Level Reset) source event. */
	IPC_MESSAGE_TDISP_INTERRUPT_SOURCE_PERST_UP = 0x3,		/**< PERST UP source event. */
	IPC_MESSAGE_TDISP_INTERRUPT_SOURCE_PERST_DOWN = 0x4,	/**< PERST DOWN source event. */
};

/**
 * Define payload structure for the TDISP interrupt message.
 */
struct ipc_message_tdisp_interrupt_payload {
	uint32_t source;											/**< The source of the event. */
	uint64_t vf_mask;											/**< Mask indicating which virtual functions are impacted by the source event. */
	uint32_t pf_mask;											/**< Mask indicating whether the Physical Function is impacted by the source event. */
	uint32_t reg[IPC_MESSAGE_TDISP_INTERRUPT_NUM_REGISTERS];	/**< Register data associated with the source event. */
};

/**
 * Define the IPC message structure for a TDISP interrupt message.
 *
 * Admin -> HSP
 */
struct ipc_message_tdisp_interrupt {
	struct ipc_message_header header;					/**< Common header for IPC messages. */
	struct ipc_message_tdisp_interrupt_payload payload;	/**< Payload for the TDISP interrupt message. */
};


_Static_assert ((sizeof (struct ipc_message_tdisp_interrupt) <= sizeof (struct ipc_message)),
	"Size of ipc_message_tdisp_interrupt is greater than size of ipc_message.");



#pragma pack(pop)


void ipc_message_build_response (struct cmd_interface_msg *message, uint8_t log_message_id,
	uint32_t error_code);


#endif	/* IPC_MESSAGE_H_ */
