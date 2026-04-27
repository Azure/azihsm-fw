// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MCTP_NOTIFIER_MSFT_MUTABLE_LIST_STATIC_H_
#define MCTP_NOTIFIER_MSFT_MUTABLE_LIST_STATIC_H_

#include "msft_protocol/mctp_notifier_msft_mutable_list.h"


/* Internal functions declared to allow for static initialization. */
int mctp_notifier_msft_mutable_list_send_notification_request (
	const struct mctp_notifier_interface *notifier,	uint8_t *payload, size_t payload_len);
int mctp_notifier_msft_mutable_list_register_listener (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid);
int mctp_notifier_msft_mutable_list_force_register_listener (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid);
int mctp_notifier_msft_mutable_list_deregister_listener (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid);


/**
 * Constant initializer for the task API.
 */
#define	MCTP_NOTIFIER_MSFT_MUTABLE_LIST_API_INIT { \
		.register_listener = mctp_notifier_msft_mutable_list_register_listener, \
		.force_register_listener = mctp_notifier_msft_mutable_list_force_register_listener, \
		.deregister_listener = mctp_notifier_msft_mutable_list_deregister_listener, \
		.send_notification_request = mctp_notifier_msft_mutable_list_send_notification_request \
	}


/**
 * Initialize a static instance of a MCTP notifier mutable list which does not log notification failure errors.
 *
 * There could be cases where the notifier is sending notification to an endpoint which does not
 * have support for the particular notification or the endpoint is down, in those cases the send_notification_request
 * will fail and log an error. This initializer can be used to create a notifier instance which can skip logging send
 * failure for the notification.
 *
 * There is no validation done on the arguments.
 *
 * @param transport_ptr msg_transport context.
 * @param state_ptr mctp_notifier_state context.
 * @param eid_list_ptr eid array list pointer.
 * @param eid_list_ptr_len eid array list length.
 * @param msg_buffer_ptr transport message buffer pointer.
 * @param msg_buffer_ptr_len transport message buffer length.
 * @param response_timeout_ms Response timeout in ms.
 */
#define	mctp_notifier_msft_mutable_list_static_init_no_fail_log(state_ptr, transport_ptr, eid_list_ptr, \
	eid_list_ptr_len, msg_buffer_ptr, msg_buffer_ptr_len, response_timeout_ms) { \
		.base = { \
			.base = MCTP_NOTIFIER_MSFT_MUTABLE_LIST_API_INIT, \
			.transport = transport_ptr, \
			.msg_buffer = msg_buffer_ptr, \
			.msg_buffer_len = msg_buffer_ptr_len, \
			.resp_timeout_ms = response_timeout_ms, \
			.skip_send_fail_log = true, \
		}, \
		.state = state_ptr, \
		.eid_list = eid_list_ptr, \
		.eid_list_len = eid_list_ptr_len, \
	}

/**
 * Initialize a static instance of a MCTP notifier mutable list.
 *
 * There is no validation done on the arguments.
 *
 * @param transport_ptr msg_transport context.
 * @param state_ptr mctp_notifier_state context.
 * @param eid_list_ptr eid array list pointer.
 * @param eid_list_ptr_len eid array list length.
 * @param msg_buffer_ptr transport message buffer pointer.
 * @param msg_buffer_ptr_len transport message buffer length.
 * @param response_timeout_ms Response timeout in ms.
 */
#define	mctp_notifier_msft_mutable_list_static_init(state_ptr, transport_ptr, eid_list_ptr, \
	eid_list_ptr_len, msg_buffer_ptr, msg_buffer_ptr_len, response_timeout_ms) { \
		.base = { \
			.base = MCTP_NOTIFIER_MSFT_MUTABLE_LIST_API_INIT, \
			.transport = transport_ptr, \
			.msg_buffer = msg_buffer_ptr, \
			.msg_buffer_len = msg_buffer_ptr_len, \
			.resp_timeout_ms = response_timeout_ms, \
		}, \
		.state = state_ptr, \
		.eid_list = eid_list_ptr, \
		.eid_list_len = eid_list_ptr_len, \
	}


#endif	/* MCTP_NOTIFIER_MSFT_MUTABLE_LIST_STATIC_H_ */
