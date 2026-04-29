// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_COMMANDS_H_
#define MANTICORE_COMMANDS_H_

#include "firmware/graceful_shutdown.h"
#include "msft_protocol/msft_mctp_protocol.h"


/**
 * Version of the Manticore protocol provided by this implementation.
 */
#define	MANTICORE_PROTOCOL_VERSION			0

/**
 * Command codes for the Manticore MSFT command set.
 */
enum {
	MSFT_PROTOCOL_MANTICORE_GET_DRAIN_TIME = 0x03,	/**< Set drain time for iDFU */
	MSFT_PROTOCOL_MANTICORE_SET_DRAIN_TIME = 0x04,	/**< Get drain time for iDFU */
};

#pragma pack(push, 1)
/**
 * A request to get the Manticore Drain time.
 */
struct manticore_get_drain_time_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
};

/**
 * The response containing drain time of manticore.
 */
struct manticore_get_drain_time_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint32_t drain_time_ms;								/**< Bit mask of drain time. */
};

/**
 * A request to set the Manticore Drain time.
 */
struct manticore_set_drain_time_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint32_t drain_time_ms;
};


/**
 * The drain time response message.
 */
struct manticore_set_drain_time_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
};

#pragma pack(pop)


int manticore_set_drain_time (struct cmd_interface_msg *request,
	const struct graceful_shutdown_control *shutdown_ctrl);

int manticore_get_drain_time (struct cmd_interface_msg *request,
	const struct graceful_shutdown_control *shutdown_ctrl);


#endif	/* MANTICORE_COMMANDS_H_ */
