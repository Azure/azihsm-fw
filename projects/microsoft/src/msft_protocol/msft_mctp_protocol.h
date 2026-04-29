// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MSFT_MCTP_PROTOCOL_H_
#define MSFT_MCTP_PROTOCOL_H_

#include <stdint.h>
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cmd_interface.h"
#include "mctp/mctp_base_protocol.h"


/* TODO:  This layer really doesn't know or care about MCTP, so it would be better if MCTP was
 * removed from the naming.  This protocol could theoretically run over other transports, especially
 * when the tight coupling between Cerberus protocol constructs and MCTP is broken. */


/**
 * Minimum message length to be a valid MSFT request.
 */
#define MSFT_MCTP_PROTOCOL_MIN_REQUEST_LEN                  \
	(sizeof (struct cerberus_protocol_header) + (sizeof (struct msft_mctp_protocol_header)))

/**
 * Minimum message length to be a valid MSFT response.
 */
#define MSFT_MCTP_PROTOCOL_MIN_RESPONSE_LEN                 \
	(sizeof (struct cerberus_protocol_header) + \
		(sizeof (struct msft_mctp_protocol_response_header)))

/**
 * Maximum message payload length for a MSFT MCTP request.
 */
#define MSFT_MCTP_PROTOCOL_MAX_PAYLOAD_PER_REQUEST          \
	(MCTP_BASE_PROTOCOL_MAX_MESSAGE_BODY - MSFT_MCTP_PROTOCOL_MIN_REQUEST_LEN)

/**
 * Default timeout when waiting for a response to an MSFT MCTP request.
 */
#define MSFT_MCTP_PROTOCOL_DEFAULT_RESPONSE_TIMEOUT_MS		1000

/* Required header fields for a valid MSFT MCTP request. */
#define MSFT_MCTP_PROTOCOL_MSFT_PCI_VID						0x1414
#define MSFT_MCTP_PROTOCOL_ESCAPE_SEQ1						0x80
#define MSFT_MCTP_PROTOCOL_ESCAPE_SEQ2						0xFF


/**
 * Identifiers for MSFT MCTP command sets.
 */
enum {
	MSFT_MCTP_PROTOCOL_COMMAND_SET_BASE = 0x00,			/**< The required base command set. */
	MSFT_MCTP_PROTOCOL_COMMAND_SET_BMC = 0x01,			/**< The BMC command set. */
	MSFT_MCTP_PROTOCOL_COMMAND_SET_ROT = 0x02,			/**< The RoT command set. */
	MSFT_MCTP_PROTOCOL_COMMAND_SET_MANTICORE = 0x03,	/**< The Manticore command set. */
	MSFT_MCTP_PROTOCOL_COMMAND_SET_1P_BMC = 0x04,		/**< The 1P BMC command set. */
	MSFT_MCTP_PROTOCOL_COMMAND_SET_FIPS = 0x05,			/**< The FIPS command set. */
	MSFT_MCTP_PROTOCOL_COMMAND_SET_TIP = 0x06,			/**< The BMC-TIP command set. */
	MSFT_MCTP_PROTOCOL_COMMAND_SET_OVL3 = 0x07,			/**< The OVL3 command set. */
};


#pragma pack(push, 1)
/**
 * MSFT MCTP request header.  This is common across all command sets.
 */
struct msft_mctp_protocol_header {
	uint8_t command_set;		/**< Command set identifier for the request. */
	uint16_t protocol_version;	/**< Protocol version used to construct the request. */
	uint8_t command;			/**< Command identifier for the request. */
};

/**
 * MSFT MCTP response header.  This is common across all command sets.
 */
struct msft_mctp_protocol_response_header {
	uint8_t command_set;		/**< Command set identifier for the response. */
	uint16_t protocol_version;	/**< Protocol version used to construct the response. */
	uint8_t command;			/**< Command identifier for the response. */
	uint8_t completion_code;	/**< Completion code indicating the execution result. */
};

#pragma pack(pop)


int msft_mctp_protocol_populate_header (struct msft_mctp_protocol_header *header, uint8_t command,
	uint8_t command_set, uint16_t version);

void msft_mctp_protocol_populate_cerberus_header (struct cerberus_protocol_header *header);
void msft_mctp_protocol_add_cerberus_header (struct cmd_interface_msg *message);


#endif	/* MSFT_MCTP_PROTOCOL_H_ */
