// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MCTP_NOTIFIER_MSFT_H_
#define MCTP_NOTIFIER_MSFT_H_

#include <stdbool.h>
#include <stdint.h>
#include "platform_api.h"
#include "mctp/mctp_notifier_interface.h"
#include "msft_protocol/msft_mctp_protocol.h"


/**
 * MCTP notifier MSFT object. This will handle listener EID
 * register/deregister/send_notification by using mctp transport interface.
 */
struct mctp_notifier_msft {
	struct mctp_notifier_interface base;	/**< MCTP notifier interface base object */
	const struct msg_transport *transport;	/**< Message transport instance */
	uint8_t *msg_buffer;					/**< Message buffer for sending notification */
	uint32_t msg_buffer_len;				/**< Length of the above msg_buffer */
	uint32_t resp_timeout_ms;				/**< Response timeout value in ms */
	bool skip_send_fail_log;				/**< Flag to skip logging send failure for the notification */
};


int mctp_notifier_msft_send_notification_request (
	const struct mctp_notifier_interface *notifier,	uint8_t *payload, size_t payload_len,
	uint8_t dest_eid);


#endif	/* MCTP_NOTIFIER_MSFT_H_ */
