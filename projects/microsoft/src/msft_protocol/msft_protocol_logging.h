// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MSFT_PROTOCOL_LOGGING_H_
#define MSFT_PROTOCOL_LOGGING_H_

#include "logging/msft_debug_log.h"


/**
 * Logging messages for Microsoft Vendor Defined Protocol (MVDP) command handling.
 */
enum {
	MSFT_PROTOCOL_LOGGING_ERROR,					/**< An error occurred while processing a request. */
	MSFT_PROTOCOL_LOGGING_SKIP_ERROR_DATA,			/**< Status error data did not fit into the message. */
	MSFT_PROTOCOL_LOGGING_REBOOT_FAILED,			/**< Failed to execute a requested reboot. */
	MSFT_PROTOCOL_LOGGING_NOTIFICATION_FAILED,		/**< Failed to send notification to registered listeners. */
	MSFT_PROTOCOL_LOGGING_NOTIFICATION_RESP_ERROR,	/**< Failed to parse notification response. */
};


#endif	/* MSFT_PROTOCOL_LOGGING_H_ */
